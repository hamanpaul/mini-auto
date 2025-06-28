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
#include "usbapi.h"
#if (defined(NEW_UI_ARCHITECTURE) )
    #include "MainFlow.h"
#endif
#include <mars_controller/mars_dma.h>
#include "p2pserver_api.h"
#include "hdmiapi.h"

#if PWIFI_SUPPORT
#include "pwifiapi.h"
#endif


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
u8 uiEnMenu=0;

u8 uiEnZoom=0;
u32 uiEnZoom_Y_Offset=0;
u32 uiEnZoom_UV_Offset=0;
 
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
u8 start_iconflag[UIACTIONNUM]={0};
u8 UI_CFG_RES[4] = {0};
u8 UI_TMP_RES[4] = {0};
u8 UI_P2P_RES[4] = {0};
u8 UI_P2P_PSW[UI_P2P_PSW_MAX_LEN];
u8 Start_UI_P2P_PSW[UI_P2P_PSW_MAX_LEN];
u8 UI_VGA_MaskArea[MULTI_CHANNEL_MAX][9];
u8 UI_HD_MaskArea[MULTI_CHANNEL_MAX][9];
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
u16 graphDispWidth[]= {
    PANNEL_X,
    TVOUT_X,
};
u16 graphDispHeight[]= {
    PANNEL_Y,
    TVOUT_Y,
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
u8  UI_USBLastStat;
u8  uiVersion[32]={0};
u8  uiVersionTime[9]={0};
u8  CurMotEnb[4], CurMotdayLev[4], CurMotnightLev[4];
u8  uiQuadDisplay;  /*1: quad mode, 0: single mode, 2: dual mode*/
u8 gChangResTimeout=0;
u32 prev_P2pVideoQuality=2;
u8  uiSetTxTimeStamp[MULTI_CHANNEL_MAX] = {0};
#if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
u8  uiSetRfLightTimer[MULTI_CHANNEL_MAX] = {UI_SET_RF_BUSY};
#elif(UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
u8  uiSetRfLightTimer[MULTI_CHANNEL_MAX][7] = {UI_SET_RF_BUSY};
#endif
u8  uiSetRfLightDimmer[MULTI_CHANNEL_MAX] = {UI_SET_RF_BUSY};
u8  uiSetRfLightDur[MULTI_CHANNEL_MAX] = {UI_SET_RF_BUSY};
u8  uiSetRfLightState[MULTI_CHANNEL_MAX] = {UI_SET_RF_BUSY};
u8  uiRFStatue[MAX_RFIU_UNIT]={UI_RF_STATUS_OTHER,UI_RF_STATUS_OTHER,UI_RF_STATUS_OTHER,UI_RF_STATUS_OTHER};

#if RFIU_RX_WAKEUP_TX_SCHEME
u8 uiPIRScheduleOnOff[MAX_RFIU_UNIT];
#endif

bool PlaybackDBG = FALSE;
#if DOOR_BELL_SUPPORT
bool dortest = FALSE;
u32  DorBel_TriID = 0;
#endif
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern int gOnlineNum;

extern OS_EVENT             *mpeg4ReadySemEvt;
extern INT32U  guiIISCh0RecDMAId, guiIISCh1RecDMAId, guiIISCh2RecDMAId, guiIISCh3RecDMAId;
extern INT32U  guiIISCh0PlayDMAId, guiIISCh1PlayDMAId, guiIISCh2PlayDMAId, guiIISCh3PlayDMAId,guiIISCh4PlayDMAId;

/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */
extern void uiSetTVOutXY(void);
extern void iduTVColorbar_onoff(u8  onoff);
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
        #if ((SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1)||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1))
        for (i = 0; i < UIACTIONNUM; i++)
            iconflag[i] = defaultvalue[i];

        #else
        Read_UI_Setting();
        #endif
        #if RFIU_SUPPORT
            spiReadRF_ID();
        #endif
        #if NIC_SUPPORT
            spiReadNet();
        #endif  
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

u8 CheckCurrResolution(void)
{
    if(getCurrentVDOSize() == TV_1080i60)
        return 1;
    else
        return 0;
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

#if 0   /*for debug*/
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

    #endif


        if(sysVideoInSel == VIDEO_IN_TV)
        {
            sysTVinFormat = getTVinFormat();
            DEBUG_UI("-->sysTVinFormat=%d\n",sysTVinFormat);
        }
    }

}
void uiTask(void* pData)
{
    u8 err;

#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1, time2;
#endif

    uiWaitMainInitReady();

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
#if SD_TASK_INSTALL_FLOW_SUPPORT
                    case UI_KEY_DEVCD:
#endif
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

        #if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
            (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) ||\
            (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2)|| (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))
            for(i=0;i<MAX_RFIU_UNIT;i++)
            {
                setFlag|=(RFIU_TX_STA_LINK_BROKEN<<(i*(32/MAX_RFIU_UNIT)) ) ;
                setFlag|=(RFIU_TX_STA_LINK_OK<<(i*(32/MAX_RFIU_UNIT)));
                setFlag|=(RFIU_TX_STA_PAIR_OK<<(i*(32/MAX_RFIU_UNIT)));
            }
        #else
            for(i=0;i<MAX_RFIU_UNIT;i++)
            {
                setFlag|=(RFIU_RX_STA_LINK_BROKEN<<(i*(32/MAX_RFIU_UNIT)) ) ;
                setFlag|=(RFIU_RX_STA_LINK_OK<<(i*(32/MAX_RFIU_UNIT)));
                setFlag|=(RFIU_RX_STA_PAIR_OK<<(i*(32/MAX_RFIU_UNIT)));
                setFlag|=(RFIU_RX_STA_ERROR<<(i*(32/MAX_RFIU_UNIT)));
                //setFlag|=(RFIU_RX_STA_CHGRESO<<(i*(32/MAX_RFIU_UNIT)));
                //setFlag|=(RFIU_RX_STA_FRAMESYNC<<(i*(32/MAX_RFIU_UNIT)));
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
    #if(RF_RX_FULLVIEW_RESO_SEL == RF_RX_RESO_VGA)
        if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight>=720)
        {
        #if RFRX_FULLSCR_HD_SINGLE
              iduPlaybackMode(RF_RX_2DISP_WIDTH,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight/2,RF_RX_2DISP_WIDTH);
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
    #elif(RF_RX_FULLVIEW_RESO_SEL == RF_RX_RESO_HD)    
        if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
        {
        #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
           //iduPlaybackMode( 1280 & 0xffffff80,720,RF_RX_2DISP_WIDTH*2);
           memDispBufArrage(DISPBUF_9300FHD);
           iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
        #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
           if(sysTVOutOnFlag)
           {
            #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                sysTVswitchResolutionbyImagesize();
            #else
                if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                else
                  iduPlaybackMode(1920,1080,1920);
            #endif
           }
           else
           {
              iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
           }
        #else   
           iduPlaybackMode(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth/2,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight/2,RF_RX_2DISP_WIDTH*2);
        #endif
        }
        else
        {
        #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            memDispBufArrage(DISPBUF_NORMAL);
        #endif
        #if( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
            if(sysTVOutOnFlag)
            {
            #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                sysTVswitchResolutionbyImagesize();
            #else
                if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                else
                  iduPlaybackMode(1920,1080,1920);
            #endif
            }
            else
            {
              iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            }
        #else
            iduPlaybackMode(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight,RF_RX_2DISP_WIDTH*2);
        #endif
        }
    #endif

        if(sysTVOutOnFlag)
        {
            if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
               tvTVE_INTC   =TV_INTC_FRMEND__ENA;
            else
               tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
	    }
		else
	    {
	        IduIntCtrl |= IDU_FTCINT_ENA;
		}
	}
    else if( sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA )
    {
        if(sysTVOutOnFlag)
        {
            if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
            {
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            }
            else
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30)  || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
               tvTVE_INTC   =TV_INTC_FRMEND__ENA;
            else
               tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
	    }
		else
	    {
            #if(LCM_OPTION == VGA_1024X768_60HZ)
               iduPlaybackMode(1024,720,RF_RX_2DISP_WIDTH*2);
            #else
               iduPlaybackMode(1024,600,RF_RX_2DISP_WIDTH*2);
            #endif
            
	        IduIntCtrl |= IDU_FTCINT_ENA;
		}
    }
    else if(sysCameraMode==SYS_CAMERA_MODE_RF_RX_QUADSCR)
    {
    #if( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )

    if(sysTVOutOnFlag)
    {
    #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
        sysTVswitchResolutionbyImagesize();
    #endif
    }
      #if RFRX_HALF_MODE_SUPPORT
        if(rfiuRX_CamOnOff_Num <= 2)
        {
            if(sysTVOutOnFlag)
                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            else
                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT*2,RF_RX_2DISP_WIDTH*2);
        }
        else
      #endif
            iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);

    #else
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
    #endif
        if(sysTVOutOnFlag)
        {
            if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30)  || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
               tvTVE_INTC   =TV_INTC_FRMEND__ENA;
            else
               tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
	    }
		else
	    {
	        IduIntCtrl |= IDU_FTCINT_ENA;
		}
    }
    else if(sysCameraMode == SYS_CAMERA_MODE_PLAYBACK)
    {
        if(asfVopWidth > RF_RX_2DISP_WIDTH*2) //1920
        {
          #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            memDispBufArrage(DISPBUF_NORMAL);
          #endif
          #if( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
            if(sysTVOutOnFlag)
            {
            #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                sysTVswitchResolutionbyImagesize();
            #else
                if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                else
                  iduPlaybackMode(1920,1080,1920);
            #endif
            }
            else    //panel
            {
              TvOutMode = SYS_TV_OUT_HD720P60;
              iduPlaybackMode(1920/2,1072/2,1280);
            }
          #else
            iduPlaybackMode(asfVopWidth,asfVopHeight,RF_RX_2DISP_WIDTH*2);
          #endif
        }
        else    //720
        {
        #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
           //iduPlaybackMode( 1280 & 0xffffff80,720,RF_RX_2DISP_WIDTH*2);
           memDispBufArrage(DISPBUF_9300FHD);
           iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
        #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
           if(sysTVOutOnFlag)
           {
            #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                sysTVswitchResolutionbyImagesize();
            #else
                if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                else
                  iduPlaybackMode(1920,1080,1920);
            #endif
           }
           else
           {
              iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
           }
        #else   
           iduPlaybackMode(asfVopWidth/2,asfVopHeight/2,RF_RX_2DISP_WIDTH*2);
        #endif
        }
        
        if(sysTVOutOnFlag)
        {
            if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
               tvTVE_INTC   =TV_INTC_FRMEND__ENA;
            else
               tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
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
#if (UI_VERSION == UI_VERSION_TX)
#else
    uiSetTVOutXY();
    uiOSDPreviewInit();
#endif
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
#if RFIU_RX_WAKEUP_TX_SCHEME
    u8 i;
#endif

    uiCheckTVInFormat();

    /* Initialize UI menu */
    uiMenuBootSetInit();
    #if (UI_SUPPORT_TREE == 1)
    uiMenuInit();
    #endif
    uiMenuOSDReset();

    uiMenuTreeInit();
#if (UI_VERSION == UI_VERSION_TX)
#else
    uiOsdIconInit();
#endif

#if RFIU_RX_WAKEUP_TX_SCHEME
    for(i=0;i<MAX_RFIU_UNIT;i++)
       uiPIRScheduleOnOff[i]=1;
#endif


    uiSemEvt = OSSemCreate(0);
    message_MboxEvt = OSMboxCreate(NULL);
    #if(UI_VERSION == UI_VERSION_THREE_TASK)
        MyHandler.MenuMode = VIDEO_MODE;
        OSTaskCreate(UIMOVIE_TASK, UIMOVIE_TASK_PARAMETER, UIMOVIE_TASK_STACK, UIMOVIE_TASK_PRIORITY);
    #else
        OSTaskCreate(UI_TASK, UI_TASK_PARAMETER, UI_TASK_STACK, UI_TASK_PRIORITY);
        OSTaskCreate(UI_SUB_TASK, UI_SUB_TASK_PARAMETER, UI_SUB_TASK_STACK, UI_SUB_TASK_PRIORITY);
    #endif

    uiOSDSemEvt = OSSemCreate(1);
    uiAlarmSemEvt = OSSemCreate(1);
    gUiKeyFlagGrp = OSFlagCreate(0x00000000, &err);
    gUiStateFlagGrp = OSFlagCreate(0x00000000, &err);
#if DOOR_BELL_SUPPORT
    DoorBellinit();
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

    Check PIR schedule at that moment.

Arguments:

    None.

Return Value:

    0 - PIR should POWER OFF.
    1 - PIR should POWER ON
*/

u32 uiCheckPIRSchedule(int CamID)
{
    //not implement now.
 #if RFIU_RX_WAKEUP_TX_SCHEME  
    return uiPIRScheduleOnOff[CamID];
 #else
    return 1;
 #endif
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
            sysSetEvt(direction, playbackflag);
            DEBUG_UI("uiReadForward or Backward (%d) VideoFile\r\n",direction);
            return TRUE;
        }
    }
    DEBUG_UI("Can not Read File %d\r\n",waitFlag);
    return FALSE;
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
        if (Main_Init_Ready == 1)
            DEBUG_UI("post uiSemEvt key %d to UI\n",UIKey);
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
    u8  tmp;
	

    if(dcfStorageType != STORAGE_MEMORY_SD_MMC)
        return 0;

    if ((pFile = dcfOpen((char *)InitSettingFileName, "r")) == NULL)
    {
        DEBUG_UI("Error: dcf open %s error!\n", InitSettingFileName);
        return 0;
    }

    dcfRead(pFile, SettingAddr, pFile->size, &codeSize);
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

    if(sysVideoInSel == VIDEO_IN_TV)
    {
       sysTVinFormat=getTVinFormat();
       DEBUG_UI("-->sysTVinFormat=%d\n",sysTVinFormat);
    }

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
#if SD_CARD_DISABLE

#else
    dcfNewFile = 0;
#endif

    level= sysCheckSDCD();

    if(mode == 1)    //mode=1: 第一次開機.
        UI_SDLastLevel = level;
    else  //開機後熱插拔
        level = UI_SDLastLevel;

    if(level==SDC_CD_OFF)
    {   //High
        DEBUG_UI("SD Remove!!\n\r");
        if(mode) // mode:1 first run on power on
        {
            sysSDCD_OFF(1);
        }
        else
        {
            if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_SDC)
    	        MemoryFullFlag = FALSE;
            status = sysSetEvt(SYS_EVT_SDCD_OFF, 0);
            if(status)
                OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_WAIT_SET_ALL, OS_IPC_WAIT_FOREVER, &err);
            else
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
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
                OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_WAIT_SET_ALL, OS_IPC_WAIT_FOREVER, &err);
            else
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
        }

        return 1;
    }

CheckSdCdAbnormal:

#if 0    
    if(sysPlaybackVideoStart==1)
    {
        DEBUG_UI("Abnormal Operation in playback!\n\r");
        uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,138 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
        osdDrawMessage(MSG_ABNORMAL_OPERATION, CENTERED, 110+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
        sysPlaybackVideoStop = 1;
        sysPlaybackVideoPause=0;
        sysPlaybackVideoStart=0;
        timerCountPause(1, 0);
        OSTimeDly(10); // Delay for protect system
        //sysSetEvt(SYS_EVT_PWROFF, 0);
        sysPowerOff(0);
    }
#endif    
    return 0;
}

#if USB_HOST_MASS_SUPPORT
s32 uiCheckUSBCD(u8 mode)       //from UI task
{
    u8 level,err;
    int status;


    //---------//
    DEBUG_UI("========== uiCheckUSBCD ==========\n\r");
    dcfNewFile = 0;

    level= sysCheckUSBCD();

    if(mode == 1)    //mode=1: 第一次開機.
        UI_USBLastStat = level;
    else  //開機後熱插拔
        level = UI_USBLastStat;

    if(level==USB_CD_OFF)
    {
        DEBUG_UI("USB Remove!!\n\r");
        if(mode) // mode:1 first run on power on
        {
            sysUSBCD_OFF(1);
        }
        else
        {
    	    MemoryFullFlag = FALSE;
        #if USB_HOST_MASS_SUPPORT    
            status = sysSetEvt(SYS_EVT_USBCD_OFF, 0);
        #endif
            if(status)
                OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_WAIT_SET_ALL, OS_IPC_WAIT_FOREVER, &err);
            else
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_SET, &err);
            goto CheckSdCdAbnormal;
        }
        return 0;
    }
    else
    {  
        DEBUG_UI("USB In!!\n\r");
        if(mode)
        {
            status=sysUSBCD_IN(1);
        }
        else
        {
            status = sysSetEvt(SYS_EVT_USBCD_IN, 0);
            if(status)
                OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_WAIT_SET_ALL, OS_IPC_WAIT_FOREVER, &err);
            else
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_SET, &err);
        }
        return 1;
    }

CheckSdCdAbnormal:

#if 0    
    if(sysPlaybackVideoStart==1)
    {
        DEBUG_UI("Abnormal Operation in playback!\n\r");
        uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,138 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
        osdDrawMessage(MSG_ABNORMAL_OPERATION, CENTERED, 110+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
        sysPlaybackVideoStop = 1;
        sysPlaybackVideoPause=0;
        sysPlaybackVideoStart=0;
        timerCountPause(1, 0);
        OSTimeDly(10); // Delay for protect system
        sysPowerOff(0);
    }
#endif

    return 0;
}
#endif


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

    gRfiuUnitCntl[SelChannel].RX_MpegDec_Stop=1;
    OSTimeDly(8);
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

u8 uiRfSuspendDelDecTask_ALL(u8 dummy)
{
    INT8U err;
    int i;

    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    for(i=0;i<MAX_RFIU_UNIT;i++)
        gRfiuUnitCntl[i].RX_MpegDec_Stop=1;
    OSTimeDly(7);
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {       
        if (gRfiu_MpegDec_Sta[i] != RFI_MPEGDEC_TASK_RUNNING)
        {
            gRfiu_MpegDec_Sta[i] = RFI_MPEGDEC_TASK_NONE;
            gRfiuUnitCntl[i].RX_MpegDec_Stop=0;
            continue;
        }

        switch(i)
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
        DEBUG_UI("\n-->Delete RFIU_DEC_TASK_%d\n",i);
        gRfiu_MpegDec_Sta[i] = RFI_MPEGDEC_TASK_NONE;
        gRfiuUnitCntl[i].RX_MpegDec_Stop=0;
    }
    OSSemPost(gRfiuSWReadySemEvt);
    
    return 1;
}

u8 uiRfCreateDecTask(u8 SelChannel)
{
    INT8U err;
    
    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    //if ( (gRfiu_Op_Sta[SelChannel] == RFIU_RX_STA_LINK_OK)  && (gRfiu_WrapDec_Sta[SelChannel] == RFI_WRAPDEC_TASK_RUNNING) && (gRfiu_MpegDec_Sta[SelChannel] == RFI_MPEGDEC_TASK_NONE) )
    if ( (gRfiu_MpegDec_Sta[SelChannel] == RFI_MPEGDEC_TASK_NONE) )
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
    OSSemPost(gRfiuSWReadySemEvt);
    return 1;
}
#if((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)&&USE_NEW_MEMORY_MAP)
s32 uiClearFfQuadBuf(s32 index)
{
    u32  *startAddr,*startAddr_uv, Width, Hight, totalW;
    u32  i,j;
    u32  *tempAddr,*tempAddr_uv;

    Width = RF_RX_2DISP_WIDTH*2;
    Hight = RF_RX_2DISP_HEIGHT;
    totalW = Width*2;
    if(sysCameraMode != SYS_CAMERA_MODE_RF_RX_QUADSCR)
        return 0;


    DEBUG_UI("--->uiClearFfQuadBuf:%d\n",index);

    switch (index)
    {
        case 0:
            startAddr = (u32*)MainVideodisplaybuf[0]+DISPLAY_BUFFER_Y_OFFSET;
            startAddr_uv = (u32*)MainVideodisplaybuf[0]+ULTRA_FHD_SIZE_Y+DISPLAY_BUFFER_UV_OFFSET;
            break;

        case 1:
            startAddr = (u32*)(MainVideodisplaybuf[0] + Width/2)+DISPLAY_BUFFER_Y_OFFSET;
            startAddr_uv = (u32*)(MainVideodisplaybuf[0] + Width/2)+ULTRA_FHD_SIZE_Y+DISPLAY_BUFFER_UV_OFFSET;
            break;

        case 2:
            startAddr = (u32*)(MainVideodisplaybuf[0] + Width*Hight)+DISPLAY_BUFFER_Y_OFFSET;
            startAddr_uv = (u32*)(MainVideodisplaybuf[0] + Width*Hight/2)+ULTRA_FHD_SIZE_Y+DISPLAY_BUFFER_UV_OFFSET;
            break;

        case 3:
            startAddr = (u32*)(MainVideodisplaybuf[0] + Width/2 + Hight*Width)+DISPLAY_BUFFER_Y_OFFSET;
            startAddr_uv = (u32*)(MainVideodisplaybuf[0] + Width/2 + (Hight/2)*Width)+ULTRA_FHD_SIZE_Y+DISPLAY_BUFFER_UV_OFFSET;
            break;

        case 4:  /* Half Mode Left*/
            startAddr = (u32*)MainVideodisplaybuf[0]+DISPLAY_BUFFER_Y_OFFSET;
            startAddr_uv = (u32*)MainVideodisplaybuf[0]+ULTRA_FHD_SIZE_Y+DISPLAY_BUFFER_UV_OFFSET;
            Hight += 240;
            break;

        case 5:  /* Half Mode Right*/
            startAddr = (u32*)(MainVideodisplaybuf[0] + Width/2)+DISPLAY_BUFFER_Y_OFFSET;
            startAddr_uv = (u32*)(MainVideodisplaybuf[0] + Width/2)+ULTRA_FHD_SIZE_Y+DISPLAY_BUFFER_UV_OFFSET;
            Hight += 240;
            break;
        
    }

 #if (CHIP_OPTION  >= CHIP_A1018A)
//    startAddr_uv = startAddr + PNBUF_SIZE_Y;
    Width = Width>>2;
        
    for (i = 0; i < Hight; i++)
    {
        tempAddr = startAddr;
        tempAddr_uv = startAddr_uv;
        for (j = 0; j < Width/2; j++)
        {
            *tempAddr = 0x00000000;
            tempAddr++;
            *tempAddr_uv = 0x80808080;
            tempAddr_uv++;        
        }
        startAddr+=(totalW>>3);
        startAddr_uv+=(totalW>>3);
    }
    //not implement yet!
 #else //YUV422
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
 #endif
}
#else
s32 uiClearFfQuadBuf(s32 index)
{
    u32  i, j, Width, Hight, totalW;
    u32  *startAddr = 0, *startAddr_uv = 0;
    u32  *tempAddr, *tempAddr_uv;

    Width = RF_RX_2DISP_WIDTH;
    Hight = RF_RX_2DISP_HEIGHT;
    totalW = RF_RX_2DISP_WIDTH;

    if(sysCameraMode != SYS_CAMERA_MODE_RF_RX_QUADSCR)
        return 0;

    DEBUG_RED("--->uiClearFfQuadBuf:%d\n",index);

    switch (index)
    {
        case 0:
            startAddr = (u32*)MainVideodisplaybuf[0];
            startAddr_uv = startAddr + PNBUF_SIZE_Y/4;
            break;

        case 1:
//            startAddr = (u32*)(MainVideodisplaybuf[0] + Width/2);
//            startAddr_uv = (u32*)(MainVideodisplaybuf[0] + Width/2) + PNBUF_SIZE_Y/4;
            startAddr = (u32*)(MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH);
            startAddr_uv = (u32*)(MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH) + PNBUF_SIZE_Y/4;
            break;

        case 2:
//            startAddr = (u32*)(MainVideodisplaybuf[0] + Width*Hight);
//            startAddr_uv = (u32*)(MainVideodisplaybuf[0] + Width*Hight/2) + PNBUF_SIZE_Y/4;
            startAddr = (u32*)(MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*RF_RX_2DISP_HEIGHT*2);
            startAddr_uv = (u32*)(MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*RF_RX_2DISP_HEIGHT) + PNBUF_SIZE_Y/4;
            break;

        case 3:
//            startAddr = (u32*)(MainVideodisplaybuf[0] + Width/2 + Hight*Width);
//            startAddr_uv = (u32*)(MainVideodisplaybuf[0] + Width/2 + (Hight/2)*Width) + PNBUF_SIZE_Y/4;
            startAddr = (u32*)(MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_WIDTH*RF_RX_2DISP_HEIGHT*2);
            startAddr_uv = (u32*)(MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_WIDTH*RF_RX_2DISP_HEIGHT) + PNBUF_SIZE_Y/4;
            break;

        case 4:  /* Half Mode Left*/
            startAddr = (u32*)MainVideodisplaybuf[0];
            startAddr_uv = startAddr + PNBUF_SIZE_Y/4;
            Hight += 240;
            break;

        case 5:  /* Half Mode Right*/
            startAddr = (u32*)(MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH);
            startAddr_uv = startAddr + PNBUF_SIZE_Y/4;
            Hight += 240;
            break;
    }

 #if (CHIP_OPTION  >= CHIP_A1018A)
//    startAddr_uv = startAddr + PNBUF_SIZE_Y;
    Width = Width>>2;
    for (i = 0; i < Hight; i++)
    {
        tempAddr = startAddr;
        tempAddr_uv = startAddr_uv;
        for (j = 0; j < Width; j++)
        {
            *tempAddr = 0x00000000;
            tempAddr++;
            *tempAddr_uv = 0x80808080;
            tempAddr_uv++;        
        }
        startAddr+=(totalW>>1);
        startAddr_uv+=(totalW>>1);
    }
    //not implement yet!
 #else //YUV422
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
 #endif
 return 0;
}
#endif
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
      #if((AUDIO_OPTION == AUDIO_ADC_DAC)  && (AUDIO_DEVICE == AUDIO_IIS_WM8940))
      if( sysTVOutOnFlag )
          WM8940_PLL_AuxPlay_ALL();
      else        
          WM8940_MicRec_AuxPlay_Switch2panel();
      #endif
        rfiu_AudioRetONOFF_IIS(0);
        DEBUG_GPIO("Talkint Key Off!\n\r");

        return 0;
    }
    else if(rfiuAudioRetStatus==RF_AUDIO_RET_OFF)
    {
      #if((AUDIO_OPTION == AUDIO_ADC_DAC)  && (AUDIO_DEVICE == AUDIO_IIS_WM8940))
        Init_IIS_WM8940_MicRec_MonoPlay();
      #endif
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

    //DEBUG_UI("cam %d uiSetRfResolutionRxToTx %d\r\n",camera,setting);

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (uiP2PMode == 0)
    {
        UI_CFG_RES[camera] = setting;
        Save_UI_Setting();
    }
    UI_P2P_RES[camera] = setting;
    
    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;

    switch (setting)
    {
        case UI_RESOLTUION_VGA:
            if((gRfiuUnitCntl[camera].TX_PicWidth == 640) && (gRfiuUnitCntl[camera].TX_PicHeight == 480))
                return UI_SET_RF_OK;
            sprintf((char*)uartCmd,"RESO 640 480");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 640;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 480;
        #endif
            break;
        
        case UI_RESOLTUION_HD:
            if ( ((gRfiuUnitCntl[camera].TX_Status & RFIU_TX_STA__HD_SUPPORT) == 0) && (((gRfiuUnitCntl[camera].TX_Status & RFIU_TX_STA__FULLHD_SUPPORT) == 0)))
                return UI_SET_RF_OK;
            if((gRfiuUnitCntl[camera].TX_PicWidth == 1280) && (gRfiuUnitCntl[camera].TX_PicHeight == 720))
                return UI_SET_RF_OK;
            sprintf((char*)uartCmd,"RESO 1280 720");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 1280;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 720;
        #endif
            break;

        case UI_RESOLTUION_QVGA:
            if((gRfiuUnitCntl[camera].TX_PicWidth == 320) && (gRfiuUnitCntl[camera].TX_PicHeight == 240))
                return UI_SET_RF_OK;
            sprintf((char*)uartCmd,"RESO 320 240");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 320;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 240;
        #endif
            break;

        case UI_RESOLUTION_D1_480V:
            if((gRfiuUnitCntl[camera].TX_PicWidth == 704) && (gRfiuUnitCntl[camera].TX_PicHeight == 480))
                return UI_SET_RF_OK;
            sprintf((char*)uartCmd,"RESO 704 480");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 704;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 480;
        #endif
            break;

        case UI_RESOLUTION_D1_576V:
            if((gRfiuUnitCntl[camera].TX_PicWidth == 704) && (gRfiuUnitCntl[camera].TX_PicHeight == 576))
                return UI_SET_RF_OK;
            sprintf((char*)uartCmd,"RESO 704 576");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 704;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 576;
        #endif
            break;

        case UI_RESOLTUION_QHD:
            if ( ((gRfiuUnitCntl[camera].TX_Status & RFIU_TX_STA__HD_SUPPORT) == 0) && (((gRfiuUnitCntl[camera].TX_Status & RFIU_TX_STA__FULLHD_SUPPORT) == 0)))
                return UI_SET_RF_OK;
            if((gRfiuUnitCntl[camera].TX_PicWidth == 640) && (gRfiuUnitCntl[camera].TX_PicHeight == 352))
                return UI_SET_RF_OK;
            sprintf((char*)uartCmd,"RESO 640 352");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 640;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 352;
        #endif
            break;

        case UI_RESOLTUION_FHD:
            if ( ((gRfiuUnitCntl[camera].TX_Status & RFIU_TX_STA__HD_SUPPORT) != 0) && (((gRfiuUnitCntl[camera].TX_Status & RFIU_TX_STA__FULLHD_SUPPORT) != 0)))
            {

            }
            else
            {
                return UI_SET_RF_OK;
            }
            if((gRfiuUnitCntl[camera].TX_PicWidth == 1920) && ((gRfiuUnitCntl[camera].TX_PicHeight == 1072) ||(gRfiuUnitCntl[camera].TX_PicHeight == 1088) ))
                return UI_SET_RF_OK;
            sprintf((char*)uartCmd,"RESO 1920 1072");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 1920;
            if(gRfiuUnitCntl[camera].TX_PicHeight == 1072)
                VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 1072;
            else
                VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 1080;
        #endif
            break;

        case UI_RESOLTUION_4M:
            if ( ((gRfiuUnitCntl[camera].TX_Status & RFIU_TX_STA__HD_SUPPORT) != 0) && (((gRfiuUnitCntl[camera].TX_Status & RFIU_TX_STA__FULLHD_SUPPORT) != 0)))
            {

            }
            else
            {
                return UI_SET_RF_OK;
            }
            if((gRfiuUnitCntl[camera].TX_PicWidth == 2688) && (gRfiuUnitCntl[camera].TX_PicHeight == 1520))
                return UI_SET_RF_OK;
            sprintf((char*)uartCmd,"RESO 2688 1520");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 2688;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 1520;
        #endif
            break;
        
        case UI_RESOLTUION_1600x896:
            if ( (((gRfiuUnitCntl[camera].TX_Status & RFIU_TX_STA__FULLHD_SUPPORT) != 0)))
            {

            }
            else
            {
                return UI_SET_RF_OK;
            }
            if((gRfiuUnitCntl[camera].TX_PicWidth == 1600) && (gRfiuUnitCntl[camera].TX_PicHeight == 896))
                return UI_SET_RF_OK;
            sprintf((char*)uartCmd,"RESO 1600 896");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 1600;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 896;
        #endif
            break;
    }

#if MULTI_CHANNEL_VIDEO_REC
    RecStatus = MultiChannelGetCaptureVideoStatus(camera+MULTI_CHANNEL_LOCAL_MAX);
    if (RecStatus != 0)
        uiCaptureVideoStopByChannel(camera);
#endif
    cnt=0;
    RfBusy=1;

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
    return UI_SET_RF_OK;
}

u8 uiSetRfBrightnessRxToTx(s8 brivalue,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    //DEBUG_UI("cam %d uiSetRfBrightnessRxToTx %d day level %d night level %d\r\n",camera, brivalue, dayLev, nightLev);
    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiuUnitCntl[camera].RFpara.TX_SensorBrightness == brivalue)
        return UI_SET_RF_NO_NEED;

    sprintf((char*)uartCmd,"BRIT %d", brivalue);
    CurMotEnb[camera] = brivalue;

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfBrightnessRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    gRfiuUnitCntl[camera].RFpara.TX_SensorBrightness = brivalue;
    
    return UI_SET_RF_OK;
}

u8 uiSetSensorContrastRxToTx(s8 level,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;

    sprintf((char*)uartCmd,"CONT %d", level);

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetSensorContrastRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    gRfiuUnitCntl[camera].RFpara.TX_SensorContrast= level;
    
    return UI_SET_RF_OK;
}

u8 uiSetSensorSatConShpRxToTx(s8 Sat,s8 Con,s8 Shp,s8 FlipMirr,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;

    sprintf((char*)uartCmd,"SATU %d %d %d %d", Sat,Con,Shp,FlipMirr);

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetSensorSatConShpRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    gRfiuUnitCntl[camera].RFpara.TX_SensorSaturation= Sat;
    gRfiuUnitCntl[camera].RFpara.TX_SensorContrast= Con;
    gRfiuUnitCntl[camera].RFpara.TX_SensorSharpness= Shp;
    gRfiuUnitCntl[camera].RFpara.TX_SensorFlipMirr= FlipMirr;
    return UI_SET_RF_OK;
}

u8 uiSetSensorSharpnessRxToTx(s8 level,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;

    sprintf((char*)uartCmd,"SHRP %d", level);

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetSensorSharpnessRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    gRfiuUnitCntl[camera].RFpara.TX_SensorSharpness= level;
    
    return UI_SET_RF_OK;
}

u8 uiSetRfMotionRxToTx(s8 Enable,u8 dayLev,u8 nightLev,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    //DEBUG_UI("cam %d uiSetRfMotionRxToTx %d day level %d night level %d\r\n",camera, Enable, dayLev, nightLev);
    //DEBUG_UI("cam %d uiSetRfMotionRxToTx %d day level %d night level %d\r\n",camera, gRfiuUnitCntl[camera].RFpara.MD_en,gRfiuUnitCntl[camera].RFpara.MD_Level_Day, gRfiuUnitCntl[camera].RFpara.MD_Level_Night);

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;

    if ((gRfiuUnitCntl[camera].RFpara.MD_en == Enable) && (gRfiuUnitCntl[camera].RFpara.MD_Level_Day == dayLev)&&
        (gRfiuUnitCntl[camera].RFpara.MD_Level_Night == nightLev))
        return UI_SET_RF_NO_NEED;

    sprintf((char*)uartCmd,"MDCFG %d %d %d", Enable, dayLev, nightLev);
    CurMotEnb[camera] = Enable;
    CurMotdayLev[camera] = dayLev;
    CurMotnightLev[camera] = nightLev;

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfMotionRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }

    gRfiuUnitCntl[camera].RFpara.MD_en = Enable;
    gRfiuUnitCntl[camera].RFpara.MD_Level_Day = dayLev;
    gRfiuUnitCntl[camera].RFpara.MD_Level_Night = nightLev;
    
    return UI_SET_RF_OK;
}

u8 uiSetRfPIRRxToTx(s8 Enable, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    extern s8  isPIRsenSent[MAX_RFIU_UNIT];
    //DEBUG_UI("cam %d uiSetRfPIRRxToTx %d \r\n",camera, Enable);

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiuUnitCntl[camera].RFpara.PIR_en == Enable)
        return UI_SET_RF_OK;
    
    if (isPIRsenSent[camera] == 1)
        return UI_SET_RF_OK;

    sprintf((char*)uartCmd,"PIRCFG %d", Enable);
        
    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfPIRRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }

    gRfiuUnitCntl[camera].RFpara.PIR_en = Enable;
    isPIRsenSent[camera] = 1;
    return UI_SET_RF_OK;
}

u8 uiSetRfTimeStampOnRxToTx(s8 Enable, u8 camera)
{
    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    gRfiuUnitCntl[camera].RFpara.TX_TimeStampOn = Enable;
    if(rfiu_UpdateTXOthersPara(camera) == 0)
        return UI_SET_RF_BUSY;
    else
        return UI_SET_RF_OK;
}

u8 uiSetRfTimeStampTypeRxToTx(u8 Type, u8 camera)
{
    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    gRfiuUnitCntl[camera].RFpara.TX_TimeStampType = Type;
    if(rfiu_UpdateTXOthersPara(camera) == 0)
        return UI_SET_RF_BUSY;
    else
        return UI_SET_RF_OK;
}

u8 uiSetRfSubStreamQualityRxToTx(u8 Sel, u8 camera)
{
    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    gRfiuUnitCntl[camera].RFpara.TX_SubStreamBRSel = Sel;
    if(rfiu_UpdateTXOthersPara(camera) == 0)
        return UI_SET_RF_BUSY;
    else
        return UI_SET_RF_OK;
}

u8 uiSetRfFlickerRxToTx(u8 setting)
{
    u8 i;
    int RfBusy = 1, cnt = 0;
    u8 uartCmd[20];  

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;


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
        for (i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            cnt=0;
            //DEBUG_UI("========================> cam%d !!!\r\n",i);
            if (((rfiuRX_CamOnOff_Sta >> i) & 0x01) == 0)
                continue;
            
            if (gRfiu_Op_Sta[i] != RFIU_RX_STA_LINK_OK)
                continue;
            
            if (gRfiuUnitCntl[i].RFpara.TxSensorAE_FlickSetting == setting)
                continue;
            while (RfBusy != 0)
            {
                if (cnt >UI_SET_FR_CMD_RETRY)
                {
                    DEBUG_UI("uiMenuSet_TVout_Format Timeout !!!\r\n");
                    return UI_SET_RF_BUSY;
                }
                RfBusy = rfiu_RXCMD_Enc(uartCmd, i);
                cnt++;
                OSTimeDly(1);
            }

            if (RfBusy==0)
               gRfiuUnitCntl[i].RFpara.TxSensorAE_FlickSetting = setting;
            RfBusy = 1;
        }
    }

    return UI_SET_RF_OK;
    
}



s32 uiSetRfTimeRxToTx(s32 camera)
{
    u8 uartCmd[32];
    int RfBusy = 1, cnt = 0;
    RTC_DATE_TIME   localTime;
    
    //DEBUG_UI("cam %d uiSetRfTimeRxToTx \r\n",camera);
    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;

    RTC_Get_Time(&localTime);
    sprintf((char*)uartCmd,"TIME %d/%d/%d %d:%d:%d", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfTimeRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }

    
    return UI_SET_RF_OK;
}






u8 uiSetRfUnlockRxToTx(s8 Enable, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfUnlockRxToTx %d \r\n",camera, Enable);

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;
    
    sprintf((char*)uartCmd,"SETGPO %d", Enable);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfUnlockRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return UI_SET_RF_OK;
}

u8 uiSetRfLightingRxToTx(u8 Enable, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfLightingRxToTx %d \r\n",camera, Enable);

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;
    
    Enable &= 0x01;
    sprintf((char*)uartCmd,"SETPWM %d", Enable);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfLightingRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        if(rfiu_RXCMD_Enc(uartCmd, camera) == 0) 
            RfBusy = UI_SET_RF_OK;
        else
            RfBusy = UI_SET_RF_BUSY;
        cnt++;
        OSTimeDly(1);
    }
    return UI_SET_RF_OK;
}

u8 uiSetRfLightDurationRxToTx(u8 Val, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;
    
    DEBUG_UI("cam %d uiSetRfLightDurationRxToTx %d \r\n",camera, Val);
    Val = ((Val << 1) | 0x10);
    sprintf((char*)uartCmd,"SETPWM %d", Val);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfLightDurationRxToTx Timeout !!!\r\n");
            uiSetRfLightDur[camera] = RfBusy;
            return UI_SET_RF_BUSY;
        }
        if(rfiu_RXCMD_Enc(uartCmd, camera) == 0) 
            RfBusy = UI_SET_RF_OK;
        else
            RfBusy = UI_SET_RF_BUSY;
        cnt++;
        OSTimeDly(1);
    }
    uiSetRfLightDur[camera] = RfBusy;
    return UI_SET_RF_OK;
}

#if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
u8 uiSetRfLightTimerRxToTx(u8 Hour1, u8 Min1, u8 Hour2, u8 Min2, u8 Week, u8 camera, u8 isSyn)
{
    DEBUG_UI("cam %d uiSetRfLightTimerRxToTx %02d %02d %02d %02d %d %d\r\n",camera, Hour1, Min1, Hour2, Min2, Week, isSyn);

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;
    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;

    if(rfiu_SetTXSchedule(camera, Hour1, Min1, Hour2, Min2, Week, isSyn) == 0)
        uiSetRfLightTimer[camera] = UI_SET_RF_BUSY;
    else
        uiSetRfLightTimer[camera] = UI_SET_RF_OK;

    return uiSetRfLightTimer[camera];
 }
#elif (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
u8 uiSetRfLightTimerRxToTx(u8 week, u8 part1, u8 part2, u8 part3, u8 part4, u8 part5, u8 part6, u8 camera, u8 u8Enable)
{

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;
    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;
    //DEBUG_UI("cam %d week %d uiSetRfLightTimerRxToTx %x %x %x %x %x %x syn %d\r\n",camera, week, part1, part2, part3, part4, part5, part6, u8Enable);

    if(rfiu_SetTXSchedule(camera, week, part1, part2, part3, part4, part5, part6, u8Enable) == 0)
        uiSetRfLightTimer[camera][week] = UI_SET_RF_BUSY;
    else
        uiSetRfLightTimer[camera][week] = UI_SET_RF_OK;

    return uiSetRfLightTimer[camera][week];
}
#endif

u8 uiSetRfLightDimmerRxToTx(u8 Val, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;
    
    DEBUG_UI("cam %d uiSetRfLightDimmerRxToTx %d \r\n",camera, Val);
    Val = ((Val << 5) | 0x80);
    sprintf((char*)uartCmd,"SETPWM %d", Val);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfLightDimmerRxToTx Timeout !!!\r\n");
            uiSetRfLightDimmer[camera] = RfBusy;
            return UI_SET_RF_BUSY;
        }
        if(rfiu_RXCMD_Enc(uartCmd, camera) == 0) 
            RfBusy = UI_SET_RF_OK;
        else
            RfBusy = UI_SET_RF_BUSY;
        cnt++;
        OSTimeDly(1);
    }
    uiSetRfLightDimmer[camera] = RfBusy;
    return UI_SET_RF_OK;
}

#if (UI_VERSION == UI_VERSION_MAYON)
u8 uiSetRfManualLightingRxToTx(u8 Enable, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfManualLightingRxToTx %d \r\n",camera, Enable);

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;
    
    sprintf((char*)uartCmd,"SETGPO %d", Enable);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfManualLightingRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        if(rfiu_RXCMD_Enc(uartCmd, camera) == 0) 
            RfBusy = UI_SET_RF_OK;
        else
            RfBusy = UI_SET_RF_BUSY;
        cnt++;
        OSTimeDly(1);
    }
    return RfBusy;
}
#else
u8 uiSetRfManualLightingRxToTx(u8 Enable, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfManualLightingRxToTx %d \r\n",camera, Enable);

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;
    
    sprintf((char*)uartCmd,"SETGPO %d", Enable);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfManualLightingRxToTx Timeout !!!\r\n");
            uiSetRfLightState[camera] = RfBusy;
            return UI_SET_RF_BUSY;
        }
        if(rfiu_RXCMD_Enc(uartCmd, camera) == 0) 
            RfBusy = UI_SET_RF_OK;
        else
            RfBusy = UI_SET_RF_BUSY;
        cnt++;
        OSTimeDly(1);
    }
    uiSetRfLightState[camera] = RfBusy;
    return UI_SET_RF_OK;

}
#endif

u8 uiSetRfNumberRxToTx(u8 Number, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfNumberRxToTx %d \r\n",camera, Number);

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;
    
    sprintf((char*)uartCmd,"SETGPO %d", Number);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfNumberRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return UI_SET_RF_OK;

}





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
    u8  uartCmd[16];
    int RfBusy = 1, cnt = 0;
    s8  SetRes;

    DEBUG_UI("uiSetP2PStatueToRF cam %d level %d\r\n",CamId, level);

    if (Main_Init_Ready == 0)
        return;

    if (((rfiuRX_CamOnOff_Sta >> CamId) & 0x01) == 0)
        return;

    switch (level)
    {
        case 0: /*P2P disconnect*/
            uiP2PRestoreCfgTime = UI_P2P_RESTORT_WAIT_TIME;
            //prev_P2pVideoQuality=2;
            break;

        /*App Set level 1~3*/
        case 1:
        case 2:
        case 3:
            #if( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
            if(sysTVOutOnFlag)
            {
                if(level == 1 && level != rfiuRX_P2pVideoQuality) //App切換resolution,從med/low切到FHD
                {
                    DEBUG_UI("UI level FHD %d rfiuRX_P2pVideoQuality org HD/QHD %d\n", level, rfiuRX_P2pVideoQuality);
                    iduTVColorbar_onoff(1);
                    //iduTVOSDDisable(IDU_OSD_L0_WINDOW_0);
                }
                else if(rfiuRX_P2pVideoQuality == 1 && level != 1)//App切換resolution,FHD切到非FHD
                {
                    DEBUG_UI("UI level HD/QHD %d rfiuRX_P2pVideoQuality org FHD %d\n", level, rfiuRX_P2pVideoQuality);
                    iduTVColorbar_onoff(1);
                    //iduTVOSDDisable(IDU_OSD_L0_WINDOW_0);
                }
            }
            #endif
            rfiuRX_P2pVideoQuality = level;   
            uiP2PMode = level;
            rfiuRX_OpMode |= RFIU_RX_OPMODE_P2P;
            if(gOnlineNum > 1)
            {
                rfiuRX_OpMode |= RFIU_RX_OPMODE_P2P_MULTI;
            }
            else
            {
                rfiuRX_OpMode &= (~RFIU_RX_OPMODE_P2P_MULTI);
            }
            sprintf((char*)uartCmd,"MODE %d %d", rfiuRX_OpMode, rfiuRX_P2pVideoQuality);
            prev_P2pVideoQuality=rfiuRX_P2pVideoQuality;
            iconflag[UI_MENU_SETIDX_P2P_LEVEL] = level;
            Save_UI_Setting();
            while(RfBusy != 0)
            {
                RfBusy = rfiu_RXCMD_Enc(uartCmd, CamId);
                if (cnt >UI_SET_FR_CMD_RETRY)
                {
                    DEBUG_UI("uiSetP2PStatueToRF Timeout:%d !!!\n",RfBusy);
                    rfiuRX_OpModeCmdRetry[CamId]=1;
                    return;
                }
                cnt++;
                OSTimeDly(1);
            }

            #if( (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
                if(level == 1)
                    SetRes = UI_RESOLTUION_FHD;
                else
                    SetRes = UI_RESOLTUION_HD;
                uiSetRfResolutionRxToTx(SetRes, CamId);
            #endif
            uiP2PRestoreCfgTime = 0;
            break;

            
        case 6: /*App connect in normal mode*/
            uiP2PMode = level;
            rfiuRX_OpMode |= RFIU_RX_OPMODE_P2P;
            if(gOnlineNum > 1)
            {
                rfiuRX_OpMode |= RFIU_RX_OPMODE_P2P_MULTI;
            }
            else
            {
                rfiuRX_OpMode &= (~RFIU_RX_OPMODE_P2P_MULTI);
            }
            
            rfiuRX_P2pVideoQuality=prev_P2pVideoQuality;
            sprintf((char*)uartCmd,"MODE %d %d", rfiuRX_OpMode, rfiuRX_P2pVideoQuality);
            while(RfBusy != 0)
            {
                RfBusy = rfiu_RXCMD_Enc(uartCmd, CamId);
                if (cnt >UI_SET_FR_CMD_RETRY)
                {
                    DEBUG_UI("uiSetP2PStatueToRF Timeout:%d !!!\n",RfBusy);
                    rfiuRX_OpModeCmdRetry[CamId]=1;
                    return;
                }
                cnt++;
                OSTimeDly(1);
            }
            uiP2PRestoreCfgTime = 0;
            break;

        case 7:
            uiP2PMode = level;
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
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

    if (((rfiuRX_CamOnOff_Sta >> camera) & 0x01) == 0)
        return UI_SET_RF_NO_NEED;

    if (Type == 0)  /*HD*/
    {
        sprintf((char*)uartCmd,"MASKAREA_HD");
        for ( i = 0; i < 9; i++)
        {
            sprintf((char*)tmpCmd," %d", UI_HD_MaskArea[camera][i]);
            strcat((char*)uartCmd, (char*)tmpCmd);
        }        
    }
    else
    {
        sprintf((char*)uartCmd,"MASKAREA_VGA");
        for ( i = 0; i < 9; i++)
        {
            sprintf((char*)tmpCmd," %d", UI_VGA_MaskArea[camera][i]);
            strcat((char*)uartCmd, (char*)tmpCmd);
        }        
    }
    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfMotionMaskArea Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return UI_SET_RF_OK;
}

u8 uiSetRfDisplayMode(u8 setting)
{
    int count;
    u8  err, i;
  #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
    u8  temp;
  #endif
  #if RFIU_RX_SHOW_ONLY
    char Rfcmd[32];
  #endif
    OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_UI("uiSetRfDisplayMode %d \r\n",setting);
    UISetRFMode = setting;
    switch (setting)
    {
        case UI_MENU_RF_QUAD:
            sysCameraMode = SYS_CAMERA_MODE_RF_RX_QUADSCR;
          #if (RFIU_RX_SHOW_ONLY && (SW_APPLICATION_OPTION == MR8110_BABYMONITOR))
            for(i=0;i<MAX_RFIU_UNIT;i++)
               iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i]=UI_MENU_SETTING_RESOLUTION_QHD;
          #endif
   
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            uiRfSuspendDelDecTask_ALL(0);
            sysRFRXPIP_en=0;
            uiEnZoom=0;
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

            DEBUG_UI("sysRFRxInMainCHsel=%d\n",sysRFRxInMainCHsel);
          #if RFRX_HALF_MODE_SUPPORT
            if(sysTVOutOnFlag)
            {
            #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                sysTVswitchResolutionbyImagesize();
            #endif
            }
            if(rfiuRX_CamOnOff_Num <= 2)
            {
              if(sysTVOutOnFlag)
                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT_TV*2,RF_RX_2DISP_WIDTH*2);
              else
                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT*2,RF_RX_2DISP_WIDTH*2);
            }
            else
            {
              #if( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
              if(sysTVOutOnFlag)
              {
                if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                else
                  iduPlaybackMode(1920,1080,1920);
//                #if (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I60)
//                iduPlaybackMode(1920,1080,1920);
//                #else
//                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
//                #endif
              }
              else
                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
              #else
              iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
              #endif
            }
          #else
              iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
          #endif
          
          #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
		  #if(USE_NEW_MEMORY_MAP)
		  Idu_ClearBuf_HD_IN_ULTRA_FHD(0);
		  #else
          memDispBufArrage(DISPBUF_NORMAL);
          Idu_ClearBuf(1);		  
		  #endif
		  #else
          iduTVColorbar_onoff(1);    
          Idu_ClearBuf(1);
          iduTVColorbar_onoff(0);    
          #endif
          
            rfiuRX_OpMode= (rfiuRX_OpMode | RFIU_RX_OPMODE_QUAD | RfIU_RX_OPMODE_FORCE_I) & (~RFIU_RX_OPMODE_PIP);
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
            rfiuRX_OpMode = rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);

            count   = 0;
            while(count < MAX_RFIU_UNIT)
            {
                uiRfCreateDecTask(count);
                count ++;
            }            
            break;

        case UI_MENU_RF_DUAL:
            sysCameraMode = SYS_CAMERA_MODE_RF_RX_DUALSCR;

            rfiuRX_OpMode= rfiuRX_OpMode & ( ~(RFIU_RX_OPMODE_QUAD | RFIU_RX_OPMODE_P2P | RFIU_RX_OPMODE_PIP | RFIU_RX_OPMODE_P2P_MULTI) );
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            iduTVColorbar_onoff(1); 
            Idu_ClearBuf(DISPLAY_BUF_NUM);
            iduTVColorbar_onoff(0); 

    	 #if( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) )
            SYS_ANA_TEST2 |= 0x08; //swith TV DAC to subTV controller.
	     #endif
            subTV_Preview(640,480);
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                    marsDMAClose(guiIISCh0RecDMAId);
                    guiIISCh0RecDMAId = 0xFF;
                }
            #endif
            //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.

            count   = 0;
            while(count < MAX_RFIU_UNIT)
            {
                uiRfCreateDecTask(count);
                count ++;
            }
            break;

        case UI_MENU_RF_FULL:
    #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
        case UI_MENU_RF_PIPSWAP:
    #endif

            for ( i = 0; i < MAX_RFIU_UNIT; i++)
                uiRFStatue[i] = UI_RF_STATUS_OTHER;

            
          #if (RFIU_RX_SHOW_ONLY && (SW_APPLICATION_OPTION == MR8110_BABYMONITOR))
            for(i=0;i<MAX_RFIU_UNIT;i++)
               iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i]=UI_MENU_SETTING_RESOLUTION_HD;
          #endif
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            uiRfSuspendDelDecTask_ALL(0);
            //uiEnZoom=0;
    #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            if(setting==UI_MENU_RF_PIPSWAP)
            {
               temp=sysRFRxInMainCHsel;
               sysRFRxInMainCHsel=sysRFRxInPIPCHsel;
               sysRFRxInPIPCHsel=temp;
            }
    #endif

            sysCameraMode = SYS_CAMERA_MODE_RF_RX_FULLSCR;

            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                    marsDMAClose(guiIISCh0RecDMAId);
                    guiIISCh0RecDMAId = 0xFF;
                }
            #endif

            if(sysRFRxInMainCHsel==0xff)
            {
                sysRFRxInMainCHsel=0; 
                count=0;
                while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=MAX_RFIU_UNIT) )
                {                  
                    sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                    count ++;
                }
            }
            
            
          #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
            {
               #if(USE_NEW_MEMORY_MAP)
               Idu_ClearBuf_FHD_IN_ULTRA_FHD(0);			  
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
			   #else
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
			   Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
			   #endif
            }
            else if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==0) && (iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+sysRFRxInMainCHsel]==UI_MENU_SETTING_RESOLUTION_1920x1072) )
            {
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
            }
            else
            {
               #if(USE_NEW_MEMORY_MAP)
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
			   Idu_ClearBuf_HD_IN_ULTRA_FHD(0);
			   #else
               memDispBufArrage(DISPBUF_NORMAL);
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
			   Idu_ClearBuf(1);
			   #endif
            }
          #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
            if(sysTVOutOnFlag)
            {
                //gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
            #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                sysTVswitchResolutionbyImagesize();
            #else
                if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                else
                  iduPlaybackMode(1920,1080,1920);
            #endif

                Idu_ClearBuf(1);               
                //gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
            }
            else
            {
                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                iduTVColorbar_onoff(1); 
                Idu_ClearBuf(1);
                iduTVColorbar_onoff(0); 
            }            
          #elif(SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1)
            Idu_ClearBuf(1);
          #else
            iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            Idu_ClearBuf(1);
          #endif
          
            uiRfCreateDecTask(sysRFRxInMainCHsel);
            //OSTimeDly(1);

            if(sysRFRXPIP_en)
               rfiuRX_OpMode= rfiuRX_OpMode | (RFIU_RX_OPMODE_QUAD | RfIU_RX_OPMODE_FORCE_I | RFIU_RX_OPMODE_PIP);
            else
               rfiuRX_OpMode= (rfiuRX_OpMode | RfIU_RX_OPMODE_FORCE_I) & ( ~(RFIU_RX_OPMODE_QUAD | RFIU_RX_OPMODE_PIP) );
            
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
            rfiuRX_OpMode = rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);
            

            if(sysRFRXPIP_en)
            {

                if( (sysRFRxInPIPCHsel==sysRFRxInMainCHsel) || ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInPIPCHsel) & 0x01) == 0) )
                {
                    count=0;
                    do
                    {
                        do
                        {
                            sysRFRxInPIPCHsel = (sysRFRxInPIPCHsel+1) % MAX_RFIU_UNIT;
                            count ++;
                        }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInPIPCHsel) & 0x01) == 0) && (count<=MAX_RFIU_UNIT) );
                    }while( (sysRFRxInMainCHsel==sysRFRxInPIPCHsel) && (sysRFRXPIP_en==1) && (count<=MAX_RFIU_UNIT) );

                }

                if( (sysRFRxInPIPCHsel !=sysRFRxInMainCHsel) && ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInPIPCHsel) & 0x01) != 0) )
                {
                   DEBUG_UI("PIP CH-%d is eanble\n",sysRFRxInPIPCHsel);
                   uiRfCreateDecTask(sysRFRxInPIPCHsel);
                }
                else
                {
                   DEBUG_UI("PIP CH is not enough!\n");
                }

            }

            break;

   	    case UI_MENU_RF_ENTER_SETUP:
			      sysCameraMode = SYS_CAMERA_MODE_UI;
					
			      count=0;
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            for ( i = 0; i < MAX_RFIU_UNIT; i++)
                uiRFStatue[i] = UI_RF_STATUS_OTHER;
            rfiuRX_OpMode= rfiuRX_OpMode & ( ~(RFIU_RX_OPMODE_QUAD | RFIU_RX_OPMODE_PIP) );
			   
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
       
            uiRfSuspendDelDecTask_ALL(0);
            
            OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.

            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                    marsDMAClose(guiIISCh0RecDMAId);
                    guiIISCh0RecDMAId = 0xFF;
                }
            #endif

            #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
            #if((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)&&(USE_NEW_MEMORY_MAP))
		    Idu_ClearBuf_HD_IN_ULTRA_FHD(0);
			#else
            Idu_ClearBuf(6);
			#endif
            #endif
		 	break;

        case UI_MENU_RF_ENTER_PLAYBACK:
       
            
            sysCameraMode = SYS_CAMERA_MODE_PLAYBACK;
            count=0;
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            for ( i = 0; i < MAX_RFIU_UNIT; i++)
                uiRFStatue[i] = UI_RF_STATUS_OTHER;
            rfiuRX_OpMode= rfiuRX_OpMode & ( ~(RFIU_RX_OPMODE_QUAD | RFIU_RX_OPMODE_PIP) );
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
            
            uiRfSuspendDelDecTask_ALL(0);

            OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.

            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                    marsDMAClose(guiIISCh0RecDMAId);
                    guiIISCh0RecDMAId = 0xFF;
                }
            #endif

            #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
			#if((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)&&USE_NEW_MEMORY_MAP)
			Idu_ClearBuf_HD_IN_ULTRA_FHD(0);
			#else
            Idu_ClearBuf(1);
			#endif
            #endif
            break;

       case UI_MENU_RF_ENTER_MASKAREA:
            sysCameraMode = SYS_CAMERA_MODE_RF_RX_MASKAREA;
            for ( i = 0; i < MAX_RFIU_UNIT; i++)
                uiRFStatue[i] = UI_RF_STATUS_OTHER;

            rfiuRX_OpMode= (rfiuRX_OpMode | RFIU_RX_OPMODE_QUAD) & (~RFIU_RX_OPMODE_PIP);
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);

            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            uiRfSuspendDelDecTask_ALL(0);
            if(sysTVOutOnFlag)
            {
            	#if((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)&&USE_NEW_MEMORY_MAP)
				#else
                if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
                {
                   iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                }
                else
                   iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
				#endif
            }
            else
            {
                #if(LCM_OPTION == VGA_1024X768_60HZ)
                    iduPlaybackMode(1024,720,RF_RX_2DISP_WIDTH*2);
                #else
                    iduPlaybackMode(1024,600,RF_RX_2DISP_WIDTH*2);
                #endif
            }
        #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            memDispBufArrage(DISPBUF_NORMAL);
        #endif
	        #if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)&&(USE_NEW_MEMORY_MAP))
			Idu_ClearBuf_HD_IN_ULTRA_FHD(0);
			#else
            Idu_ClearBuf(1);
			#endif


            //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.

            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                    marsDMAClose(guiIISCh0RecDMAId);
                    guiIISCh0RecDMAId = 0xFF;
                }
            #endif
            uiRfCreateDecTask(sysRFRxInMainCHsel);  

            rfiuRX_OpMode = rfiuRX_OpMode | RfIU_RX_OPMODE_FORCE_I;
            rfiu_SetRXOpMode_1(sysRFRxInMainCHsel);
            rfiuRX_OpMode = rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);

            MainVideodisplaybuf_idx=0;
            break;

        default:
            DEBUG_UI("Error setting in  %d uiSetRfDisplayMode\r\n",setting);
            break;
    }
    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_CLR, &err);
    return 1;
}

u8 uiSetRfChangeChannel(u8 setting)
{
    int count;
    u8 err, bRet=0;
    
    DEBUG_UI("uiSetRfChangeChannel %d\r\n",setting);
    //uiEnZoom=0;

    switch (setting)
    {
        case UI_MENU_CHANNEL_ADD:
            iduTVColorbar_onoff(1);
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
              rfiu_AudioRetONOFF_IIS(0);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;

            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;

            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0RecDMAId);
                   guiIISCh0RecDMAId = 0xFF;
                }
            #endif

            count=0;
            do
            {
                do
                {
                    sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                    count ++;
                }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=MAX_RFIU_UNIT) );
            }while( (sysRFRxInMainCHsel==sysRFRxInPIPCHsel) && (sysRFRXPIP_en==1) && (count<=MAX_RFIU_UNIT) );
            
        #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
            {
               #if(USE_NEW_MEMORY_MAP)	
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
			   Idu_ClearBuf_FHD_IN_ULTRA_FHD(0);
			   #else
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
			   Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
			   #endif
            }
            else if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==0) && (iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+sysRFRxInMainCHsel]==UI_MENU_SETTING_RESOLUTION_1920x1072) )
            {
               #if(USE_NEW_MEMORY_MAP)		
			   //memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
			   iduPlaybackFrame(PKBuf0);			   
			   Idu_ClearBuf_FHD_IN_ULTRA_FHD(0);
			   #else
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
			   Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
			   #endif
            }
            else
            {
               #if(USE_NEW_MEMORY_MAP)
               //memDispBufArrage(DISPBUF_NORMAL);
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
               iduPlaybackFrame(PKBuf0);
			   Idu_ClearBuf_FHD_IN_ULTRA_FHD(0);
			   #else
               memDispBufArrage(DISPBUF_NORMAL);
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
               iduPlaybackFrame(PKBuf0);
			   Idu_ClearBuf(1);
			   #endif
            }
        #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
            if(sysTVOutOnFlag)
            {

            #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                sysTVswitchResolutionbyImagesize();
            #else
                if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                else
                  iduPlaybackMode(1920,1080,1920);
            #endif
                Idu_ClearBuf(1);
            }
            else
            {
                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                Idu_ClearBuf(1);
            }
        #else
            iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            Idu_ClearBuf(1);
        #endif
        
            rfiuRX_OpMode = rfiuRX_OpMode | RfIU_RX_OPMODE_FORCE_I;
            rfiu_SetRXOpMode_1(sysRFRxInMainCHsel);
            rfiuRX_OpMode = rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);
        
            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
               //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
               uiRfCreateDecTask(sysRFRxInMainCHsel);
            }

            DEBUG_UI("RF Channel increase: %d\r\n",sysRFRxInMainCHsel);
            break;

        case UI_MENU_CHANNEL_SUB:
            iduTVColorbar_onoff(1);
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
               rfiu_AudioRetONOFF_IIS(0);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;

            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;

            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0RecDMAId);
                   guiIISCh0RecDMAId = 0xFF;
                }
            #endif

            count=0;
            do
            {
                do
                {
                    if (sysRFRxInMainCHsel == 0)
                        sysRFRxInMainCHsel = MAX_RFIU_UNIT;

                    sysRFRxInMainCHsel = (sysRFRxInMainCHsel-1) % MAX_RFIU_UNIT;
                    count ++;
                }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=MAX_RFIU_UNIT) );
            }while( (sysRFRxInMainCHsel==sysRFRxInPIPCHsel) && (sysRFRXPIP_en==1) && (count<=MAX_RFIU_UNIT) );

        #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
            {
               #if(USE_NEW_MEMORY_MAP)
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
			   Idu_ClearBuf_FHD_IN_ULTRA_FHD(0);
			   #else
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
			   Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
			   #endif
            }
            else if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==0) && (iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+sysRFRxInMainCHsel]==UI_MENU_SETTING_RESOLUTION_1920x1072) )
            {
               #if(USE_NEW_MEMORY_MAP)
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
			   iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf_FHD_IN_ULTRA_FHD(0);
			   #else
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
			   Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
			   #endif
            }
            else
            {
               #if(USE_NEW_MEMORY_MAP)	
               memDispBufArrage(DISPBUF_NORMAL);
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
               iduPlaybackFrame(PKBuf0);
			   Idu_ClearBuf_FHD_IN_ULTRA_FHD(0);
			   #else
               memDispBufArrage(DISPBUF_NORMAL);
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
               iduPlaybackFrame(PKBuf0);
			   Idu_ClearBuf(1);
			   #endif
            }
        #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
            if(sysTVOutOnFlag)
            {

            #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                sysTVswitchResolutionbyImagesize();
            #else
                if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                else
                  iduPlaybackMode(1920,1080,1920);
            #endif
                Idu_ClearBuf(1);
            }
            else
            {
                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                Idu_ClearBuf(1);
            }
        #else
            iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            Idu_ClearBuf(1);
        #endif

            rfiuRX_OpMode = rfiuRX_OpMode | RfIU_RX_OPMODE_FORCE_I;
            rfiu_SetRXOpMode_1(sysRFRxInMainCHsel);
            rfiuRX_OpMode = rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);

            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                uiRfCreateDecTask(sysRFRxInMainCHsel);
            }
            DEBUG_UI("RF Channel decrease: %d\r\n",sysRFRxInMainCHsel);
            break;

        case UI_MENU_CHANNEL_1:
            iduTVColorbar_onoff(1);
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                sysRFRxInMainCHsel = 0;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                return 1;
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            if( (0==sysRFRxInPIPCHsel) && sysRFRXPIP_en )
            {
                DEBUG_UI("CH-1 is occupied by PIP!\n");
                return 0;
            }
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
               rfiu_AudioRetONOFF_IIS(0);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;

            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0RecDMAId);
                   guiIISCh0RecDMAId = 0xFF;
                }
            #endif

            sysRFRxInMainCHsel = 0;
        #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
            {
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
            }
            else if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==0) && (iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+sysRFRxInMainCHsel]==UI_MENU_SETTING_RESOLUTION_1920x1072) )
            {
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
            }
            else
            {
               memDispBufArrage(DISPBUF_NORMAL);
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf(1);
            }
        #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
            if(sysTVOutOnFlag)
            {
            #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                sysTVswitchResolutionbyImagesize();
            #else
                if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                else
                  iduPlaybackMode(1920,1080,1920);
            #endif
                Idu_ClearBuf(1);
            }
            else
            {
                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                Idu_ClearBuf(1);
            }
        #else
            iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            Idu_ClearBuf(1);
        #endif

            rfiuRX_OpMode = rfiuRX_OpMode | RfIU_RX_OPMODE_FORCE_I;
            rfiu_SetRXOpMode_1(sysRFRxInMainCHsel);
            rfiuRX_OpMode = rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);

            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                uiRfCreateDecTask(sysRFRxInMainCHsel);
                DEBUG_UI("RF Channel 1 OK\r\n");
            }
            break;

        case UI_MENU_CHANNEL_2:
            iduTVColorbar_onoff(1);
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                sysRFRxInMainCHsel = 1;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                return 1;
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            if( (1==sysRFRxInPIPCHsel) && sysRFRXPIP_en )
            {
                DEBUG_UI("CH-2 is occupied by PIP!\n");
                return 0;
            }
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
               rfiu_AudioRetONOFF_IIS(0);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;


            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0RecDMAId);
                   guiIISCh0RecDMAId = 0xFF;
                }
            #endif

            sysRFRxInMainCHsel = 1;
        #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
            {
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
            }
            else if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==0) && (iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+sysRFRxInMainCHsel]==UI_MENU_SETTING_RESOLUTION_1920x1072) )
            {
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
            }
            else
            {
               memDispBufArrage(DISPBUF_NORMAL);
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf(1);
            }
        #else
            iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            Idu_ClearBuf(1);
        #endif
        
            rfiuRX_OpMode = rfiuRX_OpMode | RfIU_RX_OPMODE_FORCE_I;
            rfiu_SetRXOpMode_1(sysRFRxInMainCHsel);
            rfiuRX_OpMode = rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);
            
            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                uiRfCreateDecTask(sysRFRxInMainCHsel);
                DEBUG_UI("RF Channel 2 OK\r\n");
            }
            break;

        case UI_MENU_CHANNEL_3:
            iduTVColorbar_onoff(1);
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                sysRFRxInMainCHsel = 2;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                return 1;
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            if( (2==sysRFRxInPIPCHsel) && sysRFRXPIP_en )
            {
                DEBUG_UI("CH-3 is occupied by PIP!\n");
                return 0;
            }
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
               rfiu_AudioRetONOFF_IIS(0);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;


            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0RecDMAId);
                   guiIISCh0RecDMAId = 0xFF;
                }
            #endif

            sysRFRxInMainCHsel = 2;
        #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
            {
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
            }
            else if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==0) && (iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+sysRFRxInMainCHsel]==UI_MENU_SETTING_RESOLUTION_1920x1072) )
            {
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
            }
            else
            {
               memDispBufArrage(DISPBUF_NORMAL);
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf(1);
            }
        #else
            iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            Idu_ClearBuf(1);
        #endif

            rfiuRX_OpMode = rfiuRX_OpMode | RfIU_RX_OPMODE_FORCE_I;
            rfiu_SetRXOpMode_1(sysRFRxInMainCHsel);
            rfiuRX_OpMode = rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);
            
            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                uiRfCreateDecTask(sysRFRxInMainCHsel);
                DEBUG_UI("RF Channel 3 OK\r\n");
            }
            break;

        case UI_MENU_CHANNEL_4:
            iduTVColorbar_onoff(1);
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                sysRFRxInMainCHsel = 3;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                return 1;
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            if( (3==sysRFRxInPIPCHsel) && sysRFRXPIP_en )
            {
                DEBUG_UI("CH-4 is occupied by PIP!\n");
                return 0;
            }
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
               rfiu_AudioRetONOFF_IIS(0);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;


            
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0RecDMAId);
                   guiIISCh0RecDMAId = 0xFF;
                }
            #endif

            sysRFRxInMainCHsel = 3;
        #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
            {
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
            }
            else if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==0) && (iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+sysRFRxInMainCHsel]==UI_MENU_SETTING_RESOLUTION_1920x1072) )
            {
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf_9300FHD(DISPLAY_BUF_NUM);
            }
            else
            {
               memDispBufArrage(DISPBUF_NORMAL);
               iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
               iduPlaybackFrame(PKBuf0);
               Idu_ClearBuf(1);
            }
        #else
            iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            Idu_ClearBuf(1);
        #endif

            rfiuRX_OpMode = rfiuRX_OpMode | RfIU_RX_OPMODE_FORCE_I;
            rfiu_SetRXOpMode_1(sysRFRxInMainCHsel);
            rfiuRX_OpMode = rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);
            
            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                uiRfCreateDecTask(sysRFRxInMainCHsel);
                DEBUG_UI("RF Channel 4 OK\r\n");
            }
            break;

        case UI_MENU_CHANNEL_SCAN:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            if (gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_CLR, &err);
                return 1;
            }
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
               rfiu_AudioRetONOFF_IIS(0);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;

            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);

            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
              #if IIS1_REPLACE_IIS5
                if(guiIISCh0PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0PlayDMAId);
                   guiIISCh0PlayDMAId = 0xFF;
                }
              #else
                if(guiIISCh4PlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh4PlayDMAId);
                   guiIISCh4PlayDMAId = 0xFF;
                }
              #endif
            #endif

            #if RFIU_RX_AUDIO_RETURN
                if(guiIISCh0RecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISCh0RecDMAId);
                   guiIISCh0RecDMAId = 0xFF;
                }
            #endif
            count=0;
            while((gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] != RFI_WRAPDEC_TASK_RUNNING) && (count<4))
            {
#if ((HW_BOARD_OPTION == MR9200_RX_JIT_M916HN4) || (UI_VERSION == UI_VERSION_MAYON))  
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
#endif
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


u8 uiSetRf_PIP_ChangeChannel(u8 setting)
{
    int count;
    u8 err, bRet=0;
    
    if (UISetRFMode != UI_MENU_RF_FULL)
    {
         DEBUG_UI("Warning! PIP not run on Single mode.\n");
         return 0;
    }
    
    OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_UI("uiSetRf_PIP_ChangeChannel %d\r\n",setting);

    switch (setting)
    {
        case UI_MENU_CHANNEL_ADD:
            gRfiuUnitCntl[sysRFRxInPIPCHsel].RX_MpegDec_Stop=1;

            uiRfSuspendDelDecTask(sysRFRxInPIPCHsel);
            uiRFStatue[sysRFRxInPIPCHsel] = UI_RF_STATUS_OTHER;

            count=0;
            do
            {
                do
                {
                    sysRFRxInPIPCHsel = (sysRFRxInPIPCHsel+1) % MAX_RFIU_UNIT;
                    count ++;
                }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInPIPCHsel) & 0x01) == 0) && (count<=MAX_RFIU_UNIT) );
            }while( (sysRFRxInMainCHsel==sysRFRxInPIPCHsel) && (sysRFRXPIP_en==1) && (count<=MAX_RFIU_UNIT) );

            if(gRfiu_WrapDec_Sta[sysRFRxInPIPCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
               uiRfCreateDecTask(sysRFRxInPIPCHsel);
            }

            DEBUG_UI("PIP Channel increase: %d\r\n",sysRFRxInPIPCHsel);
            break;

        case UI_MENU_CHANNEL_SUB:
            gRfiuUnitCntl[sysRFRxInPIPCHsel].RX_MpegDec_Stop=1;

            uiRfSuspendDelDecTask(sysRFRxInPIPCHsel);
            uiRFStatue[sysRFRxInPIPCHsel] = UI_RF_STATUS_OTHER;

            count=0;
            do
            {
                do
                {
                    if (sysRFRxInPIPCHsel == 0)
                        sysRFRxInPIPCHsel = MAX_RFIU_UNIT;

                    sysRFRxInPIPCHsel = (sysRFRxInPIPCHsel-1) % MAX_RFIU_UNIT;
                    count ++;
                }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInPIPCHsel) & 0x01) == 0) && (count<=MAX_RFIU_UNIT) );
            }while( (sysRFRxInMainCHsel==sysRFRxInPIPCHsel) && (sysRFRXPIP_en==1) && (count<=MAX_RFIU_UNIT) );

            
            if(gRfiu_WrapDec_Sta[sysRFRxInPIPCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                uiRfCreateDecTask(sysRFRxInPIPCHsel);
            }
            DEBUG_UI("PIP Channel decrease: %d\r\n",sysRFRxInPIPCHsel);
            break;

        case UI_MENU_CHANNEL_1:
            if( (0==sysRFRxInMainCHsel) && sysRFRXPIP_en )
            {
                DEBUG_UI("CH-1 is occupied by Main CH!\n");
                return 0;
            }
            gRfiuUnitCntl[sysRFRxInPIPCHsel].RX_MpegDec_Stop=1;

            uiRfSuspendDelDecTask(sysRFRxInPIPCHsel);
            uiRFStatue[sysRFRxInPIPCHsel] = UI_RF_STATUS_OTHER;

            sysRFRxInPIPCHsel = 0;
            if(gRfiu_WrapDec_Sta[sysRFRxInPIPCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                uiRfCreateDecTask(sysRFRxInPIPCHsel);
                DEBUG_UI("PIP Channel 1 OK\r\n");
            }
            break;

        case UI_MENU_CHANNEL_2:
            if( (1==sysRFRxInMainCHsel) && sysRFRXPIP_en )
            {
                DEBUG_UI("CH-2 is occupied by Main CH!\n");
                return 0;
            }
            gRfiuUnitCntl[sysRFRxInPIPCHsel].RX_MpegDec_Stop=1;

            uiRfSuspendDelDecTask(sysRFRxInPIPCHsel);
            uiRFStatue[sysRFRxInPIPCHsel] = UI_RF_STATUS_OTHER;

            sysRFRxInPIPCHsel = 1;
            if(gRfiu_WrapDec_Sta[sysRFRxInPIPCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                uiRfCreateDecTask(sysRFRxInPIPCHsel);
                DEBUG_UI("PIP Channel 2 OK\r\n");
            }            
            break;

        case UI_MENU_CHANNEL_3:
            if( (2==sysRFRxInMainCHsel) && sysRFRXPIP_en )
            {
                DEBUG_UI("CH-3 is occupied by Main CH!\n");
                return 0;
            }
            gRfiuUnitCntl[sysRFRxInPIPCHsel].RX_MpegDec_Stop=1;

            uiRfSuspendDelDecTask(sysRFRxInPIPCHsel);
            uiRFStatue[sysRFRxInPIPCHsel] = UI_RF_STATUS_OTHER;

            sysRFRxInPIPCHsel = 2;
            if(gRfiu_WrapDec_Sta[sysRFRxInPIPCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                uiRfCreateDecTask(sysRFRxInPIPCHsel);
                DEBUG_UI("PIP Channel 3 OK\r\n");
            }            

            break;

        case UI_MENU_CHANNEL_4:
             if( (3==sysRFRxInMainCHsel) && sysRFRXPIP_en )
            {
                DEBUG_UI("CH-4 is occupied by Main CH!\n");
                return 0;
            }
            gRfiuUnitCntl[sysRFRxInPIPCHsel].RX_MpegDec_Stop=1;

            uiRfSuspendDelDecTask(sysRFRxInPIPCHsel);
            uiRFStatue[sysRFRxInPIPCHsel] = UI_RF_STATUS_OTHER;

            sysRFRxInPIPCHsel = 3;
            if(gRfiu_WrapDec_Sta[sysRFRxInPIPCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                uiRfCreateDecTask(sysRFRxInPIPCHsel);
                DEBUG_UI("PIP Channel 4 OK\r\n");
            }            


            break;

    }
    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_CLR, &err);
    return bRet;
}

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

    rfiuRxMainAudioPlayStart=0;
    rfiuRxMainVideoPlayStart=0;
    if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
       rfiu_AudioRetONOFF_IIS(0);
    uiRfSuspendDelDecTask_ALL(0);
    #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
      #if IIS1_REPLACE_IIS5
        if(guiIISCh0PlayDMAId != 0xFF)
        {
           marsDMAClose(guiIISCh0PlayDMAId);
           guiIISCh0PlayDMAId = 0xFF;
        }
      #else
        if(guiIISCh4PlayDMAId != 0xFF)
        {
           marsDMAClose(guiIISCh4PlayDMAId);
           guiIISCh4PlayDMAId = 0xFF;
        }
      #endif
    #endif

    #if RFIU_RX_AUDIO_RETURN
        if(guiIISCh0RecDMAId != 0xFF)
        {
            marsDMAClose(guiIISCh0RecDMAId);
            guiIISCh0RecDMAId = 0xFF;
        }
    #endif

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
            }while( ( ((rfiuRX_CamOnOff_Sta >> Chsel) & 0x01) == 0) && (count<=MAX_RFIU_UNIT) );
            sysRFRxInMainCHsel=Chsel;
            break;

        case UI_MENU_CHANNEL_SUB:
            count=0;
            Chsel=sysRFRxInMainCHsel;
            do
            {
                Chsel = (Chsel-1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> Chsel) & 0x01) == 0) && (count<=MAX_RFIU_UNIT) );
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
            }while( ( ((rfiuRX_CamOnOff_Sta >> Chsel) & 0x01) == 0) && (count<=MAX_RFIU_UNIT) );
            sysRF_PTZ_CHsel=Chsel;
            break;

        case UI_MENU_CHANNEL_SUB:
            count=0;
            Chsel=sysRF_PTZ_CHsel;
            do
            {
                Chsel = (Chsel-1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> Chsel) & 0x01) == 0) && (count<=MAX_RFIU_UNIT) );
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
//    uiSetRfResolutionRxToTx(UI_P2P_RES[camId], camId);
    return 1;
}

#endif

u32 uiGetMenuMode(void)
{
    return MyHandler.MenuMode;
}


#if 1
u32 uiSetZoomMode(u8 Channel, u32 ZoomX, u32 ZoomY, u8 act)
{
#if RFIU_SUPPORT

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[Channel] != RFIU_RX_STA_LINK_OK)
        return 0;   

    if( sysCameraMode != SYS_CAMERA_MODE_RF_RX_FULLSCR)
        return 0;


    if (((rfiuRX_CamOnOff_Sta >> Channel) & 0x01) == 0)
        return 0;

    if (act == 1)
    {
        if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight > 860)
        {//FHD
        #if(SW_APPLICATION_OPTION  == MR9300_RFDVR_RX1RX2)
             #if 1
                if(ZoomX+896>1792)
                    ZoomX=1792-896;
                if(ZoomY+720>1440)
                    ZoomY=1440-720;
             #else
                if(ZoomX+1024>1920)
                    ZoomX=1920-1024;
                if(ZoomY+720>1440)
                    ZoomY=1440-720;
             #endif   
                uiEnZoom_Y_Offset=1920*ZoomY + 0 +  ZoomX & 0xffffff80;
                uiEnZoom_UV_Offset=1920*ZoomY/2 + 0 + ZoomX & 0xffffff80;
        #else
                if(ZoomX+512>960)
                    ZoomX=960-512;
                if(ZoomY+288>536)
                    ZoomY=536-288;
                
                uiEnZoom_Y_Offset=RF_RX_2DISP_WIDTH*2*ZoomY +  ZoomX & 0xffffffc0;
                uiEnZoom_UV_Offset=RF_RX_2DISP_WIDTH*2*ZoomY/2 + ZoomX & 0xffffffc0;
        #endif
        }
        else if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight >= 720)
        {//HD
            if(ZoomX+640>1280)
                ZoomX=1280-640;
            if(ZoomY+360>720)
                ZoomY=720-360;
        #if(SW_APPLICATION_OPTION  == MR9300_RFDVR_RX1RX2)
            uiEnZoom_Y_Offset=RF_RX_2DISP_WIDTH*2*ZoomY + ZoomX & 0xffffff80;
            uiEnZoom_UV_Offset=RF_RX_2DISP_WIDTH*2*ZoomY/2 + ZoomX & 0xffffff80;
        #else
            uiEnZoom_Y_Offset=RF_RX_2DISP_WIDTH*2*ZoomY + ZoomX & 0xffffff80;
            uiEnZoom_UV_Offset=RF_RX_2DISP_WIDTH*2*ZoomY/2 + ZoomX & 0xffffff80;
        #endif
        }
        else
        {//QHD
            if(ZoomX+320>640)
                ZoomX=640-320;
            if(ZoomY+176>352)
                ZoomY=352-176;
            uiEnZoom_Y_Offset=RF_RX_2DISP_WIDTH*2*ZoomY +  ZoomX & 0xffffffc0;
            uiEnZoom_UV_Offset=RF_RX_2DISP_WIDTH*2*ZoomY/2 + ZoomX & 0xffffffc0;
        }
        uiEnZoom=1;
        //DEBUG_UI("UI Set Zoom to size cam%d %d, %d !!!\r\n",Channel, ZoomX, ZoomY);
    }
    else
    {
        uiEnZoom_Y_Offset=0;
        uiEnZoom_UV_Offset=0;
        uiEnZoom=0;
        //DEBUG_UI("UI Set CAM%d Zoom Off !!!\r\n",Channel);
    }

    
    return 0;
#else
    DEBUG_UI("UI Set Zoom not ready !!!\r\n");
    return 0;
#endif

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
        DEBUG_UI("UI Set Zoom to size cam%d %d, %d !!!\r\n",Channel, ZoomX, ZoomY);
    else
        DEBUG_UI("UI Set CAM%d Zoom Off !!!\r\n",Channel);

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
#if((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3) || (UI_VERSION == UI_VERSION_MAYON))
    if (level == 0)
        osdDrawRemoteOn(UI_OSD_CLEAR);
    else
        osdDrawRemoteOn(UI_OSD_DRAW);
#endif
#endif
}


u8 uiSetP2PPassword(u8 *password)
{
    DEBUG_UI("uiSetP2PPassword %s\r\n",password);
    memcpy(UI_P2P_PSW, password, sizeof(UI_P2P_PSW));
    LoadP2PPassword((char *)password);
    if (Main_Init_Ready == 1)
        Save_UI_Setting();
    return 1;
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
            uiSetDefaultSetting();
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

s32 Save_UI_Setting(void)
{
    u8 check_val = 0;
    u8 update = 0, err;
    //--------------------------//
    if(Main_Init_Ready == 0)
        return 0; 
   #if(FPGA_BOARD_A1018_SERIES)
      return 0;
   #endif
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    update = uiCompareSaveData();
    if (update == 1)
    {
        check_val = uiGetSaveChecksum();
        DEBUG_UI("SaveUI iconflag %d check_val %d \n",iconflag[UI_MENU_SETIDX_CHECK],check_val);
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
                return 0;
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
    return 1;
}

s32 Save_UI_Setting_Task(s32 dummy)
{
	return Save_UI_Setting();
}

void uiReadRFIDFromFlash(u8 *FlashAddr)
{

#if ( PWIFI_SUPPORT && PWIFI_PAIR_SUPPORT)
    DEBUG_GREEN("Read Flash\n");
    memcpy(&TX_Pair_Mode,FlashAddr,sizeof(TX_Pair_Mode));
    FlashAddr += sizeof(TX_Pair_Mode);
    memcpy(&pwifiMacAddrDst,FlashAddr,sizeof(pwifiMacAddrDst));
#else

    int i;
    memcpy(&uiRFID,FlashAddr,sizeof(uiRFID));
  #if (RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)  
    DEBUG_MAIN("RFID: %x, %x, %x, %x, %x, %x, %x, %x \n",uiRFID[0],uiRFID[1],uiRFID[2],uiRFID[3],uiRFID[4],uiRFID[5],uiRFID[6],uiRFID[7]);
  #else
    DEBUG_MAIN("RFID: %x, %x, %x, %x \n",uiRFID[0],uiRFID[1],uiRFID[2],uiRFID[3]);
  #endif
 
    memcpy(&uiRFCODE,FlashAddr+UI_SETUP_RFID_SIZE,sizeof(uiRFCODE));
  #if (RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)  
    DEBUG_MAIN("RFCODE: %x, %x, %x, %x, %x, %x, %x, %x \n",uiRFCODE[0],uiRFCODE[1],uiRFCODE[2],uiRFCODE[3],uiRFCODE[4],uiRFCODE[5],uiRFCODE[6],uiRFCODE[7]);
  #else
    DEBUG_MAIN("RFCODE: %x, %x, %x, %x \n",uiRFCODE[0],uiRFCODE[1],uiRFCODE[2],uiRFCODE[3]);
  #endif
  
    memcpy(&rfiuRSSI_CALDiff,FlashAddr+UI_SETUP_RFID_SIZE+UI_SETUP_RFID_SIZE,sizeof(rfiuRSSI_CALDiff));
  #if (RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)  
    DEBUG_UI("RF RSSI DIFF: %d, %d, %d, %d, %d, %d, %d, %d \n",rfiuRSSI_CALDiff[0],rfiuRSSI_CALDiff[1],rfiuRSSI_CALDiff[2],rfiuRSSI_CALDiff[3],rfiuRSSI_CALDiff[4],rfiuRSSI_CALDiff[5],rfiuRSSI_CALDiff[6],rfiuRSSI_CALDiff[7]);
  #else
    DEBUG_UI("RF RSSI DIFF: %d, %d, %d, %d \n",rfiuRSSI_CALDiff[0],rfiuRSSI_CALDiff[1],rfiuRSSI_CALDiff[2],rfiuRSSI_CALDiff[3]);
  #endif  
    for(i=0;i<RFID_MAX_WORD;i ++)
    {
        if(rfiuRSSI_CALDiff[i] >  40)
           rfiuRSSI_CALDiff[i]=0; 

        if(rfiuRSSI_CALDiff[i] < -40)
           rfiuRSSI_CALDiff[i]=0; 
    }

#endif
}

#if NIC_SUPPORT
void uiReadNetworkIDFromFlash(u8 *FlashAddr)
{
    memcpy(&uiP2PID,FlashAddr,sizeof(uiP2PID));
    DEBUG_MAIN("TUTKID: %s \n",uiP2PID);

    memcpy(&uiMACAddr,FlashAddr+UI_SETUP_GUID_SIZE,sizeof(uiMACAddr));
    DEBUG_MAIN("MAC: %x-%x-%x-%x-%x-%x \n",uiMACAddr[0],uiMACAddr[1],uiMACAddr[2],uiMACAddr[3],uiMACAddr[4],uiMACAddr[5]);
}
#endif

void uiReadVersionFromFlash(u8 * FlashAddr)
{
    memcpy(&uiVersion , FlashAddr, sizeof(uiVersion));
    memcpy(&uiVersionTime , FlashAddr+sizeof(uiVersion), sizeof(uiVersionTime));
    DEBUG_UI("*******Version: %s***%s***\n",uiVersion,uiVersionTime);
}

void uiWriteRFIDFromFlash(u8 *FlashAddr)
{

#if (PWIFI_SUPPORT && PWIFI_PAIR_SUPPORT)
    memcpy((void *)(FlashAddr),  (void *)&TX_Pair_Mode, sizeof(TX_Pair_Mode));
    FlashAddr += sizeof(TX_Pair_Mode);
    memcpy((void *)(FlashAddr),  (void *)&pwifiMacAddrDst, sizeof(pwifiMacAddrDst));
#else
    memcpy((void *)(FlashAddr),  (void *)&uiRFID, sizeof(uiRFID));
    memcpy((void *)(FlashAddr +UI_SETUP_RFID_SIZE),  (void *)&uiRFCODE, sizeof(uiRFCODE));
    memcpy((void *)(FlashAddr +UI_SETUP_RFID_SIZE +UI_SETUP_RFID_SIZE),  (void *)&rfiuRSSI_CALDiff, sizeof(rfiuRSSI_CALDiff));
#endif
}

#if NIC_SUPPORT
void uiWriteNetworkIDFromFlash(u8 *FlashAddr)
{
    memcpy((void *)FlashAddr,  (void *)&uiP2PID, sizeof(uiP2PID));
    memcpy((void *)(FlashAddr+UI_SETUP_GUID_SIZE), (void *)&uiMACAddr,sizeof(uiMACAddr));
}
#endif

u8 uiTemperatureConversion(u8 degreeVal, u8 ConverseType)
{
    if (ConverseType == UI_DEGREE_TYPE_FAHRENHEIT) /* (*F) Converse to (*C) */
        return (u8)((((degreeVal - 32) * 5) / 9.0) + 0.5);
    else /* (*C) Converse to (*F) */
        return (u8)((((degreeVal * 9) / 5) + 32.0) + 0.5);
}
