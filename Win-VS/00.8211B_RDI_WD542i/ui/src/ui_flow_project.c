/*

  Copyright (c) 2008 Mars Semiconductor Corp.

   Module Name:

   ui_flow_project.c

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
#include "sysapi.h"
#include "usbapi.h"
#include "ui_project.h"
#include "siuapi.h"
#include "isuapi.h"
#include "asfapi.h"
#include "uartapi.h"
#include "dcfapi.h"
#include "board.h"
#include "aviapi.h"
#include "timerapi.h"
#include "ipuapi.h"
#include "spiapi.h"
#include "osd_draw_project.h"
#include "gpioapi.h"
#include "iisapi.h"
#if(CHIP_OPTION >= CHIP_A1013A)
#include "rfiuapi.h"
#endif

#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
#if (HW_BOARD_OPTION==MR6730_FINEDIGITAL_LCD)
#include "i2capi.h"
#endif


#if(UART_GPS_COMMAND == 1)
#include "gpsapi.h"
#endif
#if (HW_BOARD_OPTION == MR6730_WINEYE)
#include "MotionDetect_API.h"
#endif

#if (IS_COMMAX_DOORPHONE) || (IS_HECHI_DOORPHONE) || (HW_BOARD_OPTION == MR6730_AFN)
    #include "MainFlow.h"
    #include "Pic.h"
#endif
#if ICOMMWIFI_SUPPORT  
#include "p2pserver_api.h"
#endif
/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */
#define UI_MENU_SIZE_X           640
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
#define UI_TX_SYNC_TIME         3
#endif
#define UI_TEST_MODE_TIMEOUT    121

/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
static u8 GS[23]={0};
UI_HANDLER MyHandler;
u8 PlayListDspType = UI_DSP_PLAY_LIST_FILE;
u8 osdYShift = 0;      /*for NTSC osdYShift = 0, PAL osdYShift = 44*/
u8 TvOutMode = 0;       /*NTSC*/
u8 MotionMaskArea[MASKAREA_MAX_ROW][MASKAREA_MAX_COLUMN]={0};
u8 StartMotMask[MASKAREA_MAX_ROW][MASKAREA_MAX_COLUMN] = {0};
u8 showTime = 0;
u8 uiStartP2PID[P2PID_LENGTH]={0};
u8 uiP2PID[P2PID_LENGTH];
u16 OSDIconEndX,OSDIconEndY, OSDIconMidX,OSDIconMidY;
u16 PbkOSD1, PbkOSD2, PbkOSD1High, PbkOSD2High;
u32 TV_X,TV_Y;
u32 autoofftick;
u32 Display_X, Display_Y;
u32 uiStartRFID[RFID_MAX_WORD]={0};
u32 uiStratRFCode[RFID_MAX_WORD]={0};
u32 uiRFID[RFID_MAX_WORD];
u32 uiRFCODE[RFID_MAX_WORD];
u8  uiStartMACAddr[MAC_LENGTH]={0};
u8  uiMACAddr[MAC_LENGTH];
#if (HW_BOARD_OPTION == MR6730_AFN)
s8  OverwriteStringEnable   = 0;
#else
s8  OverwriteStringEnable   = 1;
#endif
s8 gsParseDirName[9]="MFG";
#if ( (HW_BOARD_OPTION == MR8600_RX_RDI )  || (HW_BOARD_OPTION == MR8120_RX_RDI) )
u8  uiLEDSelect=0;
#endif
RTC_DATE_TIME ScheduledTimeFrom = { 9, 1, 1, 0, 0, 0 }, ScheduledTimeTo = { 9, 2, 1, 0, 0, 0 };
RTC_DATE_TIME StartSchTimeFrom = { 9, 1, 1, 0, 0, 0 }, StartSchTimeTo = { 9, 2, 1, 0, 0, 0 };
RTC_DATE_TIME SetTime;
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
u8 UI_Default_P2P_PSW[UI_P2P_PSW_MAX_LEN] = {"iLikeZinwell"};
#else
u8 UI_Default_P2P_PSW[UI_P2P_PSW_MAX_LEN] = {"000000"};
#endif
#if(HW_BOARD_OPTION == MR6730_AFN)     
u32 playbackDir;
#endif
#if (NIC_SUPPORT)
UI_NET_INFO UINetInfo;
UI_NET_INFO StartUINetInfo;
#endif
#if (HW_BOARD_OPTION==MR6730_FINEDIGITAL_LCD)
u8 Lock_video=TRUE;
#endif
u8  uiRFStatue[MAX_RFIU_UNIT]={UI_OSD_NONE};
#if(HW_BOARD_OPTION==MR6730_WINEYE)
#if (G_SENSOR == G_SENSOR_LIS302DL)
u8  GLimitX                 = 0x20; // Log file write F if X absolute G sensor value greater than GLimitX, 1G = 0x10.
u8  GLimitY                 = 0x20; // Log file write F if Y absolute G sensor value greater than GLimitY, 1G = 0x10.
u8  GLimitZ                 = 0x20; // Log file write F if Z absolute G sensor value greater than GLimitZ, 1G = 0x10.
#elif (G_SENSOR == G_SENSOR_H30CD)
u16 GLimitX                 = 1500; // Log file write F if X absolute G sensor value greater than GLimitX, 1G = 1000.
u16 GLimitY                 = 1500; // Log file write F if Y absolute G sensor value greater than GLimitY, 1G = 1000.
u16 GLimitZ                 = 1500; // Log file write F if Z absolute G sensor value greater than GLimitZ, 1G = 1000.
#elif (G_SENSOR == G_SENSOR_BMA150)
u16 GLimitX                 = 0x100; // Log file write F if X absolute G sensor value greater than GLimitX, 1G = 0x80.
u16 GLimitY                 = 0x100; // Log file write F if Y absolute G sensor value greater than GLimitY, 1G = 0x80.
u16 GLimitZ                 = 0x100; // Log file write F if Z absolute G sensor value greater than GLimitZ, 1G = 0x80.
#endif
#if(G_SENSOR_DETECT)
u8  GSensorEvent;
#endif
u8  BuzzerToggle    = 0;   // 1: On, 0: Off
u8  EMFile          = 0;    // 1: Recorded file by receiving "Enter Recording" signal(PORT1=0, PORT2=1, PORT3=0) - (Event norMal), 0: otherwise
u8  FileType        = FILE_TYPE_DR;
s8  uiUpFWSDInit[]  = "\\pa9spi.bin";
s8  uiUpFWKey[]     = "\\pa9001.bin";
#endif
#if ISU_OVERLAY_ENABLE
u8  szVideoOverlay1[MAX_OVERLAYSTR];
u8  szVideoOverlay2[MAX_OVERLAYSTR];
u8  szLogString[MAX_OVERLAYSTR];
#endif
#if (HW_BOARD_OPTION == MR8120_TX_HECHI)
u8 powerdown_count = 0;
#endif
#if (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
u32 uiSetLightDuration = 0;
u32 uiCurrentLightTime = 0;
u8  uiInScheduleLight = 0;
u16  uiInManualLight = 0;
u8  uiLightTimer[4];    /*10:11~12:14*/
u8  uiLightTimer1[4];    /*10:11~12:14*/
u8  uiLightTimer2[4];    /*10:11~12:14*/
u8  uiLightWeek1[7];     /*1: on, 0:off*/
u8  uiLightWeek2[7];     /*1: on, 0:off*/
u8  uiStartLightTimer[4];    /*10:11~12:14*/
u8  uiStartLightWeek[7];     /*1: on, 0:off*/
u32 uiSetTriggerDimmer = 0;
#endif

#if (HW_BOARD_OPTION == MR6730_AFN)	
#if (CIU_BOB_MODE)
u8 g_ShowBobOnOSD=0;//replace the "SHOW_BOB_ON_OSD"		
u8 g_CIU_BobMode=0;
#endif

u8 gPreviewInitDone=0;

#if (CIU_OSD_METHOD_2)
u8 gCOSD2bPosAutoChange=1;// 1:enabled

#ifndef COSD2B_X1_PRE
#error "COSD2B_X1_PRE undefined"
#else
int COSD2b_X1 = COSD2B_X1_PRE;
int COSD2b_Y1 = COSD2B_Y1_PRE;
int COSD2b_X2 = COSD2B_X2_PRE;
int COSD2b_Y2 = COSD2B_Y2_PRE;
#endif
#endif//#if (CIU_OSD_METHOD_2)

#endif //#if (HW_BOARD_OPTION == MR6730_AFN)
#if((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
u8 uiCheckNightTime=0;
u8 uiNightVision=0;
u8 uiBootPressKey = 0;
u32 uiBootPressTic;
u32 uiTestTimeoutCnt = 0;
u8  uiIsVM9710=2;
#endif
#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
u8 uiTXstaus=0;
#endif
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern u8 uiMenuVideoSizeSetting;
extern u8 uiIsRFBroken;
extern u8 uiIsRFParing[RFID_MAX_WORD];   // 1:Pairing 0:Not Pairing
#if(RFIU_SUPPORT)
extern int gRfiuSyncWordTable[RFID_MAX_WORD];
extern int gRfiuCustomerCode[RFID_MAX_WORD];
#endif
#if(HW_BOARD_OPTION==MR6730_WINEYE)
extern  u8  HasEventFile;
extern  u8  Current_REC_Mode;
extern  s8  ispFWFileName[32];
#endif
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];
extern u8 PhotoFramenum;
u8 counttime=0;
u16 counttimetotal=0;
extern u8 uiIPAddr[4];
extern u8 uiSubnetMask[4];
extern u8 uiDefaultGateway[4];
extern u8 uiISStatic;
extern u8 uiP2PMode;
extern  u32 *CiuOverlayImg1_Top;
extern  u32 *CiuOverlayImg1_Bot;
extern  u32 *CiuOverlayImg2_Top;
extern  u32 *CiuOverlayImg2_Bot;
extern  u8  OSDTimestameLevel;
extern u8 UI_SDLastLevel;
extern u8 SD_detect_status; //sd status 1:OK 0 :not
#if (Melody_SNC7232_ENA)
extern u8 Melody_play_num;
extern u8 Melody_audio_level;
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
#if RFIU_SUPPORT
void uiSynRfConfig(u8 camId)
{
    u8  MEnable, level, nightLev,Syn = 0;
    u8  uartCmd[16];

    // compare TX & RX resolution, and sync it
    //DEBUG_UI("========== Video Size is %d ==========\r\n",iconflag[UI_MENU_SETIDX_CH1_RES+camId]);
    if (uiP2PMode == 0)
    {
        switch(iconflag[UI_MENU_SETIDX_CH1_RES+camId])
        {
            case UI_MENU_SETTING_RESOLUTION_HD:
                if((gRfiuUnitCntl[camId].TX_PicWidth != 1280) || (gRfiuUnitCntl[camId].TX_PicHeight != 720))
                    uiMenuSet_RXCameraResolution(iconflag[UI_MENU_SETIDX_CH1_RES+camId],camId);
                break;

            case UI_MENU_SETTING_RESOLUTION_VGA:
                if((gRfiuUnitCntl[camId].TX_PicWidth != 640) || (gRfiuUnitCntl[camId].TX_PicHeight != 480))
                    uiMenuSet_RXCameraResolution(iconflag[UI_MENU_SETIDX_CH1_RES+camId],camId);
                break;

            case UI_MENU_SETTING_RESOLUTION_QVGA:
                if((gRfiuUnitCntl[camId].TX_PicWidth != 320) || (gRfiuUnitCntl[camId].TX_PicHeight != 240))
                    uiMenuSet_RXCameraResolution(iconflag[UI_MENU_SETIDX_CH1_RES+camId],camId);
                break;
        }
    }
    else
    {
        level = uiGetP2PStatueToRF();
        sprintf((char*)uartCmd,"MODE %d %d", rfiuRX_OpMode, level);
        rfiu_RXCMD_Enc(uartCmd,camId);
    }
}
#endif

void uiSetZoomOut(void)
{
    #if GET_SIU_RAWDATA_PURE //Lucian: For debug!
    {
        u8 MSG[64];
        #if SELECT_ADJUST_EL_AGC
            siuAdjustSW -=10;
            if(siuAdjustSW<1)
                siuAdjustSW=1;
            sprintf((char*)MSG,"EL=%d",siuAdjustSW);
        #else
            siuAdjustAGC --;
            if(siuAdjustAGC<1)
                siuAdjustAGC=1;
            sprintf((char*)MSG,"AGC=%d",siuAdjustAGC);
        #endif
            uiClearOSDBuf(2);
            uiOSDASCIIStringByColor(MSG, (OSDIconMidX-srtlen(MSG)*OSD_STRING_W/2) , (OSDIconMidX-8) , OSD_Blk2 , 0xc0, 0x00);
    }
    #else
        if(sysCameraMode == SYS_CAMERA_MODE_PREVIEW)
        {
            sysPreviewZoomFactor --;

            if(siuOpMode == SIUMODE_MPEGAVI)
            {
                if(sysPreviewZoomFactor<0)
                   sysPreviewZoomFactor=0;
                else
                   sysbackSetEvt(SYS_BACK_VIDEOZOOMINOUT, sysPreviewZoomFactor);
            }
            else
            {
                if(sysPreviewZoomFactor<0)
                   sysPreviewZoomFactor=0;
                else
                   sysSetEvt(SYS_EVT_PREVIEWZOOMINOUT, sysPreviewZoomFactor);
            }
        }
    #endif
}

void uiSetZoomIn(void)
{
    #if GET_SIU_RAWDATA_PURE //Lucian: For debug!
    {
        u8 MSG[64];
        #if SELECT_ADJUST_EL_AGC
            siuAdjustSW +=10;
            sprintf((char*)MSG,"EL=%d",siuAdjustSW);
        #else
            siuAdjustAGC ++;
            sprintf((char*)MSG,"AGC=%d",siuAdjustAGC);
        #endif
        uiClearOSDBuf(2);
        uiOSDASCIIStringByColor(MSG, (OSDIconMidX-srtlen(MSG)*OSD_STRING_W/2) , (OSDIconMidX-8) , OSD_Blk2 , 0xc0, 0x00);
    }
    #else
        if(sysCameraMode == SYS_CAMERA_MODE_PREVIEW)
        {
            sysPreviewZoomFactor ++;
            if(siuOpMode == SIUMODE_MPEGAVI)
            {
                if(sysPreviewZoomFactor > (MAX_VIDEOCLIP_ZOOM_FACTOR-1) )
                    sysPreviewZoomFactor=MAX_VIDEOCLIP_ZOOM_FACTOR-1;
                else
                    sysbackSetEvt(SYS_BACK_VIDEOZOOMINOUT, sysPreviewZoomFactor);
            }
            else
            {
                if(sysPreviewZoomFactor > (MAX_PREVIEW_ZOOM_FACTOR-1) )
                    sysPreviewZoomFactor=MAX_PREVIEW_ZOOM_FACTOR-1;
                else
                    sysSetEvt(SYS_EVT_PREVIEWZOOMINOUT, sysPreviewZoomFactor);
            }
        }
    #endif
}

void uiFlowPlayback_Delete_File(void)
{
    IduVideo_ClearPKBuf(0);
    if(Write_protet() && gInsertCard==1)
    {
        DEBUG_UI("Write_protet.....\r\n");
        osdDrawProtect(1);
    }
    else
    {
    	#if(HW_BOARD_OPTION != MR6730_AFN) 	    
        osdDrawFillWait();
		#endif

		#if (HW_BOARD_OPTION == MR6730_AFN)
	
		#if (USE_UI_TASK_WDT)	
		UI_TaskWdt_Enabled_Set(UITSKWDT_DISABLE);
		#endif 
    
		if((PlayListDspType == UI_DSP_PLAY_LIST_DIR&&global_totaldir_count)||(PlayListDspType == UI_DSP_PLAY_LIST_FILE&&global_totalfile_count))
			uiOsdDrawPlaybackMenuDeleteMsg(UI_OSD_DRAW);
		#endif 
		
        if (PlayListDspType == UI_DSP_PLAY_LIST_DIR)
        {
            DEBUG_UI("Delete Folder\r\n");
		#if (HW_BOARD_OPTION != MR6730_AFN)
		if(dcfPlaybackDelDir() == 0)
			DEBUG_UI("Delete Folder Fail\r\n");
		
		#else

	    	if(dcfPlaybackDelDir() == 0)
            {
                DEBUG_UI("Delete Folder Fail\r\n");
            }		
			#if (UI_PBMENU_M2)
			//do nothing
	        #else		    
            else
            {
                playbackDir--;
                if (!playbackDir)
                    playbackDir = global_totaldir_count;
            }
			#endif 	
		#endif 
        }
        else if (PlayListDspType == UI_DSP_PLAY_LIST_FILE)
        {
            DEBUG_UI("Delete File\r\n");
            dcfPlaybackDel();
			#if (HW_BOARD_OPTION == MR6730_AFN)
			#if (UI_PBMENU_M2)
			//do nothing
			#else
            playback_location--;
            if (!playback_location)
                playback_location = global_totalfile_count;
			#endif
			#endif 		
        }

	#if (HW_BOARD_OPTION == MR6730_AFN)   
		uiOsdDrawPlaybackMenuDeleteMsg(UI_OSD_CLEAR);
		
		#if (USE_UI_TASK_WDT)	
		UI_TaskWdt_Enabled_Set(UITSKWDT_ENABLE);
		#endif 
	#endif 		
    }
    DEBUG_UI("back to playback menu mode\r\n");
    MyHandler.MenuMode = PLAYBACK_MENU_MODE;
    uiOsdDrawPlaybackMenu(UI_KEY_DELETE);

}

void uiFlowEnterMenuMode(u32 Mode)
{
    MyHandler.MenuMode = Mode;
    siuOpMode       = SIUMODE_START;
    sysCameraMode   = SYS_CAMERA_MODE_PLAYBACK;
    iduPlaybackMode(640,480,640);
    uiMenuOSDReset();
    #if RFIU_SUPPORT
        uiSetRfDisplayMode(UI_MENU_RF_ENTER_PLAYBACK);
    #endif
    uiGraphDrawMenu();
}
void uiFlowVideoMode(u8 WhichKey)
{
    u8 err;

    switch(WhichKey)
    {
        case UI_KEY_ENTER:
            uiCaptureVideo();
            break;

        case UI_KEY_STOP:
            if(uiCaptureVideoStop() == 0)
            {
                DEBUG_UI("Stop Voice Rec\r\n");
                sysVoiceRecStop=1;
            }
            break;

        case UI_KEY_MENU:
            #if (UI_SUPPORT_TREE == 1)
                MyHandler.MenuMode = SETUP_MODE;
                uiCaptureVideoStop();
                DEBUG_UI("change to set up mode\r\n");
                uiEnterMenu(UI_MENU_NODE_SYSTEM);
                playbackflag = 0;
                uiMenuEnable=0;
                uiFlowEnterMenuMode(SETUP_MODE);
            #else
                DEBUG_UI("Do not have Menu\r\n");
            #endif
            break;

#if (AUDIO_IN_TO_OUT == 1)
        case UI_KEY_RIGHT:
            uiOsdVolumeControl(VIDEO_MODE, UI_VALUE_ADD);
            break;

        case UI_KEY_LEFT:
            uiOsdVolumeControl(VIDEO_MODE, UI_VALUE_SUBTRACT);
            break;
#endif
        case UI_KEY_PLAY:
            //uiCaptureVideoStop();
            playbackflag        = 0;
            uiMenuEnable        = 0;
            siuOpMode           = SIUMODE_START;
            sysCameraMode       = SYS_CAMERA_MODE_PLAYBACK;
            #if IS_COMMAX_DOORPHONE
                iduPlaybackMode(800,480,800);
            #else
            iduPlaybackMode(640,480,640);
            #endif
            MyHandler.MenuMode  = PLAYBACK_MENU_MODE;
            IduVideo_ClearBuf();
            IduVidBuf0Addr      = (u32)PKBuf0;

        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
        #if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
            PlayListDspType = UI_DSP_PLAY_LIST_DOOR_SELECT;
            uiOsdDrawPlaybackMenuDoor(0);
        #else
            uiOsdDrawPlaybackMenu(0);
        #endif
            DEBUG_UI("change to Playback list\r\n");
            break;

        case UI_KEY_MODE:
            //DEBUG_UI("Start Voice Rec\r\n");
            //sysSetEvt(SYS_EVT_VOICERECORD, 0);
            break;

        case UI_KEY_DELETE:
            if(OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_SET_REC|FLAGSYS_RDYSTAT_REC_START), OS_FLAG_WAIT_CLR_ANY, &err) > 0)
            {
                DEBUG_UI("Need to stop capturing\n");
                break;
            }
            if(gInsertCard==1)
            {
                DEBUG_UI("UI Snapshot\r\n");
                sysSetEvt(SYS_EVT_SNAPSHOT, 0);
            }
            else
            {
                uiOsdDrawInsertSD(OSD_Blk2);
                osdDrawPreviewIcon();
            }
            break;

        case UI_KEY_TVOUT_DET:
            if (sysTVOutOnFlag == SYS_OUTMODE_TV)
            {
                DEBUG_UI("PANNEL MODE\r\n");
                uiSetOutputMode(SYS_OUTMODE_PANEL);
            }
            else
            {   /* switch to TV-out */
                DEBUG_UI("TV MODE\r\n");
                uiSetOutputMode(SYS_OUTMODE_TV);
            }
            break;
#if (RFIU_SUPPORT == 1)
        case UI_KEY_RF_CH:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_ADD);
            break;

        case UI_KEY_UP:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_ADD);
            break;

        case UI_KEY_DOWN:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_SUB);
            break;

        case UI_KEY_RF_QUAD:
            if (iconflag[UI_MENU_SETIDX_FULL_SCREEN] == UI_MENU_RF_FULL)
            {
                uiSetRfDisplayMode(UI_MENU_RF_QUAD);
                DEBUG_UI("UI_MENU_RF_FULL \n");
            }

            else
            {
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                DEBUG_UI("UI_MENU_RF_QUAD \n");
            }

            break;

        case UI_KEY_TALK:
            uiSetTalkOnOff();
            break;

        case UI_KEY_CH1:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_1);
            break;

        case UI_KEY_CH2:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_2);
            break;

        case UI_KEY_CH3:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_3);
            break;

        case UI_KEY_CH4:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_4);
            break;
#endif
        default:
        	#if(HW_BOARD_OPTION == MR6730_AFN)	
			if(WhichKey!=UI_KEY_NONE)
			#endif
            DEBUG_UI("Error Key %d In Video Mode\r\n",WhichKey);
            break;
    }
}

void uiFlowPlaybackMode(u8 WhichKey)
{
    switch(WhichKey)
    {
        case UI_KEY_PLAY:   /*play or pause*/
        #if(HW_BOARD_OPTION == MR6730_AFN)

			#if(USE_PLAYBACK_AUTONEXT)
			if(setUI.SYS_PlayOrder==1)	
			{
			//DEBUG_UI("==START==\n");
				if(gAutoPlay_Ceased)		
				{
					gAutoPlay_Ceased=0;//reset	
					DEBUG_UI(") AutoPlay permitted (\n");
				}
			}
			#endif

		
            if (sysThumnailPtr->type != 1)//sysThumnailPtr->type ( 0:JPG, 1:ASF/AVI, 2:WAV )
        #else
            if(dcfPlaybackCurFile->fileType != DCF_FILE_TYPE_ASF)
        #endif
            {
                DEBUG_UI("Not Asf File\r\n");
			#if( (HW_BOARD_OPTION == MR6730_AFN)&&(USE_PLAYBACK_AUTONEXT) )
			if(dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_JPG)
			{
				if(setUI.SYS_PlayOrder==1)		
				{
					OSTimeDlyHMSM(0,0,1,0);//hold for 1 sec
					//go to next 
					DEBUG_UI(">>UI_KEY_B_RIGHT<<\n");
					uiSentKeyToUi(UI_KEY_B_RIGHT);
				}
			}
			#endif
                return;
            }
            if(sysPlaybackVideoPause==1)    /*pause -> play*/
            {
                DEBUG_UI("UI playback pause -> play\r\n");
                curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
                uiOsdDrawPlaybackPlaySpeed();
                timerCountPause(1, 0);
                sysPlaybackVideoPause=0;
                osdDrawPlayIndicator(sysThumnailPtr->type);
                //uiOsdEnable(0);
                //uiOsdDisable(1);
                //uiOsdEnable(2);
                break;
            }
            if(sysPlaybackVideoStart == 1)  /*play -> pause*/
            {
                if((VideoDuration-3) != (VideoNextPresentTime/1000000))
                {
                    if (curr_playback_speed != UI_PLAYBACK_SPEED_LEVEL/2)
                    {
                        DEBUG_UI("UI playback Speed return Normal\r\n");
                        curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
                        uiOsdDrawPlaybackPlaySpeed();
						#if ( (HW_BOARD_OPTION == MR6730_AFN) && (PLAYBACK_FF_REV) )
						if(curr_playback_speed==PLAYBACK_SPEED_FF_X1)
						{
							//DEBUG_UI("osdDrawPlayIndicator:Play\n");
							osdDrawPlayIndicator(sysThumnailPtr->type);
						}
						#endif
                        break;
                    }
                    DEBUG_UI("UI playback play -> pause\r\n");
                    //timerCountPause(1, 1);
                    sysPlaybackVideoPause = 1;
                    OSTimeDly(3); // Delay for protect system
                    //uiOsdDisable(0);
                    //uiOsdDisable(1);
                    //uiOsdDisable(2);
                    osdDrawPlayIndicator(sysThumnailPtr->type);
                    //uiOsdEnable(0);
                    //uiOsdEnable(2);
                    /*wait for pause finish*/
                    OSTimeDly(2);
                    break;
                }
                else
                {
                    /* playback finish, and re-play*/
                    uiPlaybackStop(GLB_DISA);
                }
            }
            DEBUG_UI("UI playback play....\r\n");
            curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
            uiOsdDrawPlaybackPlaySpeed();
            Iframe_flag=0;  // 1: We just need I-frame 0: play whole MP4
            playbackflag = 2;
            uiMenuEnable = 0x41;
            uiOsdDrawPlayTime(0, 0);
            uiReadVideoFile();
            //sysSetEvt(SYS_EVT_ReadFile, playbackflag);
            osdDrawPlayIndicator(sysThumnailPtr->type);
            break;

#if ( (HW_BOARD_OPTION == MR6730_AFN) && (PLAYBACK_FF_REV) )
	        case UI_KEY_STOP:
            if(uiPlaybackStop(GLB_ENA) == 1)
            {
			#if 1
				
				if(sysPlaybackVideoStart == 0)
				{
					DEBUG_UI("UI playback speed return to X1 \r\n");
					//set playback speed to X1
					//curr_playback_speed = (UI_PLAYBACK_SPEED_LEVEL/2);
					curr_playback_speed = PLAYBACK_SPEED_MID;
					
					uiOsdDrawPlaybackPlaySpeed();  
				}
			#endif	            
                osdDrawPlayIndicator(100);
                uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_CLEAN);
            }
			#if(USE_PLAYBACK_AUTONEXT)
			if(setUI.SYS_PlayOrder==1)
			{
			//DEBUG_UI("==STOP==\n");
				if(dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_JPG)//for stop auto-play JPG only		
				{
					gAutoPlay_Ceased=1;	
					DEBUG_UI(") AutoPlay ceased (\n");
				}
			}
			#endif			
            break;
		
			case UI_KEY_UP: //Vol+	
			#if 1
				
				if(sysPlaybackVideoStart == 0)
				{
					DEBUG_UI("Play prev file\n");
					if(uiPlaybackStop(GLB_ENA)== 1)
						osdDrawPlayIndicator(100);
					Iframe_flag=1;	// 1: We just need I-frame 0: play whole MP4
					uiRead_Forward_VideoFile(SYS_EVT_PLAYBACK_MOVE_FORWARD);
				}
			#endif 
			#if 1
				if((sysPlaybackVideoStart == 1 || sysPlaybackVideoPause==1)&&(Iframe_flag == 0))
			#else
				if((sysPlaybackVideoStart == 1)&&(sysPlaybackVideoPause==0)&&(Iframe_flag == 0))
			#endif
					uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_ADD);
		
				break;
		
			case UI_KEY_DOWN: //Vol-
			#if 1
				
				if(sysPlaybackVideoStart == 0)
				{
					DEBUG_UI("Play next file\n");
					if(uiPlaybackStop(GLB_ENA)== 1)
						osdDrawPlayIndicator(100);
					Iframe_flag=1;	// 1: We just need I-frame 0: play whole MP4
					uiRead_Forward_VideoFile(SYS_EVT_PLAYBACK_MOVE_BACKWARD);
				}
			#endif         
			#if 1
				if((sysPlaybackVideoStart == 1 || sysPlaybackVideoPause==1)&&(Iframe_flag == 0))
			#else
				if((sysPlaybackVideoStart == 1)&&(sysPlaybackVideoPause==0)&&(Iframe_flag == 0))
			#endif
					uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_SUBTRACT);
		
				break;
		
			case UI_KEY_RIGHT: /*FF*/
				
				
				if(sysPlaybackVideoStart == 0)
				{
					DEBUG_UI("Playback not start FF\r\n");
					break;
				}
				if(sysPlaybackVideoStart==1 && sysPlaybackVideoPause==0)
				{
					DEBUG_UI("UI playback FF(%d)\r\n",curr_playback_speed);
					//if (curr_playback_speed < UI_PLAYBACK_SPEED_LEVEL-1)
					if (curr_playback_speed < PLAYBACK_SPEED_H)
						curr_playback_speed++;
					uiOsdDrawPlaybackPlaySpeed();	
					if(curr_playback_speed==PLAYBACK_SPEED_FF_X1)
					{
						//DEBUG_UI("osdDrawPlayIndicator:Play\n");
						osdDrawPlayIndicator(sysThumnailPtr->type);
					}					
				}

				break;
				
			case UI_KEY_LEFT: /*REV*/
				
		
				if(sysPlaybackVideoStart == 0)
				{
					DEBUG_UI("Playback not start REW\r\n");
					break;
				}
				if(sysPlaybackVideoStart==1 && sysPlaybackVideoPause==0)
				{
					DEBUG_UI("UI playback REW(%d)\r\n",curr_playback_speed);
					//if (curr_playback_speed > 0)
					if (curr_playback_speed > PLAYBACK_SPEED_L)
						curr_playback_speed--;
					uiOsdDrawPlaybackPlaySpeed();	
					if(curr_playback_speed==PLAYBACK_SPEED_FF_X1)
					{
						//DEBUG_UI("osdDrawPlayIndicator:Play\n");
						osdDrawPlayIndicator(sysThumnailPtr->type);
					}										
				}			

				break;
		
			case UI_KEY_DELETE:
			#if 0
				DEBUG_UI("Play next file\n");
				if(uiPlaybackStop(GLB_ENA)== 1)
					osdDrawPlayIndicator(100);
				Iframe_flag=1;	// 1: We just need I-frame 0: play whole MP4
				uiRead_Forward_VideoFile(SYS_EVT_PLAYBACK_MOVE_BACKWARD);
			#endif 
				break;
		
			case UI_KEY_ENTER:
			#if 0
				DEBUG_UI("Play prev file\n");
				if(uiPlaybackStop(GLB_ENA)== 1)
					osdDrawPlayIndicator(100);
				Iframe_flag=1;	// 1: We just need I-frame 0: play whole MP4
				uiRead_Forward_VideoFile(SYS_EVT_PLAYBACK_MOVE_FORWARD);
			#endif 
		
			#if 1
				
				if(sysPlaybackVideoStart == 1)
				{
					DEBUG_UI("UI playback speed return to X1 \r\n");
					//set playback speed to X1
					//curr_playback_speed = (UI_PLAYBACK_SPEED_LEVEL/2);
					curr_playback_speed = PLAYBACK_SPEED_MID;
					
					uiOsdDrawPlaybackPlaySpeed();  
				}

				//if(sysPlaybackVideoStart == 1 || sysPlaybackVideoPause == 1)
				{
					osdDrawPlayIndicator(sysThumnailPtr->type);
				}					

			#endif			
				break;
			
#else


        case UI_KEY_STOP:
            if(uiPlaybackStop(GLB_ENA) == 1)
            {
                osdDrawPlayIndicator(100);
                uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_CLEAN);
            }
			#if(USE_PLAYBACK_AUTONEXT)
			if(setUI.SYS_PlayOrder==1)		
			{
				if(dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_JPG)//for stop auto-play JPG only
				{
					gAutoPlay_Ceased=1;	
					DEBUG_UI(") AutoPlay ceased (\n");
				}
			}
			#endif				
            break;
            
        case UI_KEY_UP: /*FF*/
            if(sysPlaybackVideoStart == 0)
            {
                DEBUG_UI("Playback not start FF\r\n");
                break;
            }
            DEBUG_UI("UI playback FF....\r\n");
            if (curr_playback_speed < UI_PLAYBACK_SPEED_LEVEL-1)
                curr_playback_speed++;
            uiOsdDrawPlaybackPlaySpeed();
            break;

        case UI_KEY_DOWN:   /*REW*/
            if(sysPlaybackVideoStart == 0)
            {
                DEBUG_UI("Playback not start REW\r\n");
                break;
            }
            DEBUG_UI("UI playback REW....\r\n");
            if (curr_playback_speed > 0)
                curr_playback_speed--;
            uiOsdDrawPlaybackPlaySpeed();
            break;

        case UI_KEY_RIGHT:  /*vol +*/
            if((sysPlaybackVideoStart == 1)&&(sysPlaybackVideoPause==0)&&(Iframe_flag == 0))
                uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_ADD);
            break;

        case UI_KEY_LEFT:   /*vol -*/
            if((sysPlaybackVideoStart == 1)&&(sysPlaybackVideoPause==0)&&(Iframe_flag == 0))
                uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_SUBTRACT);
            break;

        case UI_KEY_DELETE:
            DEBUG_UI("Play next file\n");
            if(uiPlaybackStop(GLB_ENA)== 1)
                osdDrawPlayIndicator(100);
            Iframe_flag=1;  // 1: We just need I-frame 0: play whole MP4
            uiRead_Forward_VideoFile(SYS_EVT_PLAYBACK_MOVE_BACKWARD);
            break;

        case UI_KEY_ENTER:
            DEBUG_UI("Play prev file\n");
            if(uiPlaybackStop(GLB_ENA)== 1)
                osdDrawPlayIndicator(100);
            Iframe_flag=1;  // 1: We just need I-frame 0: play whole MP4
            uiRead_Forward_VideoFile(SYS_EVT_PLAYBACK_MOVE_FORWARD);
            break;
#endif //#if(HW_BOARD_OPTION == MR6730_AFN)

        case UI_KEY_MENU:
            uiPlaybackStop(GLB_ENA);
			#if( HW_BOARD_OPTION == MR6730_AFN)
			OSTimeDlyHMSM(0, 0, 0, 100);//delay for screen ready
			#endif
            MyHandler.MenuMode = PLAYBACK_MENU_MODE;
            uiOsdDrawPlaybackMenu(UI_KEY_MENU);
            DEBUG_UI("change to Playback list\r\n");
            break;

        case UI_KEY_MODE:
            uiPlaybackStop(GLB_ENA);
            playbackflag = 0;
            uiMenuEnable=0;
            MyHandler.MenuMode = VIDEO_MODE;
            IduVideo_ClearPKBuf(0);
            #if ( (Sensor_OPTION == Sensor_OV7725_YUV601) || (Sensor_OPTION == Sensor_OV7725_VGA)) //Lsk 090518 : for RDI project, sensor input 640*480
            if(sysTVOutOnFlag && TvOutMode==SYS_TV_OUT_PAL)
            {
                IduVideo_ClearPartPKBuf(0, 61440, 1);
                IduVideo_ClearPartPKBuf(675840,737280,1);
            }
            #endif
            IduVidBuf0Addr= (u32)PKBuf0;
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf0Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            uiMenuEnterPreview(0);
            DEBUG_UI("Playback change to preview\r\n");
            #if RFIU_SUPPORT
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
            #endif
            break;







#if( (HW_BOARD_OPTION == MR6730_AFN)&&(USE_PLAYBACK_AUTONEXT) )
		case UI_KEY_B_RIGHT: /*FF*/
			DEBUG_UI("<<UI_KEY_B_RIGHT>>\n");


				
			if(sysPlaybackVideoStart == 0)
			{
				//playback has already stopped
				DEBUG_UI("Play next file\n");
			}
			else
			{
				DEBUG_UI("UI playback FF....\r\n");

				
				#if (PLAYBACK_FF_REV)
				if(curr_playback_speed!=PLAYBACK_SPEED_FF_X1)	
					curr_playback_speed = PLAYBACK_SPEED_FF_X1;
				#else
				curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
				#endif
				osdDrawPlayIndicator(UI_PLAY_ICON_PLAY);
				uiOsdDrawPlaybackPlaySpeed();		            
				//
				if(sysPlaybackVideoStart)
				{//stop current file first
					if(uiPlaybackStop(GLB_ENA) == 1)
		            {
		                osdDrawPlayIndicator(100);
		                uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_CLEAN);
		            }	
				}
				OSTimeDlyHMSM(0,0,0,100);
				if(sysPlaybackVideoStart==0)
				{
					//uiOsdDrawPlayTime(0, 0);
			   		//osdDrawPlayIcon();
					////Iframe_flag=1;	// 1: just I-frame 
			       	////uiReadVideoFile();
			       	osdDrawPlayIndicator(UI_PLAY_ICON_STOP);
				}		
			
			}		





			#if 1
			//if (PlayListDspType == UI_DSP_PLAY_LIST_FILE)
			//DEBUG_UI("PlayListDspType=%d\r\n",PlayListDspType);//debug
			if (PlayListDspType == UI_DSP_PLAY_LIST_PLAYBACK)
			{
				u8 IsContinue=1;
				u8 IsFound=1;

				DEBUG_UI("Search next...\r\n");
				
				while(IsFound)
				{
				#if (AUTONEXT_CYCLE_DIR)

					playback_location--;
					if (!playback_location)
					{					
					#if (AUTONEXT_DIR_CIRCULATED)
						playback_location = global_totalfile_count;//wraparound to the last file
						DEBUG_UI("\n==CYCLE REPEATED==\n");

					#else

						playback_location = 1;	//stay at the 1st file	
						IsFound=0;
						IsContinue=0;//end
						break;
					#endif	
					}
					
					//(*uiPlaybackListNext[PlayListDspType])();
						dcfPlaybackFileNext();


				#else
					playback_location++;
				
					#if (AUTONEXT_DIR_CIRCULATED)
					if (playback_location>global_totalfile_count)	
					{
						playback_location = 1;	//wraparound from the 1st file	
						DEBUG_UI("\n==CYCLE REPEATED==\n");
					}
					//(*uiPlaybackListPrev[PlayListDspType])();	
						dcfPlaybackFilePrev();
					#else
					if (playback_location>global_totalfile_count)	
					{
						playback_location = global_totalfile_count;//stay at the last position
						IsFound=0;
						IsContinue=0;//end
						break;
					}
					else
					{
						//(*uiPlaybackListPrev[PlayListDspType])();	
						dcfPlaybackFilePrev();
					}
					#endif
				#endif
					
					DEBUG_UI("%s\%s (%d/%d)\r\n",dcfPlaybackCurDir->pDirEnt->d_name,dcfPlaybackCurFile->pDirEnt->d_name,playback_location,global_totalfile_count);


					//if (sysThumnailPtr->type == 1)//sysThumnailPtr->type ( 0:JPG, 1:ASF/AVI, 2:WAV )	//can't be used before read file
					if(dcfPlaybackCurFile->pDirEnt->d_name)
					{
						if(dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_JPG || dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_ASF)
						{
							IsFound=0;//found		
						}
						else
						{
							DEBUG_UI("SKIP unsupported file\n");
						}
				
					}				
				}//while(IsFound)


				//uiOsdDrawPlayTime(0, 0);
				//DEBUG_UI("IsContinue=%d\n",IsContinue);


				if(gAutoPlay_Ceased)
				{
				DEBUG_UI("==ATUOPLAY Ceased==\n");
					//if(dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_JPG)//for stop auto-play JPG only		
					{
						IsContinue=0;
					}
				}



			
				if(IsContinue)
				{
					//if(uiCheckPlayback()==0)
					{//clean screen
					
						//DEBUG_UI("Clean Screen...\n");
						uiClearOSDBuf(0);	
						//OSTimeDlyHMSM(0,0,0,100);
					}
					            

					if(dcfPlaybackCurFile->pDirEnt->d_name)
			            DEBUG_UI("==READ FILE %s ...==\n",dcfPlaybackCurFile->pDirEnt->d_name);

						
					if(dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_ASF)
					{//ASF
						Iframe_flag=1;	// 1: just I-frame 
						if (dcfPlaybackCurFile)
					   	{
						  sysReadFile();////update VideoDuration by asfReadHeaderObject()
					   	}
						Iframe_flag=0;	// 1: We just need I-frame 0: play whole MP4													
						uiReadVideoFile();

						osdDrawPlayIcon();	
						osdDrawPlayIndicator(UI_PLAY_ICON_PLAY);
					}
					else if(dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_JPG)
					{//JPG
					
						//Iframe_flag=1;	// 1: just I-frame 
						Iframe_flag=0;
						uiReadVideoFile();	
						osdDrawPlayIcon();

						#if (UI_APP_UTM)
						{
							u8 tmr_id=0;
							//DEBUG_UI("UTM:%d,%d\n",(3000/UTM_TIMEBASE_MS),g_LocalTimeInSec);
							tmr_id=UI_UserTimer_Add(UTM_CB_1,(3000/UTM_TIMEBASE_MS)+1,UTM_ONESHOT);
							if(tmr_id>UTM_NUM_MAX)
							{
								DEBUG_UI("UTM ADD ERR\n");//error
							}
							/*
							else
							{
								DEBUG_UI("UTM id=%d\n",tmr_id);//error
							}
							*/
						}
						#endif
					}

					//OSTimeDlyHMSM(0,0,5,0);//<-------------	
				}
				else
				{
					//uiClearOSDBuf(0);
					OSTimeDlyHMSM(0,0,0,999);
					DEBUG_UI("\n==CYCLE END==\n");
					DEBUG_UI("\n");
				}


				
			}

			#endif


			
			break;			
#endif




        default:
            DEBUG_UI("Error Key %d In Playback Mode\r\n",WhichKey);
            break;
    }
}

void uiFlowRunAction(void)
{
    s8 action;
    u8 set_val;
    u8 result;

    DEBUG_UI("Enter Run Action\r\n");
    set_val = uiCurrNode->item.index;
    uiCurrNode->parent->child = uiCurrNode;
    uiCurrNode = uiCurrNode->parent;
    action = uiCurrNode->item.NodeData->Action_no;
    switch(action)
    {
        case UI_MENU_SETIDX_DATE_TIME:
            set_val = 1;
            break;

        case UI_MENU_SETIDX_FORMAT:
            if (set_val == UI_MENU_FORMAT_NO)
            {
                if (MyHandler.MenuMode == GOTO_FORMAT_MODE)
                {
                    OSMboxPost(speciall_MboxEvt, "NO");
                }
                else
                    uiGraphDrawMenu();
                return;
            }
            if(Write_protet() && gInsertCard==1)
            {
                IduVideo_ClearPKBuf(0);
                osdDrawProtect(1);
                uiOsdDisable(1);
                uiGraphDrawMenu();
                return;
            }
            break;

        case UI_MENU_SETIDX_PLAYBACK:
            if (set_val == UI_MENU_DELETE_YES)
            {
                uiFlowPlayback_Delete_File();
                /* Reset default to "No" */
                uiCurrNode->child = uiCurrNode->child->right;
            }
            else
            {
                DEBUG_UI("change to playback menu mode\r\n");
                MyHandler.MenuMode = PLAYBACK_MENU_MODE;
                uiOsdDrawPlaybackMenu(UI_KEY_DELETE);
            }
            return;

        case UI_MENU_SETIDX_CARDINFO:
            uiOsdDisable(OSD_Blk0);
            uiGraphDrawMenu();
            return;

        case UI_MENU_SETIDX_PAIRING:
            #if RFIU_SUPPORT
            result = uiOsdDrawPairInMenu(set_val);
            #endif
            DEBUG_UI("Pair Mode to Preview %d\r\n",result);
            playbackflag = 0;
            uiMenuEnable=0;
            IduVideo_ClearPKBuf(0);
            #if ( (Sensor_OPTION == Sensor_OV7725_YUV601) || (Sensor_OPTION == Sensor_OV7725_VGA)) //Lsk 090518 : for RDI project, sensor input 640*480
            if(sysTVOutOnFlag && TvOutMode==SYS_TV_OUT_PAL)
            {
                IduVideo_ClearPartPKBuf(0, 61440, 1);
                IduVideo_ClearPartPKBuf(675840,737280,1);
            }
            #endif
            iduSetVideoBuf0Addr(PKBuf0);
            //uiMenuEnterPreview(0);
            #if RFIU_SUPPORT

            if (result == 1)
            {
                uiSynRfConfig(set_val);
                DEBUG_UI("Change to New Channel\r\n");
                uiSetRfChangeChannel(UI_MENU_CHANNEL_1+set_val);
            }
            else
            {
                DEBUG_UI("Change to Original Channel\r\n");
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
            }
            MyHandler.MenuMode = VIDEO_MODE;
            osdDrawPreviewIcon();
            #endif
            return;

        default:
            if (action < 0)
                return;
            break;
    }
    iconflag[action] = set_val;
    uiMenuAction(action);
    switch(action)
    {
        case UI_MENU_SETIDX_FORMAT:
            osdDrawRunFormat();
            iconflag[action] = UI_MENU_FORMAT_NO;
            if (MyHandler.MenuMode == GOTO_FORMAT_MODE)
            {
                MyHandler.MenuMode = VIDEO_MODE;
                return;
            }
            break;

        default:
            break;
    }
    uiGraphDrawMenu();
    Save_UI_Setting();
}

void uiFlowSetupMode(u8 WhichKey)
{
	u8 rowNum;

    switch(WhichKey)
    {
        case UI_KEY_MENU:
            playbackflag = 0;
            uiMenuEnable=0;
            MyHandler.MenuMode = VIDEO_MODE;
            IduVideo_ClearPKBuf(0);
            #if ( (Sensor_OPTION == Sensor_OV7725_YUV601) || (Sensor_OPTION == Sensor_OV7725_VGA)) //Lsk 090518 : for RDI project, sensor input 640*480
            if(sysTVOutOnFlag && TvOutMode==SYS_TV_OUT_PAL)
            {
                IduVideo_ClearPartPKBuf(0, 61440, 1);
                IduVideo_ClearPartPKBuf(675840,737280,1);
            }
            #endif
            IduVidBuf0Addr= (u32)PKBuf0;
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf0Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            uiMenuEnterPreview(0);
            DEBUG_UI("change to preview mode\r\n");
            #if RFIU_SUPPORT
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
            #endif
            break;

        case UI_KEY_UP:
            rowNum = uiCurrNode->item.index % UI_MENU_MAX_COLUMN;
            do
            {
                uiCurrNode = uiCurrNode->left;
            }
            while((uiCurrNode->item.index % UI_MENU_MAX_COLUMN) != rowNum);
            uiGraphDrawMenu();
            break;

        case UI_KEY_DOWN:
            rowNum = uiCurrNode->item.index % UI_MENU_MAX_COLUMN;
            do
            {
                uiCurrNode = uiCurrNode->right;
            }
            while((uiCurrNode->item.index % UI_MENU_MAX_COLUMN) != rowNum);
            uiGraphDrawMenu();
            break;

        case UI_KEY_LEFT:
            uiCurrNode = uiCurrNode->left;
            uiGraphDrawMenu();
            break;

        case UI_KEY_RIGHT:
            uiCurrNode = uiCurrNode->right;
            uiGraphDrawMenu();
            break;

        case UI_KEY_ENTER:
            if (uiCurrNode->item.NodeData->Action_no == UI_MENU_SETIDX_PLAYBACK)
            {
                DEBUG_UI("change to playback menu mode\r\n");
                MyHandler.MenuMode = PLAYBACK_MENU_MODE;
                #if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
                    PlayListDspType = UI_DSP_PLAY_LIST_DOOR_SELECT;
                    uiOsdDrawPlaybackMenuDoor(0);
                #else
                    uiOsdDrawPlaybackMenu(0);
                #endif
                break;
            }
            if (uiCurrNode->child == NULL)
            {
                uiFlowRunAction();
                break;
            }
            else
            {
                if ((uiCurrNode->item.NodeData->Action_no > 0) && (uiCurrNode->child->item.index != iconflag[uiCurrNode->item.NodeData->Action_no]))
                {
                    /*get current setting*/
                    uiCurrNode = uiCurrNode->child->right;
                    while(uiCurrNode->item.index != iconflag[uiCurrNode->parent->item.NodeData->Action_no])
                        uiCurrNode = uiCurrNode->right;
                    uiCurrNode->parent->child = uiCurrNode;
                }
                else
                    uiCurrNode = uiCurrNode->child;
            }
            if (uiCurrNode->item.NodeData->Action_no != UI_MENU_SETIDX_PLAYBACK)
                uiGraphDrawMenu();
            break;

        case UI_KEY_MODE:
            /*to last page*/
            if (uiCurrNode->parent != NULL)
            {
                if (uiCurrNode->parent->item.NodeData->Action_no < 0)
                    uiCurrNode->parent->child = uiCurrNode;
                uiCurrNode = uiCurrNode->parent;
                uiGraphDrawMenu();
            }
            break;

        default:
            DEBUG_UI("Error Key %d In Setup Mode\r\n",WhichKey);
            break;
    }
}

void uiFlowOSDSetupMode(u8 WhichKey)
{
    switch(WhichKey)
    {
        case UI_KEY_RIGHT:
            uiCurrNode = uiCurrNode->right;
            uiOsdDrawMenu();
            break;

        case UI_KEY_LEFT:
            uiCurrNode = uiCurrNode->left;
            uiOsdDrawMenu();
            break;

        case UI_KEY_ENTER:
            if (uiCurrNode->child == NULL)
            {
                uiFlowRunAction();
            }
            else
            {
                if ((uiCurrNode->item.NodeData->Action_no > 0) && (uiCurrNode->child->item.index != iconflag[uiCurrNode->item.NodeData->Action_no]))
                {
                    /*get current setting*/
                    uiCurrNode = uiCurrNode->child->right;
                    while(uiCurrNode->item.index != iconflag[uiCurrNode->parent->item.NodeData->Action_no])
                        uiCurrNode = uiCurrNode->right;
                    uiCurrNode->parent->child = uiCurrNode;
                }
                else
                    uiCurrNode = uiCurrNode->child;
                uiOsdDrawMenu();
            }
            break;

        case UI_KEY_MODE:
            /*to last page*/
            if (uiCurrNode->parent != NULL)
            {
                if (uiCurrNode->parent->item.NodeData->Action_no < 0)
                    uiCurrNode->parent->child = uiCurrNode;
                uiCurrNode = uiCurrNode->parent;
                uiClearOSDBuf(OSD_Blk0);
                uiOsdDrawMenu();
            }
            break;

        case UI_KEY_MENU:
            MyHandler.MenuMode = VIDEO_MODE;
            uiMenuOSDReset();
            (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk2, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
            osdDrawPreviewIcon();
            DEBUG_UI("OSD meun change to preview mode\r\n");
            break;

        default:
            DEBUG_UI("Error Key %d In OSD Menu Mode\r\n",WhichKey);
            break;
    }
}

void uiFlowPlaybackListMode(u8 WhichKey)
{
    switch(WhichKey)
    {
        case UI_KEY_ENTER:
        case UI_KEY_UP:
        case UI_KEY_DOWN:
        case UI_KEY_RIGHT:
        case UI_KEY_LEFT:
        case UI_KEY_MENU:
            if (PlayListDspType == UI_DSP_PLAY_LIST_DOOR_SELECT)
                uiOsdDrawPlaybackMenuDoor(MyHandler.WhichKey);
            else
                uiOsdDrawPlaybackMenu(MyHandler.WhichKey);
            break;

        case UI_KEY_DELETE:
            if(gInsertCard == 0)
                break;
            if ((PlayListDspType == UI_DSP_PLAY_LIST_FILE)&& (global_totalfile_count == 0))
                break;
            else if ((PlayListDspType == UI_DSP_PLAY_LIST_DIR)&& (global_totaldir_count == 0))
                break;
            DEBUG_UI("change to Set UP mode for delete\r\n");
            uiOsdDisableAll();
        #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_D1) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
        #else
            if (sysTVOutOnFlag)
                TV_reset();
        #endif
            uiEnterMenu(UI_MENU_NODE_DELETE_NO);
            uiGraphDrawMenu();
            MyHandler.MenuMode = SETUP_MODE;
            break;

        case UI_KEY_MODE:
            playbackflag = 0;
            uiMenuEnable=0;
            MyHandler.MenuMode = VIDEO_MODE;
            IduVideo_ClearPKBuf(0);
            #if ( (Sensor_OPTION == Sensor_OV7725_YUV601) || (Sensor_OPTION == Sensor_OV7725_VGA)) //Lsk 090518 : for RDI project, sensor input 640*480
            if(sysTVOutOnFlag && TvOutMode==SYS_TV_OUT_PAL)
            {
                IduVideo_ClearPartPKBuf(0, 61440, 1);
                IduVideo_ClearPartPKBuf(675840,737280,1);
            }
            #endif
            IduVidBuf0Addr= (u32)PKBuf0;
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf0Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            uiMenuEnterPreview(0);
            #if RFIU_SUPPORT
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
            #endif
            uiOSDPreviewInit();

            DEBUG_UI("Playback list change to preview\r\n");
            break;

        default:
            DEBUG_UI("Error Key %d In Playback List Mode\r\n",WhichKey);
            break;
    }
}

void uiFlowSetMaskArea(u8 WhichKey)
{
    switch(WhichKey)
    {
        case UI_KEY_UP:
        case UI_KEY_DOWN:
        case UI_KEY_RIGHT:
        case UI_KEY_LEFT:
        case UI_KEY_ENTER:
            uiOsdDrawMaskArea(MyHandler.WhichKey);
            break;

        case UI_KEY_MODE:   /*leave*/
            DEBUG_UI("return to set up mode\r\n");
            playbackflag = 0;
            uiMenuEnable=0;
            uiCurrNode = uiCurrNode->parent;
            uiOsdDrawMaskArea(MyHandler.WhichKey);
            uiMenuAction(uiCurrNode->item.NodeData->Action_no);
            uiFlowEnterMenuMode(SETUP_MODE);
            Save_UI_Setting();
            break;
    }
}
#if (HW_BOARD_OPTION == MR6730_WINEYE)
void uiFlowUpGradeFW(void)
{
    u8  upFlag;

    gpioBuzzerCtrl(BUZZER_SEONF05SOFF);
    gpioTimerCtrLed(LED_ALL_RE);
    //gpioTimerCtrLed(LED_R_1SEC5T);
    //gpioTimerCtrLed(LED_B_1SEC5T);

    sysSPI_Disable();
    sysSD_Enable();
    /*disable watch dog when update firmware*/
    sysDeadLockMonitor_OFF();
 //   siuMotionDetect_ONOFF(0);
    strcpy(ispFWFileName, uiUpFWKey);
    upFlag = ispUpdate(1);
    if(upFlag != 0)
    {
        osdDrawISPStatus(upFlag);
        OSTimeDly(20);
        uiClearOSDBuf(2);
//        osdDrawWarningMessage("Check UI Library!",OSD_Blk2,TRUE, FALSE);
        upFlag=ispUpdate_UILIB();
/*  if(upFlag == 0)
            osdDrawWarningMessage("No UI Library, Escape Updating!",OSD_Blk2,FALSE, FALSE);
        else if (upFlag == 1)
            osdDrawWarningMessage("UI Updated Pass!",OSD_Blk2,FALSE, FALSE);
        else
            osdDrawWarningMessage("UI Updated Fail!",2,FALSE, FALSE);
*/
    }
    /*enable watch dog when update firmware finish*/
    sysDeadLockMonitor_ON();
    gpioBuzzerCtrl(BUZZER_SEONF05SOFF);
    gpioTimerCtrLed(LED_ALL_OFF);
    strcpy(ispFWFileName, uiUpFWSDInit);
    playbackflag = 0;
    uiMenuEnable=0;
    siuOpMode = 0;
    sysCameraMode = SYS_CAMERA_MODE_PLAYBACK;
    iduPlaybackMode(0,0,0);
  //  uiGraphDrawLogo();
}
#endif

void uiEventHandler(void)
{
#if (HW_BOARD_OPTION == MR6730_WINEYE)
//  DEBUG_UI("uiEventHandler mode %d\r\n",MyHandler.MenuMode);
//  gpioVlotageCtrl(0); // Ack when receive command
    switch(MyHandler.WhichKey)
    {
        case UI_KEY_STOP:
            if(uiCaptureVideoStop()== 1)
            {
                gpioBuzzerCtrl(BUZZER_OFF);
                gpioTimerCtrLed(LED_ALL_OFF);
            }
            break;

        case UI_KEY_REC:
            /*
            if(Current_REC_Mode != UI_MENU_REC_MODE_DRIVING)
            {
                uiCaptureVideoStop();
                Current_REC_Mode = UI_MENU_REC_MODE_DRIVING;
                uiMenuAction(UI_MENU_SETIDX_REC_MODE);
                DEBUG_UI("Change to Driving Mode\r\n");
                EMFile  = 1;
                DEBUG_UI("Enter Recording mode(EM file)\r\n");
            }
            */
            if(EMFile)
                break;
            if(uiCheckVideoRec()==0)    // not in video record mode
            {
       	        if(uiCaptureVideo() == 1)   // 成功開始錄影
       	        {
                    gpioBuzzerCtrl(BUZZER_ONF05SOFF);
                    gpioTimerCtrLed(LED_R_05SEC);
                    gpioTimerCtrLed(LED_B_OFF);
                    FileType    = FILE_TYPE_EM;
                    EMFile      = 1;
       	        }
                else
                {
                    DEBUG_UI("uiCaptureVideo Fail\r\n");
                }
            }
            else    // in video record mode (normal or event video record)
            {
                //DEBUG_UI("Driving Mode do not need REC again\r\n");
                DEBUG_UI("Enter Recording mode(EM file)\r\n");
                gpioBuzzerCtrl(BUZZER_ONF05SOFF);
                gpioTimerCtrLed(LED_R_05SEC);
                gpioTimerCtrLed(LED_B_OFF);
                EMFile      = 1;
            }
            break;

        case UI_KEY_PARK:
            DEBUG_UI("Get Parking Key\r\n");
            if(Current_REC_Mode != UI_MENU_REC_MODE_PARKING)
            {
                uiCaptureVideoStop();
                Current_REC_Mode = UI_MENU_REC_MODE_PARKING;
                uiMenuAction(UI_MENU_SETIDX_REC_MODE);
                DEBUG_UI("Change to Parking Mode\r\n");
            }
            if(uiCheckVideoRec()==0)
            {
       	        if(uiCaptureVideo() == 1)
       	        {
                    gpioBuzzerCtrl(BUZZER_ON3SOFF);
                    gpioTimerCtrLed(LED_R_OFF);
                    gpioTimerCtrLed(LED_B_1SEC);
       	        }
                else
                {
                    DEBUG_UI("uiCaptureVideo Fail\r\n");
                }
            }
            else
            {
                DEBUG_UI("Parking Mode do not need REC again\r\n");
            }
            break;

        case UI_KEY_RESET:
            DEBUG_UI("Enter Reset & Rebooting\r\n");
            uiCaptureVideoStop();
            if(HasEventFile)   /*has event file*/
            {
                gpioBuzzerCtrl(BUZZER_ONF1S);
                OSTimeDly(9 * 20);
            }
            else    /*no G-Seneor file*/
            {
                gpioBuzzerCtrl(BUZZER_ON2SOFF05S);
                OSTimeDly(3 * 20);
            }
            DEBUG_UI("sysForceWdt2Reboot !@!!!!!!!!!!!!!!!!!\r\n");
            sysForceWDTtoReboot();
            break;

        case UI_KEY_BUZZER:
            DEBUG_UI("Enter BUZZER\r\n");
            if(BuzzerToggle)
            {
                DEBUG_UI("Buzzer Toggle Off\r\n");
                gpioBuzzerCtrl(BUZZER_ONF05SOFF);
                OSTimeDly(30);
                iconflag[UI_MENU_SETIDX_BUZZER] = 0;
                uiMenuAction(UI_MENU_SETIDX_BUZZER);
                Save_UI_Setting();
                gpioBuzzerCtrl(BUZZER_OFF);
            } else {
                DEBUG_UI("Buzzer Toggle On\r\n");
                iconflag[UI_MENU_SETIDX_BUZZER] = 1;
                uiMenuAction(UI_MENU_SETIDX_BUZZER);
                Save_UI_Setting();
                gpioBuzzerCtrl(BUZZER_ONF05SOFF);
            }
            //OSTimeDly(2*40);
            break;

        case UI_KEY_FORMAT:
            DEBUG_UI("Enter Format\r\n");
            uiCaptureVideoStop();
            if(Write_protet() && gInsertCard==1)
            {
                DEBUG_UI("SD Error in format Key\r\n");
                gpioBuzzerCtrl(BUZZER_ONF05SOFF1S);
                gpioTimerCtrLed(LED_ALL_FLASH);
                while(1);
                break;
            }
            gpioBuzzerCtrl(BUZZER_NONF05S);
            gpioTimerCtrLed(LED_ALL_RE);
       //    siuMotionDetect_ONOFF(0);
            siuOpMode = 0;
            sysCameraMode = SYS_CAMERA_MODE_PLAYBACK;
            iduPlaybackMode(0,0,0);
            iduOSDDisable_All();
            sysSetEvt(SYS_EVT_PLAYBACK_FORMAT, 0);
            if(osdDrawRunFormat() == 0)
            {
                DEBUG_UI("SD format success\r\n");
            }
            else
            {
                DEBUG_UI("SD format fail or timeout\r\n");
                gpioBuzzerCtrl(BUZZER_ONF05SOFF1S);
                gpioTimerCtrLed(LED_ALL_FLASH);
                while(1);

            }
            uiMenuOSDReset();
            uiMenuEnterPreview(0);
            gpioBuzzerCtrl(BUZZER_OFF);
            gpioTimerCtrLed(LED_ALL_OFF);
            sysForceWDTtoReboot(); // reboot
            while(1);
            break;

        case UI_KEY_FWUP:
            DEBUG_UI("Enter UP Grade FW\r\n");
            uiCaptureVideoStop();
            uiFlowUpGradeFW();
            DEBUG_UI("Delete %s...\n", uiUpFWKey);
            if(dcfDel(uiUpFWKey, 0) == 0)
                DEBUG_UI("Error: Delete %s Error!!\n", uiUpFWKey);
            DEBUG_UI("sysForceWdt2Reboot !@!!!!!!!!!!!!!!!!!\r\n");
			sysForceWDTtoReboot();
            DEBUG_UI("Waiting for reboot...\r\n");
            while(1);
            break;

        default:
            DEBUG_UI("Error Key %d in uiEventHandler\r\n",MyHandler.WhichKey);
            return;
    }
#else
    switch(MyHandler.MenuMode)
    {
        case VIDEO_MODE:
            uiFlowVideoMode(MyHandler.WhichKey);
            break;

        case PLAYBACK_MODE:
            uiFlowPlaybackMode(MyHandler.WhichKey);
            break;

        case SETUP_MODE:
            uiFlowSetupMode(MyHandler.WhichKey);
            break;

        case OSD_SETUP_MODE:
            uiFlowOSDSetupMode(MyHandler.WhichKey);
            break;

        case PLAYBACK_MENU_MODE:
            uiFlowPlaybackListMode(MyHandler.WhichKey);
            break;

        case SET_MASK_AREA:
            uiFlowSetMaskArea(MyHandler.WhichKey);
            break;

        case SET_NUMBER_MODE:
            if (uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_DATE_TIME)
                uiGraphDrawDateTime(MyHandler.WhichKey);
            else if (uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_NETWORK)
                uiGraphDrawNetwork(MyHandler.WhichKey);
            break;

        case GOTO_FORMAT_MODE:
            switch(MyHandler.WhichKey)
            {
                case UI_KEY_UP:
                    uiCurrNode = uiCurrNode->left;
                    uiGraphDrawMenu();
                    break;

                case UI_KEY_DOWN:
                    uiCurrNode = uiCurrNode->right;
                    uiGraphDrawMenu();
                    break;

                case UI_KEY_ENTER:
                    uiFlowRunAction();
                    break;
            }

        default:
            return;
    }
#endif

}

void uiFlowCheckP2PMode(void)
{
    u8 i;
    u8  uartCmd[16];

    if (uiP2PRestoreCfgTime == 1)
    {
        uiP2PMode = 0;
        rfiuRX_OpMode = rfiuRX_OpMode & (~RFIU_RX_OPMODE_P2P);
        sprintf((char*)uartCmd,"MODE %d 0", rfiuRX_OpMode);
        for (i = 0; i < 4; i++)
        {
            uiMenuAction(UI_MENU_SETIDX_CH1_RES+i);
            rfiu_RXCMD_Enc(uartCmd,i);
        }
    }
    if (uiP2PRestoreCfgTime != 0)
        uiP2PRestoreCfgTime--;
}


/*
 *********************************************************************************************************
 * Called by UI in other file
 *********************************************************************************************************
 */

void uiSetTVOutXY(u8 mode)
{
    if(mode == UI_MENU_SETTING_TV_OUT_PAL)
    {
        Display_X = TVOUT_X;
        Display_Y = 576;
        osdYShift = 44;
        OSDIconMidY = OSDDispHeight[sysTVOutOnFlag]/2;
        OSDIconEndY = OSDDispHeight[sysTVOutOnFlag];
        PbkOSD1 = 80;
        PbkOSD2 = 180;
    }
    else if(mode == UI_MENU_SETTING_TV_OUT_NTSC)//NTSC
    {
        Display_X = TVOUT_X;
        Display_Y = 480;
        osdYShift = 0;
        OSDIconMidY = 240/2;
        OSDIconEndY = 240;
        PbkOSD1 = 80;
        PbkOSD2 = 180;
    }
    else    /*Panel*/
    {
        Display_X = PANNEL_X;
        Display_Y = PANNEL_Y;
        osdYShift = 0;
        OSDIconMidY = OSDDispHeight[sysTVOutOnFlag]/2;
        OSDIconEndY = OSDDispHeight[sysTVOutOnFlag];
        PbkOSD1 = Display_Y/3;
        PbkOSD2 = Display_Y*3/4;
    }
    OSDIconEndX = OSDDispWidth[sysTVOutOnFlag];
    OSDIconMidX = OSDDispWidth[sysTVOutOnFlag]/2;
    PbkOSD1High = PbkOSD2-PbkOSD1;
    PbkOSD2High = OSDIconEndY - PbkOSD2;
}

void uiFlowRedirection(void)
{
#if(RFIU_SUPPORT == 1)
    if( sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR )
    {
        IDU_Init(0 , 1);
        if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight>=720)
        {
              iduPlaybackMode(RF_RX_2DISP_WIDTH,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight/2,RF_RX_2DISP_WIDTH);
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
    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA )
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
    #if RFRX_HALF_MODE_SUPPORT
        if(rfiuRX_CamOnOff_Num <= 2)
          iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
        else
    #endif
          iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);

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

void uiFlowSdCardMode(void)
{
    switch(MyHandler.MenuMode)
    {
        case VIDEO_MODE:
            break;

        case PLAYBACK_MODE:
            uiFlowPlaybackMode(UI_KEY_MODE);
            break;

        case SETUP_MODE:
        case SET_NETWORK:
        case SET_DATETIME_MODE:
        case GOTO_FORMAT_MODE:
            uiFlowSetupMode(UI_KEY_MENU);
            break;

        case PLAYBACK_MENU_MODE:
            uiFlowPlaybackListMode(UI_KEY_MODE);
            break;

        default:
            DEBUG_UI("SD Card key in Mode %d not ready\n",MyHandler.MenuMode);
            break;

    }
    uiOSDPreviewInit();

}
#if ((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION  == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
void uiChangeRFrequency(u8 FreqNO)
{
    uiTestTimeoutCnt = UI_TEST_MODE_TIMEOUT;
    switch(FreqNO)
    {
        case 0:
            DEBUG_UI("TEST MODE INTO CH1\n");
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
                gpioTimerCtrLed(LED_FLASH_CH1);
            #endif
            rfiuFCCTX0Cmd2("F 0");
            rfiuFCCTX0Cmd2("D 0");
            break;
        case 1:
            DEBUG_UI("TEST MODE INTO CH8\n");
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
                gpioTimerCtrLed(LED_FLASH_CH8);
            #endif
            rfiuFCCTX0Cmd2("F 1");
            rfiuFCCTX0Cmd2("D 0");
            break;
        case 2:
            DEBUG_UI("TEST MODE INTO CH16\n");
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
                gpioTimerCtrLed(LED_FLASH_CH16);
            #endif
            rfiuFCCTX0Cmd2("F 2");
            rfiuFCCTX0Cmd2("D 0");
            break;
         case 3:
            DEBUG_UI("TEST MODE W CH1\n");
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
                gpioTimerCtrLed(LED_FLASH_CH1);
            #endif
            rfiuFCCTX0Cmd2("F 0");
            rfiuFCCTX0Cmd2("D 2");
            break;
        case 4:
            DEBUG_UI("TEST MODE W CH8\n");
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
                gpioTimerCtrLed(LED_FLASH_CH8);
            #endif
            rfiuFCCTX0Cmd2("F 1");
            rfiuFCCTX0Cmd2("D 2");
            break;
        case 5:
            DEBUG_UI("TEST MODE W CH16\n");
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
                gpioTimerCtrLed(LED_FLASH_CH16);
            #endif
            rfiuFCCTX0Cmd2("F 2");
            rfiuFCCTX0Cmd2("D 2");
            break;
       
    	case 6:
            DEBUG_UI("TEST MODE TX CH1\n");
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
                gpioTimerCtrLed(LED_FLASH_CH1);
            #endif
		    rfiuFCCRX0Cmd("2408");
			break;

    	case 7:
            DEBUG_UI("TEST MODE TX CH8\n");
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
                gpioTimerCtrLed(LED_FLASH_CH8);
            #endif
		    rfiuFCCRX0Cmd("2440");
			break;

    	case 8:
            DEBUG_UI("TEST MODE TX CH16\n");
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
                gpioTimerCtrLed(LED_FLASH_CH16);
            #endif
		    rfiuFCCRX0Cmd("2468");
			break;	
    }
}
#endif
s32 uiKeyParse(u8 key)
{
	void* msg;
    u8 err;
    u8 uartCmd[20];
    u8 result;
	u8 ID;
	u8 recording_status;
	u8 ui_doublechack_SD,sdcheck;
    u8 ui_chack_SD, level;
    #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
    static u8 GCT_test_mode = 0;
    static u8 GCT_test_cnt = 0;
    #elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    static bool SEP_test_mode = FALSE;
    static u8 SEP_test_cnt = 0;
    #endif
    
    if((system_busy_flag || (Main_Init_Ready == 0))&& key!=UI_KEY_SDCD)
    {    // show busy message
        DEBUG_UI("Busy\n");
        return 0;
    }
	#if (HW_BOARD_OPTION==MR6730_AFN)

		#if(HW_DERIV_MODEL== HW_DEVTYPE_CDVR_AFN720PSEN)
		if(key==UI_KEY_SDCD)
		{
			if(!MACRO_UI_SET_SYSSTAT_BIT_CHK(UI_SET_SYSSTAT_BIT_SD_CHG))
			{
				MACRO_UI_SET_SYSSTAT_BIT_SET(UI_SET_SYSSTAT_BIT_SD_CHG); 
				uiDrawTimeOnVideoClip(1);	//clear timestamp
			}
		}
		#endif
		
	if(key)
	#endif     
    DEBUG_UI("uiKeyParse get key %d\n",key);
    switch (key)
    {
        case UI_KEY_UP:
        case UI_KEY_LEFT:
        case UI_KEY_DOWN:
        case UI_KEY_RIGHT:
        case UI_KEY_ENTER:
        case UI_KEY_DELETE:
        case UI_KEY_PLAY:
        case UI_KEY_MENU:
        case UI_KEY_MODE:
        case UI_KEY_STOP:
        case UI_KEY_RF_QUAD:
        case UI_KEY_RF_CH:
        case UI_KEY_TALK:
        case UI_KEY_PARK:
        case UI_KEY_REC:
        case UI_KEY_FWUP:
        case UI_KEY_FORMAT:
        case UI_KEY_RESET:
        case UI_KEY_BUZZER:
        case UI_KEY_CH1:
        case UI_KEY_CH2:
        case UI_KEY_CH3:
        case UI_KEY_CH4:
		#if (HW_BOARD_OPTION == MR6730_AFN)
		case UI_KEY_B_RIGHT:
		#endif
            MyHandler.WhichKey = key;
            uiEventHandler();
            break;


#if (HW_BOARD_OPTION == MR6730_AFN) 
#if(DERIVATIVE_MODEL==MODEL_TYPE_PUSH)
		case UI_KEY_PWR_OFF:
	#if (USE_PWR_ONOFF_KEY) 
			DEBUG_UI("uiKeyParse(UI_KEY_PWR_OFF)>>>\r\n");
		#if (PWKEY_PWRCTRL)
		
					
					
		#if 1
			if ( UI_isMenuMode() ) 
			{
				//it maybe not necessary
				if(MenuMode_isActive())
				{
					MenuMode_step(KEY_Back);
					if (!MenuMode_hasSaved())
						UI_saveSettings();
					
				}
				
				//---------------------------------------
				// cover a black canvas to hide whole screen
				
				// fill black in OSD buffer
				memset( OsdBuf_Addr(), 0xC1, (TVOSD_SizeX*TVOSD_SizeY) );
				memset( ((uint8_t *)OSD_Blk0), 0xC1, (TVOSD_SizeX*TVOSD_SizeY) );
				// turn-off TV output
				//UI_VDO_EN(0);//tvFRAME_CTL &= ~TV_FRMCTL_VDO_EN;
				
				OSD_Black_Color_Bar(1);
	
				// disable TV encoder
				//UI_VDO_EN(0);//if(MACRO_UI_TVE_EN_CHK())	UI_TVE_EN(0);	
				//---------------------------------------
	
			}//if (MenuMode_isActive()) 
		#endif
		
	
	
	
	
			
			MyHandler.WhichKey = UI_KEY_PWR_OFF;
			uiCaptureVideoStop();
			uiPlaybackStop(GLB_DISA);
			uiOsdDrawPowerOff();
			//gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 0);
	
	
		#if(USE_EXT_SV)
			g_ExtSv[15]=1;
			UI_saveSettings();
		#else
			//iconflag[UI_MENU_SETIDX_PWR_DEFAULT] = 1;
			//Save_UI_Setting();			
		#endif
			
			OSTimeDly(20);
			
			DEBUG_UI("System going to down...... \r\n");
			setUI.SYS_PowerOff=1;
		#endif//#if (PWKEY_PWRCTRL)
	#endif
			break;
#endif
#endif



        case UI_KEY_SDCD:
    #if 0
            InitsysEvt();
            InitsysbackEvt();
            MemoryFullFlag = FALSE;
            osdDrawMemFull(UI_OSD_CLEAR);
			do
            {
#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
	            LCD_Off();
	            if (UI_isVideoRecording() || UI_isVideoPlaying())
	        	{
					UI_SDLastLevel= sysCheckSDCD();
					MultiChannelSysCaptureVideoStopOneCh(1);
					SD_detect_status=0;
	        	}
#endif
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_CLR, &err);
                UI_SDLastLevel= sysCheckSDCD();
	            uiCaptureVideoStop();
	            uiPlaybackStop(GLB_DISA);
	            uiFlowSdCardMode();
                osdDrawMemFull(UI_OSD_CLEAR);
	            #if IS_COMMAX_DOORPHONE
	            UI_showSDCardCheckingMsgBox();
	            #endif
	            #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
				sdcheck= sysCheckSDCD();
				if(sdcheck==1)
					OSTimeDly(20);
	            #endif
	            uiCheckSDCD(0);
                ui_doublechack_SD = sysCheckSDCD();
                DEBUG_UI(" ---->UI_SDLastLevel =%d ,ui_doublechack_SD =%d \n ",UI_SDLastLevel,ui_doublechack_SD);
            }
			while(ui_doublechack_SD!=UI_SDLastLevel);
    #else
		#if (HW_BOARD_OPTION == MR6730_AFN) 
		MACRO_UI_SET_SYSSTAT_BIT_SET(UI_SET_SYSSTAT_BIT_SD_CHG); 
		#endif	
            if((OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY | FLAGSYS_RDYSTAT_CARD_STATUS, OS_FLAG_WAIT_SET_ANY, &err)> 0)&& (err == OS_NO_ERR))
            {
			#if (HW_BOARD_OPTION == MR6730_AFN)
				if( UI_isVideoRecording() || (sysPlaybackVideoStart != 0) ){
					sysForceWDTtoReboot();
					break;
				}
			#endif              
        #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
	            LCD_Off();
	            if (UI_isVideoRecording() || UI_isVideoPlaying())
	        	{
					UI_SDLastLevel= sysCheckSDCD();
					MultiChannelSysCaptureVideoStopOneCh(1);
					SD_detect_status=0;
	        	}
        #endif
                uiCaptureVideoStop();
                uiPlaybackStop(GLB_DISA);
                InitsysEvt();
                InitsysbackEvt();
                InitsysbackLowEvt();
                MemoryFullFlag = FALSE;
                //osdDrawMemFull(UI_OSD_CLEAR);

                sysbackLowSetEvt(SYSBACKLOW_EVT_UI_KEY_SDCD,0,0,0,0);
                DEBUG_UI("Ui SDCD CARD_READY \n");
            }
            else
            {
                ui_chack_SD = sysCheckSDCD();
                #if(HW_BOARD_OPTION == MR6730_AFN)	
                if (ui_chack_SD)
                {
                    OSMboxPost(speciall_MboxEvt, "NO");
                    //UI_gotoPreviewMode();
                    //uiFlowSetupToPreview();
                }
                #else					
                if ((MyHandler.MenuMode == GOTO_FORMAT_MODE) && (ui_chack_SD))
                {
                    OSMboxPost(speciall_MboxEvt, "NO");
                    //uiFlowSetupToPreview();
                }
				#endif 
                DEBUG_UI("Ui SDCD CARD_READY not Ready\n");
            }
    #endif
            break;

        case UI_KEY_TVOUT_DET:
            DEBUG_UI("UI_KEY_TVOUT_DET \r\n");
            if (sysTVOutOnFlag == SYS_OUTMODE_TV)
            {
                DEBUG_UI("PANNEL MODE\r\n");
                uiSetOutputMode(SYS_OUTMODE_PANEL);
            }
            else
            {   /* switch to TV-out */
                DEBUG_UI("TV MODE\r\n");
                uiSetOutputMode(SYS_OUTMODE_TV);
            }
            break;

        case UI_KEY_TV_FORMAT_CHANGE:
            if (iconflag[UI_MENU_SETIDX_TV_OUT] == UI_MENU_NTSC )
            {
                uiMenuSet_TVout_Format(UI_MENU_PAL);
                iconflag[UI_MENU_SETIDX_TV_OUT]=UI_MENU_PAL;
            }
            else
            {
                uiMenuSet_TVout_Format(UI_MENU_NTSC);
                iconflag[UI_MENU_SETIDX_TV_OUT]=UI_MENU_NTSC;

            }
            Save_UI_Setting();
            break;

		case UI_KEY_USB_DET:
            DEBUG_UI("UI_KEY_USB_DET \r\n");
            uiPlaybackStop(GLB_ENA);
            if(uiCaptureVideoStop() == 1)
            {
                OSTimeDly(10); // Delay for protect system
                gpioSetLevel(0, 25, 0);
            }
            sysSetEvt(SYS_EVT_USB, 0);
            return 0;

        case UI_KEY_UART:
            msg = OSMboxPend(uart_MboxEvt, 20, &err);
            uiCmdPareCmd(msg);
            uiEventHandler();
            break;

#if RFIU_SUPPORT
        case UI_KEY_VGA:
            sprintf((char*)uartCmd,"RESO 640 480");
            if (iconflag[UI_MENU_SETIDX_FULL_SCREEN] == UI_MENU_RF_QUAD)
            {
               rfiu_RXCMD_Enc(uartCmd, 0);
               rfiu_RXCMD_Enc(uartCmd, 1);
            }
            else  if (iconflag[UI_MENU_SETIDX_FULL_SCREEN] == UI_MENU_RF_FULL)
            {
                rfiu_RXCMD_Enc(uartCmd, sysRFRxInMainCHsel);
            }

            break;

        case UI_KEY_HD:
            sprintf((char*)uartCmd,"RESO 1280 720");
            if (iconflag[UI_MENU_SETIDX_FULL_SCREEN] == UI_MENU_RF_QUAD)
            {
               rfiu_RXCMD_Enc(uartCmd, 0);
               rfiu_RXCMD_Enc(uartCmd, 1);
            }
            else  if (iconflag[UI_MENU_SETIDX_FULL_SCREEN] == UI_MENU_RF_FULL)
            {
                rfiu_RXCMD_Enc(uartCmd, sysRFRxInMainCHsel);
            }
            break;

#if ((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
        case UI_KEY_TEST_MODE:
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
            GCT_test_mode = 1;
            DEBUG_UI("INTO GCT TEST MODE\n");
            #elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
            SEP_test_mode = TRUE;
            DEBUG_UI("INTO SEP TEST MODE\n");
            #endif
            uiTestTimeoutCnt = UI_TEST_MODE_TIMEOUT;
            sysback_RF_SetEvt(SYS_BACKRF_FCC_DIRECT_TXRX, 0);
            break;
#endif

    #if (MR8600_DEMO_USE_PKEY == 1)
        case UI_KEY_RF_DUAL:
            uiSetRfDisplayMode(UI_MENU_RF_DUAL);
            break;
	#else
        case UI_KEY_RF_PAIR:
            #if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1 ) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2 ) )
              #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
              if(uiIsVM9710==0) //VM9700
              {
                  if (GCT_test_mode == 1)
                  {
                    uiChangeRFrequency(GCT_test_cnt);
                    if (GCT_test_cnt >= 8)
                        GCT_test_cnt = 0;
                    else
                        GCT_test_cnt ++;
                  }
                  else
                  {
                    uiIsRFParing[0]=1;
                    uiOsdTXWaitPair(0);
                    uiIsRFParing[0]=0;
                  }
              }
              else //VM9710
              {
                uiIsRFParing[0]=1;
                uiOsdTXWaitPair(0);
                uiIsRFParing[0]=0;
              }
              #elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
              if (SEP_test_mode == TRUE)
              {
                uiChangeRFrequency(SEP_test_cnt);
                if (SEP_test_cnt >= 8)
                    SEP_test_cnt = 0;
                else
                    SEP_test_cnt ++;
              }
              else
              {
                uiIsRFParing[0]=1;
                uiOsdTXWaitPair(0);
                uiIsRFParing[0]=0;
              }
              #else
                #if(HW_BOARD_OPTION == MR8120_TX_FRESHCAM)
                    gpioTimerCtrLed(LED_R_FLASH);
                #endif
                uiIsRFParing[0]=1;
                uiOsdTXWaitPair(0);
                uiIsRFParing[0]=0;

                #if((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
                    (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
                    gpioTimerCtrLed(LED_ON);
                #endif
              #endif

            #else
                #if(HW_BOARD_OPTION == MR8600_RX_RDI )
                    if(uiLEDSelect == 0)
                    {
                        //uiOsdDrawPair(0);
                        //uiOsdRXWaitPair(0);
                        uiIsRFParing[0]=1;
                        uiGraphDrawPair(0);

                    }
                    else if(uiLEDSelect == 1)
                    {
                        //uiOsdDrawPair(1);
                        //uiOsdRXWaitPair(1);
                        uiIsRFParing[1]=1;
                        uiGraphDrawPair(1);

                    }
                #elif(HW_BOARD_OPTION == MR8120_RX_RDI)
                    uiIsRFParing[0]=1;
                    uiGraphDrawPair(0);
                #else
                    #if (UI_PREVIEW_OSD == 1)
                        uiOsdDrawPair(0);
                    #else
                        rfiu_PAIR_Linit(0);
                    #endif

                #endif
            #endif
            break;

        case UI_KEY_RF_PAIR1:
            #if (UI_PREVIEW_OSD == 1)
                uiOsdDrawPair(1);
            #else
                rfiu_PAIR_Linit(1);
            #endif
    #endif
    #if (HW_BOARD_OPTION == MR8120_TX_RDI_CA652)
        case UI_KEY_LIGHT:
            gpioGetLevel(GPIO_GROUP_P_LED, GPIO_BIT_P_LED, &level);
            if (level == 0)
            {
                gpioSetLevel(GPIO_GROUP_P_LED, GPIO_BIT_P_LED, 1);
                rfiu_SetRXLightTrig(1);
            }
            else
            {
                gpioSetLevel(GPIO_GROUP_P_LED, GPIO_BIT_P_LED, 0);
                rfiu_SetRXLightTrig(0);
            }
            break;
    #endif
#endif
#if (Melody_SNC7232_ENA)
        case UI_KEY_Melody_Play:
            iconflag[UI_MENU_SETIDX_MELODY] = Melody_play_num;
            Melody_SNC7232_Play(Melody_play_num);
            #if SPK_CONTROL
                if (Melody_audio_level != 0)
                    gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
            #endif
            Save_UI_Setting();
           break;
        case UI_KEY_Melody_PlayAll:
            iconflag[UI_MENU_SETIDX_MELODY] = UI_MENU_SETIDX_MELODY_ALL;
            Melody_SNC7232_Play_All();
            #if SPK_CONTROL
                if (Melody_audio_level != 0)
                    gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
            #endif
            Save_UI_Setting();
            break;
        case UI_KEY_Melody_PlayNext:
            Melody_SNC7232_PlayNext(Melody_play_num);
            #if SPK_CONTROL
                if (Melody_audio_level != 0)
                    gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
            #endif
            break;
        case UI_KEY_Melody_Stop:
            iconflag[UI_MENU_SETIDX_MELODY] = UI_MENU_SETIDX_MELODY_OFF;
            Melody_SNC7232_Stop();
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_LO);
            #endif
            Save_UI_Setting();
            break;
        case UI_KEY_Melody_Pause:
            Melody_SNC7232_PAUSE();
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_LO);
            #endif
            break;
        case UI_KEY_Melody_Start:
            Melody_SNC7232_Start();
            #if SPK_CONTROL
                if (Melody_audio_level != 0)
                    gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
                else
                    gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_LO);
            #endif
           break;
        case UI_KEY_Melody_StartAll:
            Melody_SNC7232_Start_ALL();
            #if SPK_CONTROL
                if (Melody_audio_level != 0)
                    gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
				else
                    gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_LO);
            #endif
           break;
        case UI_KEY_Melody_Volume:
            Melody_SNC7232_AudioVolume(Melody_audio_level);
            break;
#endif
#if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
        case UI_KEY_GCT_LIGHT:
            #if 1
            switch (GCT_test_mode)
            {
                case 0:
                    gpioGetLevel(GPIO_GROUP_LIGHT_LED, GPIO_BIT_LIGHT_LED, &level);
                    if (level == 0)
                        gpioSetLevel(GPIO_GROUP_LIGHT_LED, GPIO_BIT_LIGHT_LED,1);
                    else
                        gpioSetLevel(GPIO_GROUP_LIGHT_LED, GPIO_BIT_LIGHT_LED,0);
                    break;
                case 1:
                    uiTestTimeoutCnt = UI_TEST_MODE_TIMEOUT;
                    switch(GCT_test_cnt)
                    {
                        case 0:
                            DEBUG_UI("TEST MODE INTO CH1\n");
                            gpioTimerCtrLed(LED_FLASH_CH1);
                            rfiuFCCTX0Cmd2("F 0");
                            rfiuFCCTX0Cmd2("D 0");
                            break;
                        case 1:
                            DEBUG_UI("TEST MODE INTO CH8\n");
                            gpioTimerCtrLed(LED_FLASH_CH8);
                            rfiuFCCTX0Cmd2("F 1");
                            rfiuFCCTX0Cmd2("D 0");
                            break;
                        case 2:
                            DEBUG_UI("TEST MODE INTO CH16\n");
                            gpioTimerCtrLed(LED_FLASH_CH16);
                            rfiuFCCTX0Cmd2("F 2");
                            rfiuFCCTX0Cmd2("D 0");
                            break;
                         case 3:
                            DEBUG_UI("TEST MODE W CH1\n");
                            gpioTimerCtrLed(LED_FLASH_CH1);
                            rfiuFCCTX0Cmd2("F 0");
                            rfiuFCCTX0Cmd2("D 2");
                            break;
                        case 4:
                            DEBUG_UI("TEST MODE W CH8\n");
                            gpioTimerCtrLed(LED_FLASH_CH8);
                            rfiuFCCTX0Cmd2("F 1");
                            rfiuFCCTX0Cmd2("D 2");
                            break;
                        case 5:
                            DEBUG_UI("TEST MODE W CH16\n");
                            gpioTimerCtrLed(LED_FLASH_CH16);
                            rfiuFCCTX0Cmd2("F 2");
                            rfiuFCCTX0Cmd2("D 2");
                            break;
                       
                    	case 6:
                            DEBUG_UI("TEST MODE TX CH1\n");
                            gpioTimerCtrLed(LED_FLASH_CH1);
                		    rfiuFCCRX0Cmd("2408");
                			break;

                    	case 7:
                            DEBUG_UI("TEST MODE TX CH8\n");
                            gpioTimerCtrLed(LED_FLASH_CH8);
                		    rfiuFCCRX0Cmd("2440");
                			break;

                    	case 8:
                            DEBUG_UI("TEST MODE TX CH16\n");
                            gpioTimerCtrLed(LED_FLASH_CH16);
                		    rfiuFCCRX0Cmd("2468");
                			break;	
                    }
                    if (GCT_test_cnt >= 8)
                        GCT_test_cnt = 0;
                    else
                        GCT_test_cnt ++;
                    break;
            }
            #else
            if (GCT_test_mode == 1)
            {
                switch(GCT_test_cnt%3)
                {
                    case 0:
                        DEBUG_UI("TEST MODE INTO CH1\n");
                        gpioTimerCtrLed(LED_FLASH_CH1);
                        rfiuFCCTX0Cmd2("F 0");
                        rfiuFCCTX0Cmd2("D 2");
                        break;
                    case 1:
                        DEBUG_UI("TEST MODE INTO CH8\n");
                        gpioTimerCtrLed(LED_FLASH_CH8);
                        rfiuFCCTX0Cmd2("F 1");
                        rfiuFCCTX0Cmd2("D 2");
                        break;
                    case 2:
                        DEBUG_UI("TEST MODE INTO CH16\n");
                        gpioTimerCtrLed(LED_FLASH_CH16);
                        rfiuFCCTX0Cmd2("F 2");
                        rfiuFCCTX0Cmd2("D 2");
                        break;
                }
                if (GCT_test_cnt == 3)
                    GCT_test_cnt = 0;
                else
                    GCT_test_cnt ++;
            }
            else if (GCT_test_mode == 0)
            {
                gpioGetLevel(GPIO_GROUP_LIGHT_LED, GPIO_BIT_LIGHT_LED, &level);
                if (level == 0)
                    gpioSetLevel(GPIO_GROUP_LIGHT_LED, GPIO_BIT_LIGHT_LED,1);
                else
                    gpioSetLevel(GPIO_GROUP_LIGHT_LED, GPIO_BIT_LIGHT_LED,0); 
            }
            #endif
            break;
#endif
#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
        case UI_KEY_SEP_NIGHT_LIGHT:
            if(iconflag[UI_MENU_SETIDX_AP_SCONFIG_DETECT] == UI_BOOT_IN_AP_MODE)
                break;
            iconflag[UI_MENU_SETIDX_NIGHT_LIGHT] ^= 1;
            uiMenuAction(UI_MENU_SETIDX_NIGHT_LIGHT);
            Save_UI_Setting();
            rfiu_SetRXLightTrig(2+iconflag[UI_MENU_SETIDX_NIGHT_LIGHT]);
            OSTimeDly(5);
            break;

        case UI_KEY_SEP_LIGHT_ON:
            if(iconflag[UI_MENU_SETIDX_AP_SCONFIG_DETECT] == UI_BOOT_IN_AP_MODE)
                break;
            iconflag[UI_MENU_SETIDX_NIGHT_LIGHT] = UI_MENU_SETIDX_LIGHT_ON;
            uiMenuAction(UI_MENU_SETIDX_NIGHT_LIGHT);
            Save_UI_Setting();
            break;

        case UI_KEY_SEP_LIGHT_OFF:
            if(iconflag[UI_MENU_SETIDX_AP_SCONFIG_DETECT] == UI_BOOT_IN_AP_MODE)
                break;
            iconflag[UI_MENU_SETIDX_NIGHT_LIGHT] = UI_MENU_SETIDX_LIGHT_OFF;
            uiMenuAction(UI_MENU_SETIDX_NIGHT_LIGHT);
            Save_UI_Setting();
            break;
#endif
        default:
            return 0;
    }
    return 1;
}



/*when device power on, some value will always return to default*/
void uiMenuSetBootSetting(void)
{
    u8  i;
    u8  check_val=0;

    DEBUG_UI("uiMenuSetBootSetting\n");
#if (1)
    for (i = 0; i < 4; i++)
    {
        UI_CFG_RES[i] = iconflag[UI_MENU_SETIDX_VIDEO_SIZE+i];
        UI_TMP_RES[i] = UI_CFG_RES[i];
    }

#endif

#if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION  == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == MR9670_DEMO_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    memcpy ((void *)iconflag,(void *)defaultvalue,  UIACTIONNUM);
    for (i = 0; i < 4; i++)
        UI_CFG_RES[i] = iconflag[UI_MENU_SETIDX_VIDEO_SIZE+i];
#endif

#if (0)
    for (i = 0; i < 4; i++)
    {
        iconflag[UI_MENU_SETIDX_VIDEO_SIZE+i] = UI_CFG_RES[i];
    }
#endif

#if(NIC_SUPPORT)
    memcpy(StartUINetInfo.IPaddr, uiIPAddr, sizeof(uiIPAddr));
    memcpy(StartUINetInfo.Netmask, uiSubnetMask, sizeof(uiSubnetMask));
    memcpy(StartUINetInfo.Gateway, uiDefaultGateway, sizeof(uiDefaultGateway));
    StartUINetInfo.IsStaticIP = uiISStatic;
#endif
    memcpy ((void *)start_iconflag,(void *)iconflag,  UIACTIONNUM);
    iconflag[UI_MENU_SETIDX_FORMAT] = UI_MENU_FORMAT_NO;
    #if( (HW_BOARD_OPTION == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8120_TX_DB2 ) ||\
        (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
      #if USE_704x480_RESO
       iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_D1_480V;
      #else
       iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_HD;
      #endif
    #endif
    iconflag[UI_MENU_SETIDX_DATE_TIME] = 0;
    memcpy(&StartMotMask[0][0], &MotionMaskArea[0][0], MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN);

    check_val = uiGetSaveChecksum();
    if (iconflag[UI_MENU_SETIDX_CHECK] != check_val){
        #if(TUTK_SUPPORT)
            memcpy(&uiStartP2PID, &uiP2PID, sizeof(uiP2PID));
        #endif

        #if(NIC_SUPPORT)
            memcpy(&uiStartMACAddr, &uiMACAddr, sizeof(uiMACAddr));
        #endif
    }
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
    memcpy((void *)uiStartLightTimer, (void *)uiLightTimer, sizeof(uiLightTimer));
#endif
#if (UI_SUPPORT_TREE == 1)
    uiGraphGetTimePhotoID();
    uiGraphGetNetwokPhotoID();
#endif

#if ( (HW_BOARD_OPTION == MR8600_RX_RDI )  || (HW_BOARD_OPTION == MR8120_RX_RDI) )
    uiGraphGetPairPhotoID();
#endif

    for(i=0;i<UIACTIONNUM;i++)
        uiMenuAction(i);
}

void uiWaitMainInitReady(void)
{
    u8  err;
    u32 waitFlag;
    u8  level;

    waitFlag = OSFlagPend(gUiStateFlagGrp, (FLAGUI_MAIN_INIT_READY|FLAGUI_SD_GOTO_FORMAT), OS_FLAG_WAIT_SET_ANY, OS_IPC_WAIT_FOREVER, &err);
    if(waitFlag & FLAGUI_MAIN_INIT_READY)
    {
	#if ((HW_BOARD_OPTION == MR6730_AFN))

			if ( iconflag[UI_MENU_SETIDX_PWR_DEFAULT] == 1)
			{
				iconflag[UI_MENU_SETIDX_PWR_DEFAULT] = 0;
				Save_UI_Setting();
			}
    #elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
        uiBootPressKey = 1;
        uiBootPressTic = OSTimeGet();
    #elif (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
        gpioSetLevel(GPIO_GROUP_LIGHT_LED, GPIO_BIT_LIGHT_LED, 1);
        if (uiIsVM9710 == 1)
            gpioGetLevel(GPIO_GROUP_LIGHT_SW, GPIO_BIT_LIGHT_SW, &level);
        else if (uiIsVM9710 == 0)
        {
            gpioSetLevel(GPIO_GROUP_LED_RF, GPIO_BIT_LED_RF, 1);
            OSTimeDly(3);
            gpioSetLevel(GPIO_GROUP_LED_RF, GPIO_BIT_LED_RF, 0);
            level = 0;
        }
        if (level == 0)
        {
            uiBootPressKey = 1;
            uiBootPressTic = OSTimeGet();
        }
        #if Melody_SNC7232_ENA
        if (iconflag[UI_MENU_SETIDX_MELODY] != UI_MENU_SETIDX_MELODY_OFF)
        {
            if (iconflag[UI_MENU_SETIDX_MELODY] == UI_MENU_SETIDX_MELODY_ALL)
                Melody_SNC7232_Play_All();
            else
                Melody_SNC7232_Play(iconflag[UI_MENU_SETIDX_MELODY]);
            uiMenuSet_SNC7232_AudioVolume_init(iconflag[UI_MENU_SETIDX_VOLUME]);
        }
        #endif
    #endif

    #if RF_TX_OPTIMIZE

    #else
        #if ( (CHIP_OPTION >= CHIP_A1013A) && (RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2) || (HW_BOARD_OPTION==MR6730_WINEYE) || (HW_BOARD_OPTION  == MR8211_ZINWELL))
        #else
            if ((gInsertCard==1) && iconflag[UI_MENU_SETIDX_IS_REC] == 1)
        #endif
            {
            #if ( (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MOV) == 0)
                DEBUG_UI("uiTask Start capture video\r\n");
                uiCaptureVideo();
            #endif
            }
    #endif
    }
    #if (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
        i2cWrite_WT6853(0x03, 0x0);
        i2cWrite_WT6853(0x01, 0x0);
    #endif
}

#if( (Sensor_OPTION == Sensor_CCIR601) ||(Sensor_OPTION == Sensor_CCIR656)|| \
    (Sensor_OPTION == Sensor_OV7725_YUV601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV) ||\
    (Sensor_OPTION == Sensor_OV7740_RAW) || (Sensor_OPTION == Sensor_OV7740_YUV601) || \
    (Sensor_OPTION == Sensor_MI1320_YUV601) || (Sensor_OPTION == Sensor_OV2643_YUV601) || \
    (Sensor_OPTION == Sensor_MT9M131_YUV601) || (Sensor_OPTION == Sensor_HM5065_YUV601) || (Sensor_OPTION == Sensor_HM1375_YUV601) || (Sensor_OPTION == Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_PO3100K_YUV601))
TV_FORMAT_STATUS uiChangeTVFormat(void)
{
    u32 CurrState = TV_IN_NTSC;
    u8  StopCapture, err;
    u8  retVal = 0;

    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_TVinFormat, OS_FLAG_SET, &err);
    CurrState=getTVinFormat();
    if (sysTVinFormat == CurrState)
        return TVFORMAT_NO_CHANGE;
    sysTVinFormat = CurrState;

    DEBUG_UI("TV-in Format is Changed in UI %d\n",sysTVinFormat);
    if(sysTVinFormat  == TV_IN_PAL)
       TvOutMode = SYS_TV_OUT_PAL;
    else
       TvOutMode = SYS_TV_OUT_NTSC;
    uiSetTVOutXY(TvOutMode);
    uiPlaybackStop(GLB_ENA);
    StopCapture = uiCaptureVideoStop();
    uiMenuAction(UI_MENU_SETIDX_VIDEO_SIZE);

    /*return to video mode*/
    if (MyHandler.MenuMode != VIDEO_MODE)
    {
        MyHandler.MenuMode=VIDEO_MODE;
        playbackflag = 0;
        uiMenuEnable=0;
        uiMenuOSDReset();
        uiMenuEnterPreview(0);
        return TVFORMAT_RTN_PREVIEW;
    }

    if(StopCapture == 1)
        retVal = uiCaptureVideo();
    if((StopCapture == 0) || (retVal == 0))
    {
        isuStop();  //Lucian: 080616
        ipuStop();
        siuStop();
        //while (SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
        setSensorWinSize(0, SIUMODE_PREVIEW);

        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
           iduPreview(640,480);
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduPreview(704,480);
           else
              iduPreview(704,576);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduPreview(720,480);
           else
              iduPreview(720,576);
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
           iduPreview(1280,720);
        }
        else
           iduPreview(640,480);

        isuPreview(0);
        ipuPreview(0);
        siuPreview(0);
        if(StopCapture == 0)
            return TVFORMAT_STILL_PREVIEW;
        else
            return TVFORMAT_CAPTURE_FAIL;
    }
    return TVFORMAT_CAPTURE_SUCCESS;
}
#endif

void uiEnterTVMode(void)
{
    sysTVOutOnFlag=1;
#if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) )
    if(sysTVinFormat  == TV_IN_PAL)
    {
        TvOutMode = SYS_TV_OUT_PAL;
    }
    else
    {
        TvOutMode = SYS_TV_OUT_NTSC;
    }
#else
    if(sysTVinFormat  == TV_IN_PAL)
    {
        TvOutMode = SYS_TV_OUT_PAL;
	    AE_Flicker_50_60_sel = 1 ;
    }
    else
    {
        TvOutMode = SYS_TV_OUT_NTSC;
	    AE_Flicker_50_60_sel = 0;
    }
#endif


    iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);

    ResetisuPanel();
    uiSetTVOutXY(TvOutMode);
    playbackflag = 0;
    uiMenuEnable=0;
}

void uiEnterPanelMode(void)
{
    sysTVOutOnFlag=0;
    if(sysTVinFormat  == TV_IN_PAL)
    {
      TvOutMode = SYS_TV_OUT_PAL;
	  AE_Flicker_50_60_sel = 1 ;
    }
    else
    {
      TvOutMode = SYS_TV_OUT_NTSC;
	  AE_Flicker_50_60_sel = 0;
    }
    sysIDU_enable();
    iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
    ResetisuPanel();
    uiSetTVOutXY(UI_MENU_SETTING_PANEL_OUT);
    playbackflag = 0;
    uiMenuEnable=0;
}

void uiMenuTreeInit(void)
{
    uiEnterMenu(0);
#if (UI_BOOT_FROM_PANEL == 1)
    /*Boot Enter Panel Out*/
    uiEnterPanelMode();
#else   /*Boot Enter TV Out*/
    uiEnterTVMode();
#endif
    MyHandler.MenuMode = VIDEO_MODE;
#if ( (HW_BOARD_OPTION == MR8120_TX_TRANWO) || (HW_BOARD_OPTION == MR8120_TX_JIT)||\
    (HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
    (HW_BOARD_OPTION  == MR8100_GCT_VM9710) ||(HW_BOARD_OPTION == MR8120_TX_TRANWO3))
	gpioDetectIRLED();
#endif
}

#if(HW_BOARD_OPTION == MR8120_TX_JESMAY )
void uiFlowRFStatus(u32 waitFlag)
{
    if(waitFlag & (RFIU_TX_STA_LINK_BROKEN ))
    {
        gpioTimerCtrLed(LED_R_OFF);
    }
    else if(waitFlag & (RFIU_TX_STA_LINK_OK ))
    {

        gpioTimerCtrLed(LED_R_ON);
    }
}

#elif(HW_BOARD_OPTION == MR8120_TX_JESMAY_LCD )
void uiFlowRFStatus(u32 waitFlag)
{
    if(waitFlag & (RFIU_TX_STA_LINK_BROKEN ))
    {
        gpioTimerCtrLed(LED_ON);
    }
    else if(waitFlag & (RFIU_TX_STA_LINK_OK ))
    {

        gpioTimerCtrLed(LED_FLASH);
    }
}
#elif(HW_BOARD_OPTION == MR8120_RX_JESMAY)
void uiFlowRFStatus(u32 waitFlag)
{
    static u8 brokenCount=0;
    if((waitFlag & (RFIU_RX_STA_LINK_BROKEN )) && (UIKey != UI_KEY_RF_PAIR ) )
    {
        if(brokenCount == 0 )
        {
            gpioTimerCtrLed(LED_R_OFF);
            uiIsRFBroken=1;
            uiOSDDrawNoSignal();
            iisMute(1);
        }

        brokenCount++;
    }
    else if(waitFlag & (RFIU_RX_STA_LINK_OK ))
    {
        brokenCount=0;
        gpioTimerCtrLed(LED_R_ON);
        uiOSDPreviewInit();
        uiIsRFBroken=0;
        iisMute(0);
    }

}
#elif (HW_BOARD_OPTION == MR8120_TX_FRESHCAM)
void uiFlowRFStatus(u32 waitFlag)
{

    if(waitFlag & (RFIU_TX_STA_LINK_BROKEN ) && (gRfiu_Op_Sta[0] ==RFIU_TX_STA_LINK_BROKEN) && (uiIsRFParing[0]==0) )
    {
        gpioTimerCtrLed(LED_R_OFF);
        uiIsRFBroken=1;

    }
    else if(waitFlag & (RFIU_TX_STA_LINK_OK ) && (gRfiu_Op_Sta[0] ==RFIU_TX_STA_LINK_OK)  )
    {
        gpioTimerCtrLed(LED_R_ON);
        uiIsRFBroken=0;

    }
    else if(waitFlag & (RFIU_TX_STA_PAIR_OK ) && (gRfiu_Op_Sta[0] ==RFIU_TX_STA_PAIR_OK))
    {
        //gpioTimerCtrLed(LED_R_ON);
        uiIsRFBroken=0;
    }

}



#elif (HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS)
void uiFlowRFStatus(u32 waitFlag)
{
    static u8 brokenCount=0;
    if((waitFlag & (RFIU_RX_STA_LINK_BROKEN )) && (UIKey != UI_KEY_RF_PAIR ) )
    {
        if(brokenCount == 0 )
        {
            gpioTimerCtrLed(LED_R_OFF);
            uiIsRFBroken=1;
            uiOSDDrawNoSignal();
        }

        brokenCount++;
    }
    else if(waitFlag & (RFIU_RX_STA_LINK_OK ))
    {
        brokenCount=0;
        gpioTimerCtrLed(LED_R_ON);
        uiIsRFBroken=0;

    }

}


#elif(HW_BOARD_OPTION == MR8600_RX_RDI )
void uiFlowRFStatus(u32 waitFlag)
{
    u8  i;
    u8  uartCmd[20];
    u8  check=0;
    static u8 count_error=0;
    if(waitFlag > 0)
    {

        for(i=0;i<2;i++)
        {

            if(waitFlag & (RFIU_RX_STA_LINK_BROKEN << (i*8))&&(uiIsRFParing[i]==0))
            {
                if(i==0)
                {
                    memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM);

                    gpioTimerCtrLed(LED_L_FLASH);
                }
                else
                {
                    memset_hw_Word(Sub1Videodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM);

                    gpioTimerCtrLed(LED_R_FLASH);
                }

                uiGraphDrawNoSignal(i);

            }
            else if(waitFlag & (RFIU_RX_STA_LINK_OK << (i*8)))
            {
                if(i==0)
                {
                    gpioTimerCtrLed(LED_L_ON);
                }
                else
                {
                    gpioTimerCtrLed(LED_R_ON);
                }
                uiIsRFParing[i]=0;
                count_error=0;
            }
            else if(waitFlag & (RFIU_RX_STA_PAIR_OK << (i*8)))
            {
                count_error=0;
                if(i==0)
                {
                    gpioTimerCtrLed(LED_L_ON);
                }
                else
                {
                    gpioTimerCtrLed(LED_R_ON);
                }
                uiIsRFParing[i]=0;
            }
            else if(waitFlag & (RFIU_RX_STA_ERROR << (i*8)))
            {
                if(count_error==0)
                {
                    if(i==0)
                    {
                        uiSetRfResolutionRxToTx(UI_RESOLTUION_VGA,0);
                    }
                    else
                    {
                        uiSetRfResolutionRxToTx(UI_RESOLTUION_VGA,1);
                    }
                    count_error=1;
                }
            }
        }
    }
}
#elif(HW_BOARD_OPTION == MR8120_RX_RDI)
void uiFlowRFStatus(u32 waitFlag)
{
    u8 camera=0;

    if(waitFlag > 0)
    {


        if(waitFlag & (RFIU_RX_STA_LINK_BROKEN )&&(uiIsRFParing[camera]==0))
        {

            memset_hw_Word(MainVideodisplaybuf[camera], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM);
            gpioTimerCtrLed(LED_L_FLASH);
            uiGraphDrawNoSignal(camera);

        }
        else if(waitFlag & RFIU_RX_STA_LINK_OK )
        {
            gpioTimerCtrLed(LED_L_ON);
            uiIsRFParing[camera]=0;
        }
        else if(waitFlag & RFIU_RX_STA_PAIR_OK)
        {
            gpioTimerCtrLed(LED_L_ON);
            uiIsRFParing[camera]=0;
        }

    }
}


#elif((SW_APPLICATION_OPTION != MR8120_RFCAM_TX1)  && ((SW_APPLICATION_OPTION != MR8100_RFCAM_TX1)) && (SW_APPLICATION_OPTION != MR8211_RFCAM_TX1) && (SW_APPLICATION_OPTION != MR8120_RFCAM_TX1_6M) && (SW_APPLICATION_OPTION != MR8120_RFCAM_TX2) )
void uiFlowRFStatus(u32 waitFlag)
{
#if(RFIU_SUPPORT)
    u8 i;

    if(MyHandler.MenuMode != VIDEO_MODE)
    {
        for(i=0;i<4;i++)
            uiRFStatue[i] = UI_OSD_NONE;
        return;
    }
    for(i=0;i<4;i++)
    {
        if(waitFlag & (RFIU_RX_STA_LINK_OK << (i*8)))
        {
            uiSynRfConfig(i);
        }
    }
    //DEBUG_UI("========== uiRFStatue[sysRFRxInMainCHsel] is %d ==========\r\n",uiRFStatue[sysRFRxInMainCHsel]);
    if (iconflag[UI_MENU_SETIDX_FULL_SCREEN] == UI_MENU_RF_QUAD)
    {
        for ( i = 0; i < 4; i++)
        {
            if(waitFlag & (RFIU_RX_STA_LINK_BROKEN << (i*8)))
            {
                if(uiRFStatue[i] != 0)
                {
                    uiOsdDrawQuadNoSignal(UI_OSD_DRAW, i);
                    uiRFStatue[i]=0;
                    DEBUG_UI("========== %d link broken ==========\r\n",i);
                }
            }
            else if(waitFlag & (RFIU_RX_STA_LINK_OK << (i*8)))
            {
                if(uiRFStatue[i] != 1)
                {
                    uiOsdDrawQuadNoSignal(UI_OSD_CLEAR, i);
                    uiRFStatue[i]=1;
                    DEBUG_UI("========== %d link OK ==========\r\n",i);
                }
            }
        }

    }
    else
    {
        if(waitFlag & (RFIU_RX_STA_LINK_BROKEN << (sysRFRxInMainCHsel*8)))
        {
            if(uiRFStatue[sysRFRxInMainCHsel] != UI_OSD_DRAW)
            {
                uiOsdDrawNoSignal(UI_OSD_DRAW);
                uiRFStatue[sysRFRxInMainCHsel]=UI_OSD_DRAW;
                //DEBUG_UI("========== %d link broken ==========\r\n",sysRFRxInMainCHsel);
            }
        }
        else if(waitFlag & (RFIU_RX_STA_LINK_OK << (sysRFRxInMainCHsel*8)))
        {
            if(uiRFStatue[sysRFRxInMainCHsel] != UI_OSD_CLEAR)
            {
                uiOsdDrawNoSignal(UI_OSD_CLEAR);
                uiRFStatue[sysRFRxInMainCHsel]=UI_OSD_CLEAR;
                //DEBUG_UI("========== %d link OK ==========\r\n",sysRFRxInMainCHsel);
            }
        }
        else if(waitFlag & (RFIU_RX_STA_CHGRESO << (sysRFRxInMainCHsel*8)))
        {
            uiOsdDrawChangeResolution(UI_OSD_DRAW);
            uiRFStatue[sysRFRxInMainCHsel]=0;
        }

    }
#endif
}
#elif(HW_BOARD_OPTION == MR8120_TX_HECHI)
void uiFlowRFStatus(u32 waitFlag)
{
    if(waitFlag & (RFIU_TX_STA_LINK_BROKEN ))
    {

    }
    else if(waitFlag & (RFIU_TX_STA_LINK_OK ))
    {

        /* pga gain set 0db*/
        DacTxCtrl &= ~0x0fff1f1d;
        //DacTxCtrl |=  0x0f99161d;
        DacTxCtrl |=   0x0fa1161d;
    }
}
#else
void uiFlowRFStatus(u32 waitFlag)   //for TX
{

}
#endif



/*
 *********************************************************************************************************
 * Called by other moudle
 *********************************************************************************************************
 */

u8 uiCompareSaveData(void)
{
    u8 update = 0;

    if (memcmp((void*)start_iconflag, (void*)iconflag, UIACTIONNUM) != 0)
    {
        memcpy ((void *)start_iconflag,(void *)iconflag,  UIACTIONNUM);
        update = 1;
    }
    if(memcmp (&StartSchTimeFrom, &ScheduledTimeFrom, sizeof(RTC_DATE_TIME)) != 0)
    {
        memcpy(&StartSchTimeFrom, &ScheduledTimeFrom,  sizeof(RTC_DATE_TIME));
        update = 1;
    }
    if (memcmp(&StartSchTimeTo, &ScheduledTimeTo, sizeof(RTC_DATE_TIME)) != 0)
    {
        memcpy(&StartSchTimeTo, &ScheduledTimeTo,  sizeof(RTC_DATE_TIME));
        update = 1;
    }
    if (memcmp(&StartMotMask[0][0], &MotionMaskArea[0][0], MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN) != 0)
    {
        memcpy(&StartMotMask[0][0], &MotionMaskArea[0][0],  MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN);
        update = 1;
    }
#if(TUTK_SUPPORT)
    if (memcmp(&uiStartP2PID, &uiP2PID, sizeof(uiP2PID)) !=0)
    {
        memcpy(&uiStartP2PID, &uiP2PID, sizeof(uiP2PID));
        update = 1;
    }
    if (memcmp(&UI_P2P_PSW, &Start_UI_P2P_PSW, sizeof(UI_P2P_PSW)) !=0)
    {
        memcpy(&Start_UI_P2P_PSW, &UI_P2P_PSW, sizeof(UI_P2P_PSW));
        update = 1;
    }
#endif
#if(RFIU_SUPPORT)
    if (memcmp(&uiStartRFID, &uiRFID, sizeof(uiRFID)) !=0)
    {
        memcpy(&uiStartRFID, &uiRFID, sizeof(uiRFID));
        update = 1;
    }
    if (memcmp(&uiStratRFCode, &uiRFCODE, sizeof(uiRFCODE)) !=0)
    {
        memcpy(&uiStratRFCode, &uiRFCODE, sizeof(uiRFCODE));
        update = 1;
    }
#endif
    if (memcmp(&UI_CFG_RES, &UI_TMP_RES, sizeof(UI_CFG_RES)) !=0)
    {
        memcpy(&UI_TMP_RES, &UI_CFG_RES, sizeof(UI_CFG_RES));
        update = 1;
    }

#if(NIC_SUPPORT)
    if (memcmp(&uiStartMACAddr, &uiMACAddr, sizeof(uiMACAddr)) !=0)
    {
        memcpy(&uiStartMACAddr, &uiMACAddr, sizeof(uiMACAddr));
        update = 1;
    }
    if (memcmp(&UINetInfo, &StartUINetInfo, sizeof(UI_NET_INFO)) !=0)
    {
        memcpy(&StartUINetInfo, &UINetInfo, sizeof(UI_NET_INFO));
        update = 1;
    }
#endif
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    if (memcmp(&TempHighMargin, &StartTempHighMargin, sizeof(TempHighMargin)) !=0)
    {
        memcpy(&StartTempHighMargin, &TempHighMargin, sizeof(TempHighMargin));
        update = 1;
    }
    if (memcmp(&TempLowMargin, &StartTempLowMargin, sizeof(TempLowMargin)) !=0)
    {
        memcpy(&StartTempLowMargin, &TempLowMargin, sizeof(TempLowMargin));
        update = 1;
    }
#endif
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
    if (memcmp(uiStartLightTimer, uiLightTimer, sizeof(uiLightTimer)) !=0)
    {
        memcpy(uiStartLightTimer, uiLightTimer, sizeof(uiLightTimer));
        update = 1;
    }

#endif
#if ICOMMWIFI_SUPPORT
    if (memcmp(&JOIN_DEFAULT_SSID, &JOIN_DEFAULT_SSID_CHECK, sizeof(JOIN_DEFAULT_SSID)) !=0)
    {
        memcpy(&JOIN_DEFAULT_SSID_CHECK, &JOIN_DEFAULT_SSID, sizeof(JOIN_DEFAULT_SSID));
        update = 1;
    }
    if (memcmp(&JOIN_DEFAULT_PSK, &JOIN_DEFAULT_PSK_CHECK, sizeof(JOIN_DEFAULT_PSK)) !=0)
    {
        memcpy(&JOIN_DEFAULT_PSK_CHECK, &JOIN_DEFAULT_PSK, sizeof(JOIN_DEFAULT_PSK));
        update = 1;
    }
#endif
    return update;
}

u8 uiGetSaveChecksum(void)
{
    u8 i, j, check_val = 0;
    u8 len;

    /*avoid warning message*/
    if(j){}
#if(RFIU_SUPPORT)
    for (i = 0; i < 4; i++)
        iconflag[UI_MENU_SETIDX_VIDEO_SIZE+i] = UI_CFG_RES[i];
#endif
    for (i = 0; i < UIACTIONNUM-1; i++)
        check_val += iconflag[i];
    check_val += (ScheduledTimeFrom.year+ScheduledTimeFrom.month+ScheduledTimeFrom.day+ScheduledTimeFrom.hour+ScheduledTimeFrom.min+ScheduledTimeFrom.sec);
    check_val += (ScheduledTimeTo.year+ScheduledTimeTo.month+ScheduledTimeTo.day+ScheduledTimeTo.hour+ScheduledTimeTo.min+ScheduledTimeTo.sec);
    for ( i = 0; i < MASKAREA_MAX_ROW; i++)
        for (j = 0; j < MASKAREA_MAX_COLUMN; j++)
            check_val += MotionMaskArea[i][j];
    check_val += UI_SET_CHECKSUM;
#if(NIC_SUPPORT)
    for (i = 0; i< 3; i++)
    {
        check_val += UINetInfo.IPaddr[i];
        check_val += UINetInfo.Netmask[i];
        check_val += UINetInfo.Gateway[i];
    }
    check_val += UINetInfo.IsStaticIP;
#endif
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
    for (i = 0; i< 4; i++)
    {
        check_val += uiLightTimer[i];
    }
#endif
#if ICOMMWIFI_SUPPORT
    len = strlen(JOIN_DEFAULT_SSID);
    for (i = 0; i< len; i++)
    {
        check_val += JOIN_DEFAULT_SSID[i];
    }
    len = strlen(JOIN_DEFAULT_PSK);
    for (i = 0; i< len; i++)
    {
        check_val += JOIN_DEFAULT_PSK[i];
    }
#endif
    return check_val;
}

/*

Routine Description:

	Stop Capture Video

Arguments:

	None.

Return Value:

	0 - Not need to stop.
	1 - Success.

*/
u8 uiCaptureVideoStopByChannel(u8 Ch_ID)
{
    if (Ch_ID >= MULTI_CHANNEL_MAX)
    {
        DEBUG_UI("Stop Capture Video Error Channel %d\r\n",Ch_ID);
        return 0;
    }

    DEBUG_UI("uiCaptureVideoStopByChannel %d\r\n",Ch_ID);
    #if MULTI_CHANNEL_VIDEO_REC
    if ( MultiChannelSysCaptureVideoStopOneCh(Ch_ID+MULTI_CHANNEL_LOCAL_MAX) == 1)
    {
        if ((MyHandler.MenuMode != SETUP_MODE) && (MyHandler.MenuMode != SET_NUMBER_MODE))
        {
            iconflag[UI_MENU_SETIDX_IS_REC] &= ~(0x1 << Ch_ID);
            Save_UI_Setting();
        }
        return 1;
    }
    #endif
    return 0;

}


/*

Routine Description:

	Capture Video by Channel

Arguments:

	None.

Return Value:

	0 - Fail.
	1 - Success.

*/
u8 uiCaptureVideoByChannel(u8 Ch_ID)
{
    u8  temp;

    if (Ch_ID >= MULTI_CHANNEL_MAX)
    {
        DEBUG_UI("Capture Video Error Channel %d\r\n",Ch_ID);
        return 0;
    }
    if ((MemoryFullFlag == TRUE) && (SysOverwriteFlag == FALSE))
    {
        DEBUG_UI("Ch %d SD Card FULL!!!!!\r\n",Ch_ID);
        return 0;
    }

#if 0
    if(gRfiu_Op_Sta[Ch_ID] == RFIU_RX_STA_LINK_BROKEN)
    {
        DEBUG_UI("Ch %d RF Link OFF in recording!!!!!\r\n",Ch_ID);
        return 0;
    }
#endif

    if(gInsertCard==1)
    {
    #if MULTI_CHANNEL_VIDEO_REC
        DEBUG_UI("uiCaptureVideoByChannel %d\r\n",Ch_ID);
        if (MultiChannelGetCaptureVideoStatus(Ch_ID+MULTI_CHANNEL_LOCAL_MAX) != 0)
            uiCaptureVideoStopByChannel(Ch_ID);
        temp = (u8)MultiChannelSysCaptureVideoOneCh(Ch_ID+MULTI_CHANNEL_LOCAL_MAX);
        if (temp == 1)
        {
            iconflag[UI_MENU_SETIDX_IS_REC] |= (0x1 << Ch_ID);
            Save_UI_Setting();
        }
        return temp;
    #endif
    }
    else
    {
        DEBUG_UI("No SD Card\r\n");
        uiOsdDrawInsertSD(UI_OSD_DRAW);
        return 0;
    }
}

/*

Routine Description:

	Stop Capture Video

Arguments:

	None.

Return Value:

	0 - Fail.
	1 - Success.

*/
u8 uiCaptureVideo(void)
{
    if(gInsertCard==1)
    {
        DEBUG_UI("UI Start capture video\r\n");
#if (MULTI_CHANNEL_VIDEO_REC && MULTI_CHANNEL_RF_RX_VIDEO_REC)
        RfRxVideoPackerEnable();
#endif
        if(uiKeyVideoCapture() == 1)
        {
            iconflag[UI_MENU_SETIDX_IS_REC]= 1;
            if((SW_APPLICATION_OPTION != MR8120_RFCAM_TX1) && ((SW_APPLICATION_OPTION != MR8100_RFCAM_TX1)) && (SW_APPLICATION_OPTION != MR8211_RFCAM_TX1) && (SW_APPLICATION_OPTION != MR8120_RFCAM_TX1_6M) && (SW_APPLICATION_OPTION != MR8120_RFCAM_TX2))
            {
                 Save_UI_Setting();
            }

            return 1;
        }
        else
            DEBUG_UI("CaptureVideo Fail\r\n");
    }
    else
    {
        DEBUG_UI("No SD Card\r\n");
        uiClearOSDBuf(OSD_Blk2);
        uiOsdDrawInsertSD(OSD_Blk2);
        osdDrawPreviewIcon();
    }
    return 0;
}

/*

Routine Description:

	Stop Capture Video

Arguments:

	None.

Return Value:

	0 - Not need to stop.
	1 - Success.

*/
u8 uiCaptureVideoStop(void)
{
    u8 err;
    u32   waitFlag;

    #if (MULTI_CHANNEL_VIDEO_REC && MULTI_CHANNEL_RF_RX_VIDEO_REC)
	    RfRxVideoPackerDisable();
    #endif

    waitFlag = OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_SET_REC|FLAGSYS_RDYSTAT_REC_START), OS_FLAG_WAIT_CLR_ANY, &err);
    if(waitFlag > 0)
    {
        if(waitFlag & FLAGSYS_RDYSTAT_SET_REC)
        {
            DEBUG_UI("Capture do not Start and now Stop %d\r\n",waitFlag);
            sysCaptureVideoStart = 1;
        }
        else
        {
            if(sysCaptureVideoStart == 0)
            {
                DEBUG_UI("Capture Start and now Stop %d\r\n",waitFlag);
                sysCaptureVideoStart = 1;
            }
            else
            {
                DEBUG_UI("UI Wait Stop Capture.....\r\n");
        #if MULTI_CHANNEL_VIDEO_REC
                MultiChannelAsfCaptureVideoStopAll();
        #else
            #if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_ASF)
                while(asfCaptureVideoStop() == 0)
            #elif(MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_AVI)
                while(aviCaptureVideoStop() == 0)
            #elif(MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MOV)
                while(movCaptureVideoStop() == 0)
            #endif
        #endif
                {
                    OSTimeDly(2);
                    DEBUG_UI("Wait\n");
                }
                OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_REC_START, OS_FLAG_WAIT_SET_ANY, OS_IPC_WAIT_FOREVER, &err);
            }
        }
        OSTimeDly(1);
        DEBUG_UI("UI stop capture video Success\r\n");
        iconflag[UI_MENU_SETIDX_IS_REC]= 0;
        Save_UI_Setting();
        return 1;
    }
    else
    {
        DEBUG_UI("Not yet begun capturing\n");
        return 0;
    }
}

#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
u8 uiTXStaustoRX(u8 data,u8 staus)
{
    if (staus)
        uiTXstaus |= data;
    else
        uiTXstaus &= ~data;
//    DEBUG_UI("uiTXstaus =%x $d\n",uiTXstaus,staus);
    rfiu_SetRXLightTrig(uiTXstaus);
    return 1;
}
#endif
u8 uiSetGoToFormat(void)
{
#if IS_COMMAX_DOORPHONE
    void DeleteAllFiles(void);  // ref. MenuFunc.c

    DeleteAllFiles();   // format internal
    OSMboxPost(speciall_MboxEvt, "NO");
#elif (HW_BOARD_OPTION == MR6730_AFN)
    extern void FormatSDCard(void);

    DEBUG_UI("<<<<<--------->>>>>SD need Format\n");

    #if 0
        FormatSDCard();		//format SD card
        OSMboxPost(speciall_MboxEvt, "NO");
    #else
        UI_gotoSDFormatMode();
    #endif

#else
    MyHandler.MenuMode  = GOTO_FORMAT_MODE;
    siuOpMode           = SIUMODE_START;
    sysCameraMode       = SYS_CAMERA_MODE_PLAYBACK;
    //uiMenuOSDReset();
    playbackflag        = 0;
    uiMenuEnable        = 0;
    iduPlaybackMode(640,480,640);
    iduOSDDisable_All();
    uiEnterMenu(UI_MENU_NODE_FORMAT_NO);
    uiGraphDrawMenu();
#endif
    return 1;
}
void uiDrawGetGSensorData(u8 *data)
{
#if (G_SENSOR == G_SENSOR_LIS302DL)
    s8              out_X, out_Y, out_Z;
    s16             Temp;
    u8              X, Y, Z, signX, signY, signZ;
    const   u8      Floating[16][3] = {"00", "06", "13", "19", "25", "31", "38", "44", "50", "56", "63", "69", "75", "81", "88", "94"};
    //const   u8      Floating[16][3] = {"0", "1", "1", "2", "3", "3", "4", "4", "5", "6", "6", "7", "8", "8", "9", "9"};
    i2cGet_LIS302DL_XYZ(&out_X, &out_Y, &out_Z);
    out_X   = (out_X > 127) ? 127 : (out_X < -128) ? -128 : out_X;
    out_Y   = (out_Y > 127) ? 127 : (out_Y < -128) ? -128 : out_Y;
    out_Z   = (out_Z > 127) ? 127 : (out_Z < -128) ? -128 : out_Z;
    X       = abs(out_X);
    Y       = abs(out_Y);
    Z       = abs(out_Z);
    signX   = (out_X >= 0) ? '+' : '-';
    signY   = (out_Y >= 0) ? '+' : '-';
    signZ   = (out_Z >= 0) ? '+' : '-';
  #if G_SENSOR_DETECT
    if((X >= GLimitX) || (Y >= GLimitY) || (Z >= GLimitZ))
    {
        DEBUG_I2C("G Sensor Value(%d, %d, %d) >= G Threshold(%d, %d, %d)\n", out_X, out_Y, out_Z, GLimitX, GLimitY, GLimitZ);
        GSensorEvent    = 1;
    }
  #endif  // #if G_SENSOR_DETECT
    sprintf ((char *)data, "(%c%d.%sG,%c%d.%sG,%c%d.%sG)",
            signX, X >> 4, Floating[X & 0xf], signY, Y >> 4,
            Floating[Y & 0xf], signZ, Z >> 4, Floating[Z & 0xf]);

#elif (G_SENSOR == G_SENSOR_BMA150)
    static  s16     out_X, out_Y, out_Z, out_T;
    u16             X, Y, Z, signX, signY, signZ;

    i2cGet_BMA150_TXYZ(&out_T, &out_X, &out_Y, &out_Z);
    X       = abs(out_X);
    Y       = abs(out_Y);
    Z       = abs(out_Z);
    signX   = (out_X >= 0) ? '+' : '-';
    signY   = (out_Y >= 0) ? '+' : '-';
    signZ   = (out_Z >= 0) ? '+' : '-';
  #if G_SENSOR_DETECT
    if((X >= GLimitX) || (Y >= GLimitY) || (Z >= GLimitZ))
    {
        DEBUG_I2C("G Sensor Value(%d, %d, %d) >= G Threshold(%d, %d, %d)\n", out_X, out_Y, out_Z, GLimitX, GLimitY, GLimitZ);
        GSensorEvent    = 1;
    }
  #endif  // #if G_SENSOR_DETECT
    sprintf ((char *)data, "(%c%d.%02dG,%c%d.%02dG,%c%d.%02dG)",
            signX, X >> 7, ((X & 0x7f) * 100) >> 7, signY, Y >> 7,
            ((Y & 0x7f) * 100) >> 7, signZ, Z >> 7, ((Z & 0x7f) * 100) >> 7);
#endif  // #if (G_SENSOR == G_SENSOR_LIS302DL)

}

void osdDrawGSensorData(void)
{
    if (OSDTimestameLevel == UI_MENU_VIDEO_OVERWRITESTR_ALL)
    {
        uiDrawGetGSensorData(GS);
        //uiMenuOSDString(OSDDispWidth[sysTVOutOnFlag] , GS , 8 , 16 , 12 , 56 , OSD_Blk2 , 2);
        //uiMenuOSDStringByColor(TVOSD_SizeX , GS , 8 , 16 , 12 , 36 , OSD_Blk2, 0xC0, 0x41);
    }
}

#if(HW_BOARD_OPTION == MR6730_AFN)	

#define UI_RETRY_REC_TIME       10
#define UI_RETRY_REC_NONE       (UI_RETRY_REC_TIME+1)


u8  uiDrawSDFail = UI_OSD_CLEAR;
u8  uiRetryRecTime = UI_RETRY_REC_NONE;

void uiFlowCheckRetryRec(void)
{
    if (uiDrawSDFail == UI_OSD_DRAW)
    {
        if (uiRetryRecTime == UI_RETRY_REC_NONE)
            uiRetryRecTime = UI_RETRY_REC_TIME;
        else
            uiRetryRecTime --;
        if (uiRetryRecTime == 0)
        {
            uiOsdDrawSDCardFail(UI_OSD_CLEAR);
            //uiFlowCheckRecState();
            uiCaptureVideo();
            uiRetryRecTime = UI_RETRY_REC_NONE;
        }
    }
}
#endif 

#if  (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)

/*
    Return 0: A > B
    Return 1: A = B
    Return 2: A < B
*/
u8  uiCmpLightTimer(u8  AHour, u8 AMin, u8 BHour, u8 BMin)
{
    if (AHour > BHour)
        return 0;
    else if (AHour == BHour)
    {
        if (AMin > BMin)
            return 0;
        else if (AMin == BMin)
            return 1;
        else
            return 2;
    }
    else
        return 2;
}


void uiCheckLightStatus(void)
{
    u8  thisWeek, inRange = 0;
    u8  i, nToD, change;
    RTC_DATE_TIME   localTime;

    RTC_Get_Time(&localTime);
    thisWeek = localTime.week;

    if (uiInManualLight != 0)
        return;
    if ((uiLightWeek1[thisWeek] == 0) && (uiLightWeek2[thisWeek] == 0))
    {
        if (uiInScheduleLight == 1)
        {
            DEBUG_UI("Light Schedule Cancel, Schedule interrupt, Light Off\r\n");
            uiInScheduleLight = 0;
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 0);
            rfiu_SetRXLightTrig(0);
            uiSetTriggerDimmer = 0;
        }
        return;
    }
    if (uiLightWeek1[thisWeek] == 1)
    {
        if (uiCmpLightTimer(uiLightTimer1[0], uiLightTimer1[1], localTime.hour, localTime.min) >= 1)
        {
            if (uiCmpLightTimer(uiLightTimer1[2], uiLightTimer1[3], localTime.hour, localTime.min) < 1)
                inRange = 1;
        }
    }
    if (uiLightWeek2[thisWeek] == 1)
    {
        if (uiCmpLightTimer(uiLightTimer2[0], uiLightTimer2[1], localTime.hour, localTime.min) >= 1)
        {
            if (uiCmpLightTimer(uiLightTimer2[2], uiLightTimer2[3], localTime.hour, localTime.min) < 1)
                inRange = 1;
        }
    }
    if (uiInScheduleLight == 1)
    {
        if (inRange == 0)
        {
            DEBUG_UI("Light Schedule Time Off\r\n");
            uiInScheduleLight = 0;
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 0);
            uiSetTriggerDimmer = 0;
            rfiu_SetRXLightTrig(0);
        }
    }
    else
    {
        if (inRange == 0)
            uiInScheduleLight = 0;
        else
        {
            if (uiInScheduleLight == 0)
            {
                DEBUG_UI("Light Schedule Time ON\r\n");
                uiInScheduleLight = 1;
                uiMenuAction(UI_MENU_SETIDX_LIGHT_DIMMER);
                gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 1);
                uiSetTriggerDimmer = UI_TRIGGER_DIMMER_TIME;
                rfiu_SetRXLightTrig(1);
            }
        }
    }
}

#endif

extern s32 MD_Diff;
extern u8  MD_trigger;     /* trigger 1:on 0:off */
extern u8  MD_level;       /* level high : 10, medium : 20, low : 30 */
extern u8  MD_status;      /* 是否啟動, 1:ON 0:OFF */
void uiFlowRunPerSec(void)
{
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
    static u8 syncTxTime = UI_TX_SYNC_TIME;
    u8  level;
#endif
    uiFlowCheckP2PMode();

#if((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    if(uiCheckNightTime>2)
    {
        rfiu_SetRXLightTrig(uiNightVision);
        uiCheckNightTime=0;
    }
    else
        uiCheckNightTime++;
    if (uiTestTimeoutCnt > 0)
    {
        uiTestTimeoutCnt--;
        if (uiTestTimeoutCnt == 0)
            sysForceWDTtoReboot();
    }
#endif
#if(HW_BOARD_OPTION == MR6730_AFN)

 	#if 1	

	#if(USE_PLAYBACK_AUTONEXT)
	//without Alarm-Out
	#else

    //for alarm control
    //clear gap after stop record
    if( setUI.MD_AlarmGap == 1){
        if(  UI_isVideoRecording() == false ){
	     setUI.MD_Alarming = 0;
    	     setUI.MD_AlarmGap = 0;
        }
    }else{
	 //count alarm time	
        if(setUI.MD_Alarming == 1 ){
            if( setUI.MD_AlarmTime == 0){

			#if (HW_BOARD_OPTION == MR6730_AFN) 
				#if(USE_MD_ALARMOUT)
				AlarmOut_OnOff(ALM_OUT_OFF);
				#endif
			#else					
				gpioSetLevel(1,19,0);
			#endif 

    	     setUI.MD_Alarming = 0;
    	     setUI.MD_AlarmGap = 1;
            }else{
                setUI.MD_AlarmTime --;
            }
        }else{

        //DEBUG_UI("================Alarm[%d]=============", setUI.SYS_AlarmTime);
           //first time to alarm after start record
        if( (setUI.RecMode == RECMODE_MOTION) && (setUI.SYS_AlarmTime != 0xFF) ){
                if( UI_isVideoRecording()  ){
       	      setUI.MD_AlarmTime = setUI.SYS_AlarmTime;
       	      setUI.MD_Alarming = 1;
		      setUI.MD_AlarmGap = 0;

			#if (HW_BOARD_OPTION == MR6730_AFN) 
				#if(USE_MD_ALARMOUT)
				AlarmOut_OnOff(ALM_OUT_ON);
				#endif			
			#else					
				gpioSetLevel(1,19,1);
			#endif 

		      setUI.ClearREC = 0;
	            }
                }
            }
        }
	#endif

    if ( !UI_isPreviewMode() ){
        return;
    }

    /*show rec icon*/
    if(  UI_isVideoRecording() ){
        uiFlowCheckRetryRec();
        uiOsdDrawRecPerSec();
	 setUI.ClearREC = 0;
	 //DEBUG_UI("--------------------flowpersec : uiOsdDrawRecPerSec\n");
    }
	#if 1
	else
	{
	
		if(setUI.recOn==UI_OSD_DRAW)
		{
			setUI.recOn = UI_OSD_CLEAR;
			osdDrawVideoOn(setUI.recOn);//clear REC icon
		}
	
	}
	#endif 

		

	#endif //#if 0


#elif IS_COMMAX_DOORPHONE
//#if 1
    switch(PhotoFramenum)
    {
    DEBUG_UI("PhotoFramenum=%d \n\n",PhotoFramenum);
        case 0:
            uiCaptureVideoStop();
            playbackflag        = 0;
            uiMenuEnable        = 0;
            siuOpMode           = SIUMODE_START;
            sysCameraMode       = SYS_CAMERA_MODE_PLAYBACK;
            iduPlaybackMode(800,480,800);
            MyHandler.MenuMode  = PLAYBACK_MENU_MODE;
            IduVideo_ClearPKBuf(0);
            uiMenuOSDReset();
            #if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
            dcfPlaybackCurFile = dcfListAlbFileEntHead;
            #else
            dcfPlaybackCurFile = dcfListFileEntHead;
            #endif
            PhotoFramenum=1;
            uiReadVideoFile();
            iduSetVideoBuf0Addr(PKBuf0);
            break;

        case 1:
            counttime++;
            counttimetotal++;
            #if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
            if(counttimetotal==(global_total_Alb_file_count*3))
            #else
            if(counttimetotal==(global_totalfile_count*3))
            #endif
            {
                PhotoFramenum=2;
                break;
            }
            if(counttime==3)
            {
                counttime=0;
                dcfPlaybackFileNext();
                uiReadVideoFile();
                iduSetVideoBuf0Addr(PKBuf0);
            }
            break;

        default:
            break;
    }
    if (MD_status == 1)
    {
        if(MD_trigger == 1)
        {
            DEBUG_UI(">.< MD_trigger %d\n",MD_trigger);
            OSSemPost(uiSemEvt);
        }
    }
#elif(HW_BOARD_OPTION==MR6730_FINEDIGITAL_LCD)
u8 status;
    i2cRead_TVP5150(0xc0,&status,TV_CHECK_FORMAT_RD_ADDR);
    if((status&0x80) != 0x80)
        sysTVInFormatLocked = FALSE;
    //DEBUG_UI("status = %d \n\n",status);
    //DEBUG_UI("sysTVInFormatLocked = %d \n\n",sysTVInFormatLocked);
    //DEBUG_UI("Lock_video = %d \n\n",Lock_video);
    if(sysTVInFormatLocked==TRUE && Lock_video==TRUE)
    {
        uiCaptureVideo();
        Lock_video=FALSE;
        //DEBUG_UI("uiCaptureVideo\n\n");
    }
    else if(sysTVInFormatLocked==FALSE && Lock_video==FALSE)
    {
        uiCaptureVideoStop();
        Lock_video=TRUE;
        //DEBUG_UI("uiCaptureVideoStop\n\n");
    }
#elif (HW_BOARD_OPTION==MR6730_WINEYE)
    if (OSDTimestameLevel == UI_MENU_VIDEO_OVERWRITESTR_ALL)
    {
        #if (G_SENSOR!=G_SENSOR_NONE)
            osdDrawGSensorData();
        #endif
    }
#elif (HW_BOARD_OPTION == MR8120_TX_HECHI)
    powerdown_count++;
    if (powerdown_count == 20)
    {
        //DEBUG_UI("###count = %d\n",count);
        if(gRfiu_Op_Sta[sysRFRxInMainCHsel] != RFIU_TX_STA_LINK_OK)
            sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, sysRFRxInMainCHsel);
        powerdown_count = 0;
    }
    if(gRfiu_Op_Sta[sysRFRxInMainCHsel] == RFIU_TX_STA_LINK_OK)
        powerdown_count = 0;
#else
    if (MyHandler.MenuMode != VIDEO_MODE)
        return;
    /*show life time*/
    //if(uiCheckVideoRec() == 0)   /*only show on perwiew*/
    {
        uiOsdDrawLifeTimePerSec();
    }
    uiOsdDrawRecPerSec();

    #if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
    uiCheckLightStatus();
    if (uiCurrentLightTime != 0)
    {
        uiCurrentLightTime--;
        if ((uiInScheduleLight != 1) && (uiCurrentLightTime == 0))
        {
            DEBUG_UI ("Triger TimeUP\n");
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 0);
            uiSetTriggerDimmer = 0;
            rfiu_SetRXLightTrig(0);
        }
    }
    if (uiInManualLight != 0)
    {
        uiInManualLight--;
        if (uiInManualLight == 0)
        {
            DEBUG_UI ("Mannual TimeUP\n");
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 0);
            uiSetTriggerDimmer = 0;
            rfiu_SetRXLightTrig(0);
        }
    }
    #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CL692) && (UI_PROJ_OPT != 0))
    if (uiSetTriggerDimmer != 0)
    {
        uiSetTriggerDimmer--;
        if (uiSetTriggerDimmer == 0)
            gpioSetLightDimmer(0);
    }
    #endif
    if (syncTxTime != 0)
    {
        syncTxTime--;
        if (syncTxTime == 0)
        {
            syncTxTime = UI_TX_SYNC_TIME;
            gpioGetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, &level);
            if (level == 0)
            {
                rfiu_SetRXLightTrig(0);
            }
            else
            {
                if (uiInManualLight == 0)
                    rfiu_SetRXLightTrig(1);
                else
                    rfiu_SetRXLightTrig(2);
            }
        }
    }
    #endif

#if (NIC_SUPPORT == 1)
    uiOsdDrawIPInfo(VIDEO_MODE);
#endif
#endif
}

s32 uiMenuSetStartMovie(s8 setting)
{
    s32 index;
    u32 sizeUsed;
    u16 uiJpgWidth, uiJpgHeight;
    u8 *pJpgStartAddr;
    sysTVinFormat = getTVinFormat();
    uiSetTVOutXY(TvOutMode);
    InitsysEvt();
    InitsysbackEvt();
    iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
#if ( (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))
    sysSD_Disable();
    sysSPI_Enable();
    sysJPEG_enable();
    sysISU_enable();
    index = spiGet_UI_FB_Index("logo.jpg");
    if (index < 0)
    {
        DEBUG_UI("uiMenuSetStartMovie Get UI_FB Index Fail\r\n");
        return 0;
    }

    if(uiGraphDrawJpgGraph(index, PKBuf1, &uiJpgWidth, &uiJpgHeight) == 1)
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf1, gJPGValidWidth, gJPGValidHeight, 0,0, uiJpgWidth,PKBuf0);

    else
        DEBUG_UI("File %s Open Fail\r\n","logo.jpg");

    IduVidBuf0Addr = (u32)PKBuf0;

    #if NEW_IDU_BRI
        BRI_IADDR_Y = IduVidBuf0Addr;
        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
    #endif
    OSTimeDly(75);



    iduTVOSDDisplay(2, 0, 0, TVOSD_SizeX, TVOSD_SizeY);
    uiClearOSDBuf(2);
    //uiMenuOSDStringByColor(TVOSD_SizeX , VERNUM , 8 , 16 , 200 , 216+osdYShift, 2 , 0xC1, 0xC0);
    uiOsdEnable(2);
    IduVideo_ClearPKBuf(0);
    IduVideo_ClearPKBuf(2);



    sysJPEG_disable();
    sysSPI_Disable();
    sysSD_Enable();

#endif
    return 1;
}

s32 uiMenuSetAutoOff(s8 setting)
{
    autoofftick = OS_tickcounter;
    return 1;
}

u8 uiReadSettingFromFlash(u8 *FlashAddr)
{
   int i;

    memcpy(iconflag, FlashAddr, UIACTIONNUM);
    FlashAddr += UIACTIONNUM;
    memcpy(&MotionMaskArea[0][0], FlashAddr, MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN);
    FlashAddr += MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN;
    memcpy(&ScheduledTimeFrom, FlashAddr, sizeof(RTC_DATE_TIME));
    FlashAddr += sizeof(RTC_DATE_TIME);
    memcpy(&ScheduledTimeTo, FlashAddr, sizeof(RTC_DATE_TIME));
    FlashAddr += sizeof(RTC_DATE_TIME);
#if (NIC_SUPPORT)
    memcpy(&UINetInfo, FlashAddr, sizeof(UI_NET_INFO));
    FlashAddr += sizeof(UI_NET_INFO);
    memcpy(UI_P2P_PSW, FlashAddr, sizeof(UI_P2P_PSW));
    FlashAddr += sizeof(UI_P2P_PSW);
#endif
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    memcpy(&TempHighMargin, FlashAddr, sizeof(&TempHighMargin));
    FlashAddr += sizeof(TempHighMargin);
    memcpy(&TempLowMargin, FlashAddr, sizeof(&TempLowMargin));
    FlashAddr += sizeof(TempLowMargin);
#endif
#if(RFIU_SUPPORT)
    for (i = 0; i < 4; i++)
        UI_CFG_RES[i]=iconflag[UI_MENU_SETIDX_VIDEO_SIZE+i];
#endif
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
    memcpy(uiLightTimer, FlashAddr, sizeof(uiLightTimer));
    FlashAddr += sizeof(uiLightTimer);
#endif
#if ICOMMWIFI_SUPPORT
    memcpy(JOIN_DEFAULT_SSID, FlashAddr, sizeof(JOIN_DEFAULT_SSID));
    FlashAddr += sizeof(JOIN_DEFAULT_SSID);
    memcpy(JOIN_DEFAULT_PSK, FlashAddr, sizeof(JOIN_DEFAULT_PSK));
    FlashAddr += sizeof(JOIN_DEFAULT_PSK);
#endif
    return 1;
}

u8 uiSetDefaultSetting(void)
{
    memcpy(iconflag, defaultvalue, UIACTIONNUM);
    memcpy(&MotionMaskArea[0][0], &StartMotMask[0][0], MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN);
    memcpy(&ScheduledTimeFrom, &StartSchTimeFrom, sizeof(RTC_DATE_TIME));
    memcpy(&ScheduledTimeTo, &StartSchTimeTo, sizeof(RTC_DATE_TIME));
#if (NIC_SUPPORT)
    memcpy(&UINetInfo, &StartUINetInfo, sizeof(UI_NET_INFO));
    memcpy(UI_P2P_PSW, UI_Default_P2P_PSW, sizeof(UI_P2P_PSW));
#endif
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    memcpy(&TempHighMargin, &StartTempHighMargin, sizeof(TempHighMargin));
    memcpy(&TempLowMargin, &StartTempLowMargin, sizeof(TempLowMargin));
#endif
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
    memcpy(uiLightTimer, uiStartLightTimer, sizeof(uiLightTimer));
#endif
#if ICOMMWIFI_SUPPORT
    memset(JOIN_DEFAULT_SSID, 0, sizeof(JOIN_DEFAULT_SSID));
    memset(JOIN_DEFAULT_PSK, 0, sizeof(JOIN_DEFAULT_PSK));
#endif
    return 1;
}

u8 uiWriteSettingToFlash(u8 *FlashAddr)
{

    memcpy ((void *)FlashAddr,(void *)iconflag,  UIACTIONNUM);
    FlashAddr += UIACTIONNUM;
    memcpy ((void *)FlashAddr, (void *)&MotionMaskArea[0][0],  MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN);
    FlashAddr += MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN;
    memcpy ((void *)FlashAddr, (void *)&ScheduledTimeFrom, sizeof(RTC_DATE_TIME));
    FlashAddr += sizeof(RTC_DATE_TIME);
    memcpy ((void *)FlashAddr, (void *)&ScheduledTimeTo, sizeof(RTC_DATE_TIME));
    FlashAddr += sizeof(RTC_DATE_TIME);
#if (NIC_SUPPORT)
    memcpy ((void *)FlashAddr, (void *)&UINetInfo, sizeof(UI_NET_INFO));
    FlashAddr += sizeof(UI_NET_INFO);
    memcpy((void *)(FlashAddr), (void *)&UI_P2P_PSW,sizeof(UI_P2P_PSW));
    FlashAddr += sizeof(UI_P2P_PSW);
#endif
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    memcpy ((void *)FlashAddr, (void *)&TempHighMargin, sizeof(TempHighMargin));
    FlashAddr += sizeof(TempHighMargin);
    memcpy ((void *)FlashAddr, (void *)&TempLowMargin, sizeof(TempLowMargin));
    FlashAddr += sizeof(TempLowMargin);
#endif
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
    memcpy ((void *)FlashAddr, (void *)uiLightTimer, sizeof(uiLightTimer));
    FlashAddr += sizeof(uiLightTimer);
#endif
#if ICOMMWIFI_SUPPORT
    memcpy((void *)(FlashAddr), (void *)&JOIN_DEFAULT_SSID,sizeof(JOIN_DEFAULT_SSID));
    FlashAddr += sizeof(JOIN_DEFAULT_SSID);
    memcpy((void *)(FlashAddr), (void *)&JOIN_DEFAULT_PSK,sizeof(JOIN_DEFAULT_PSK));
    FlashAddr += sizeof(JOIN_DEFAULT_PSK);
#endif

    return 1;
}

void uiDrawTimeOnVideoClip(s32 Param)
{
#if ISU_OVERLAY_ENABLE

    RTC_DATE_TIME   curDateTime;
    u8              cF;
    s16             Temp;
    s8              LeftLight, BrakeLight, RightLight;
    s8              PowerSwitch, BatteryDetect;
    u32             Speed;
    u8              err;
    u8              LED_Level;
    u8              chacknum=0;
  #if CDVR_LOG
    s32             StrLen;
  #endif
  #if UART_GPS_COMMAND
    static  u8      szGPS[48];
  #endif
  


    uiDrawGetGSensorData(GS);

    RTC_Get_Time(&curDateTime);
  #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    sprintf (timeForRecord1, "20%02d-%02d-%02d %02d:%02d:%02d",
        curDateTime.year, curDateTime.month, curDateTime.day,
        curDateTime.hour, curDateTime.min, curDateTime.sec);
  #else
	
	#if (HW_BOARD_OPTION == MR6730_AFN)

		#if (CIU_OSD_METHOD_2==0)
			#if(TIME_FORMAT_TYPE==1)
			  sprintf (timeForRecord1, "%02d/%02d/20%02d %02d:%02d:%02d",
				  curDateTime.day, curDateTime.month, curDateTime.year,
				  curDateTime.hour, curDateTime.min, curDateTime.sec);				
			#else
			  sprintf (timeForRecord1, "20%02d/%02d/%02d %02d:%02d:%02d",
				  curDateTime.year, curDateTime.month, curDateTime.day,
				  curDateTime.hour, curDateTime.min, curDateTime.sec);	
			#endif		
		#else
			#if(CIU_BOB_MODE)		
			  //6 spaces+20timestamp <= 32-1('\0')
			  //sprintf (timeForRecord1, "      20%02d/%02d/%02d %02d:%02d:%02d",
	  
			  //bob mode test 
			  if(g_ShowBobOnOSD)
			  {
			  sprintf (timeForRecord1, "%01x %03d 20%02d/%02d/%02d %02d:%02d:%02d",
				  g_CIU_BobMode,MD_Diff,
				  curDateTime.year, curDateTime.month, curDateTime.day,
				  curDateTime.hour, curDateTime.min, curDateTime.sec);		  
			  }
			  else
			  {
			  sprintf (timeForRecord1, "      20%02d/%02d/%02d %02d:%02d:%02d",
				  curDateTime.year, curDateTime.month, curDateTime.day,
				  curDateTime.hour, curDateTime.min, curDateTime.sec);		  
			  }
			#else
	  
			  sprintf (timeForRecord1, "20%02d/%02d/%02d %02d:%02d:%02d",
				  curDateTime.year, curDateTime.month, curDateTime.day,
				  curDateTime.hour, curDateTime.min, curDateTime.sec);
	  
			#endif
		#endif
	#else
  	sprintf (timeForRecord1, "20%02d/%02d/%02d %02d:%02d:%02d",
	  curDateTime.year, curDateTime.month, curDateTime.day,
	  curDateTime.hour, curDateTime.min, curDateTime.sec);
	#endif

  #endif
    if (strcmp(timeForRecord1, timeForRecord2))
    {
        strcpy(timeForRecord2, timeForRecord1);

    #if UART_GPS_COMMAND
        if (gGPS_data1.valid == 1)
        {
            //copy data to
            //DEBUG_SYS("T");
            OSSemPend(GPSUpdateEvt, 5, &err);
            memcpy(&gGPS_data2, &gGPS_data1, sizeof(GPS_DATA));
            gGPS_data1.valid    = 0;
            OSSemPost(GPSUpdateEvt);
        }

        if  (gGPS_data2.valid)
        {
            //DEBUG_SYS("O");
            gGPS_data2.valid    = 0;
            sprintf(szGPS, "%c%02d %02d.%03d %c%03d %02d.%03d",
                    gGPS_data2.N_S, (gGPS_data2.Lat_I /100) & 0xff, (gGPS_data2.Lat_I %100) & 0xff, gGPS_data2.Lat_F/10,
                    gGPS_data2.E_W, (gGPS_data2.Lon_I/100) & 0xff, (gGPS_data2.Lon_I %100) & 0xff, gGPS_data2.Lon_F/10);
            DEBUG_SYS("%s\n", szGPS);
        }
        else if(strlen(szGPS) == 0)
        {
            sprintf(szGPS,"%c%02d %02d.%03d %c%03d %02d.%03d",
                    'N', 0, 0, 0,
                    'E', 0, 0, 0);
            DEBUG_SYS("%s\n", szGPS);
        }
    #endif
    }
#if (G_SENSOR!=G_SENSOR_NONE)
    if (1)    // 全部顯示
    {
    #if(ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_8x16x95_2BIT)
      //#if UART_GPS_COMMAND
      #if 0
        sprintf (szVideoOverlay1, "%s %s %s %s %s                %s", VERNUM, timeForRecord1, GS, ((OSTime >> 3) & 1) ? "Rec" : "   ", BuzzerToggle ? "BZ ON " : "BZ OFF", szGPS);
      #else
        sprintf (szVideoOverlay1, "%s %d %s %s %s %s", VERNUM, chacknum, timeForRecord1, GS, ((OSTime >> 3) & 1) ? "Rec" : "   ", BuzzerToggle ? "BZ ON " : "BZ OFF");
      #endif
    #else
        sprintf (szVideoOverlay1, "%s             %s        ", timeForRecord1, GS);
    #endif
    }
    else    // 只秀時間
    {
    #if(ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_8x16x95_2BIT)
      //#if UART_GPS_COMMAND
      #if 0
        sprintf (szVideoOverlay1, "%s %s                        %s %s                %s", VERNUM, timeForRecord1, ((OSTime >> 3) & 1) ? "Rec" : "   ", BuzzerToggle ? "BZ ON " : "BZ OFF", szGPS);
      #else
        sprintf (szVideoOverlay1, "%s %s                        %s %s", VERNUM, timeForRecord1, ((OSTime >> 3) & 1) ? "Rec" : "   ", BuzzerToggle ? "BZ ON " : "BZ OFF");
      #endif
    #else
        sprintf (szVideoOverlay1, "%s                      ", timeForRecord1);
    #endif
    }
#else
    sprintf (szVideoOverlay1, "%s", timeForRecord1);
#endif
#if CDVR_LOG
    OSSemPend(LogFileSemEvt, 10, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SYS("uiDrawTimeOnVideoClip() Error: LogFileSemEvt is %d.\n", err);
        return;
    }
    sprintf(szLogString,"%s                %s", szVideoOverlay1, szGPS);
    strcpy(szLogFile, szLogString);
    StrLen      = strlen(szLogString);
    strcpy(szLogFile + StrLen, "\r\n");
    szLogFile  += StrLen + 2;
    if(szLogFile >= (LogFileBufEnd - MAX_OVERLAYSTR * 2))
    {
        pLogFileMid                     = szLogFile;
        szLogFile                       = LogFileBuf;
    }
    OSSemPost(LogFileSemEvt);
#endif
    if(OSDTimestameLevel != UI_MENU_VIDEO_OVERWRITESTR_NONE)
    {
        if (strcmp(szVideoOverlay1, szVideoOverlay2) && Param)
        {
            strcpy(szVideoOverlay2, szVideoOverlay1);
        #if ISU_OVERLAY_ENABLE
        #if 0
            GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
                            96,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                            1,12,1+24,11+4);
        //ASCII_FONT_32x40x95_2BIT
           GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
            48,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
            1,1,1+16,0+4);
        #else
          #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
            if(OverwriteStringEnable)
            {
                DEBUG_UI("szVideoOverlay1 = %s\n", szVideoOverlay1);
                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                    10,20,10+24,19+2);
            } else {
                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, "                    ",
                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                    10,20,10+24,19+2);
            }
          #elif (HW_BOARD_OPTION == MR6730_AFN)

			if(OverwriteStringEnable)
            {      
            #if PRINT_TS_MSG
				#if (USE_UI_APP_TASK)
				DMSG_OUT("szVideoOverlay1 = %s\n", szVideoOverlay1);
				#else
				DEBUG_UI("szVideoOverlay1 = %s\n", szVideoOverlay1);
				#endif //#if (USE_UI_APP_TASK)	
			#endif

				
				#if (CIU_OSD_METHOD_2==0)
					#if(DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)
					{
						//char TxtBuff[32]="";		
						char TxtBuff[20+1]="";//19+1	
						int fnt_x1=0,fnt_y1=0,fnt_x2=0,fnt_y2=0;
		
						{
							u8 IsOsdClr=1;
							if(uiMenuVideoSizeSetting==UI_MENU_VIDEO_SIZE_1280X720)
							{
							#if (IDUOSD_TIMESTAMP)
							fnt_x1=6;
							fnt_y1=30;
							fnt_x2=6+24;
							fnt_y2=30+1;
							#else
							
							#if(ISU_OVERLAY_XLARGE_FONT_TYPE==ASCII_FONT_32x28x95_2BIT)	
							fnt_x1=3;
							fnt_y1=23;
							fnt_x2=3+24;
							fnt_y2=23+1;								
							#elif(ISU_OVERLAY_XLARGE_FONT_TYPE==ASCII_FONT_32x40x95_2BIT)
							fnt_x1=3;
							fnt_y1=16;
							fnt_x2=3+24;
							fnt_y2=16+1;
							#elif(ISU_OVERLAY_XLARGE_FONT_TYPE==ASCII_FONT_32x36x95_RDI_2BIT)
							fnt_x1=3;
							fnt_y1=18;
							fnt_x2=3+24;
							fnt_y2=18+1;
							#else
							fnt_x1=3;
							fnt_y1=10;
							fnt_x2=3+24;
							fnt_y2=10+1;							
							#endif							
							#endif
							}
							else
							{//UI_MENU_VIDEO_SIZE_640x480
							#if(ISU_OVERLAY_LARGE_FONT_TYPE==ASCII_FONT_16x20x95_2BIT)
							fnt_x1=3;
							fnt_y1=21;
							fnt_x2=3+24;
							fnt_y2=21+1;								
							#elif(ISU_OVERLAY_LARGE_FONT_TYPE==ASCII_FONT_16x22x95_COMMAX_2BIT)							
							fnt_x1=3;
							fnt_y1=20;
							fnt_x2=3+24;
							fnt_y2=21;														
							#elif(ISU_OVERLAY_LARGE_FONT_TYPE==ASCII_FONT_32x36x95_RDI_2BIT || ISU_OVERLAY_LARGE_FONT_TYPE==ASCII_FONT_32x28x95_2BIT)							
							fnt_x1=1;
							fnt_y1=10;
							fnt_x2=1+32;
							fnt_y2=10+1;							
							#else
							fnt_x1=3;
							fnt_y1=10;
							fnt_x2=3+32;// (FontW/4*char_W)  should be multiples of 32
							fnt_y2=10+1;
							#endif
							}
						
							if(MACRO_UI_SET_SYSSTAT_BIT_CHK(UI_SET_SYSSTAT_BIT_SD_CHG)||(!MACRO_UI_SET_SYSSTAT_BIT_CHK(UI_SET_SYSSTAT_BIT_SD_RDY)))
							{//reject anykey when SD card is removed or inserted.
								IsOsdClr=0;								
							//DEBUG_UI("clear-1\n");
							}
							
							#if (IDUOSD_TIMESTAMP)
							if((IsOsdClr)&&(gPreviewInitDone==1))
							{							
							GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
								288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
								fnt_x1,fnt_y1,fnt_x2,fnt_y2); 
							}
							else
							{							
							//sprintf(TxtBuff,"%s","                    ");
							_SPACEs(TxtBuff,20)
							GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, TxtBuff,
								288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
								fnt_x1,fnt_y1,fnt_x2,fnt_y2);									
							//DEBUG_UI("clear-2\n");
							}
							
							#else
							{
								int FONT_W=ASCII_LARGE_FONT_WIDTH,FONT_H=ASCII_LARGE_FONT_HEIGHT;
								if(uiMenuVideoSizeSetting==UI_MENU_VIDEO_SIZE_1280X720)
								{
									FONT_W=ASCII_XLARGE_FONT_WIDTH,FONT_H=ASCII_XLARGE_FONT_HEIGHT;
								}
								else
								{
									FONT_W=ASCII_LARGE_FONT_WIDTH,FONT_H=ASCII_LARGE_FONT_HEIGHT;
								}
									
								//if((IsOsdClr)&&(gPreviewInitDone==1))
								if((gPreviewInitDone==1))
								{		
									GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
										32,FONT_W, FONT_H,
										fnt_x1,fnt_y1,fnt_x2,fnt_y2); 

								}
								else
								{

									_SPACEs(TxtBuff,20)
									GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, TxtBuff,
										32,FONT_W, FONT_H,
										fnt_x1,fnt_y1,fnt_x2,fnt_y2);

								}
							}
							#endif
						}
					}




					#else
	                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
	                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
	                    10,20,10+24,19+2); 
					#if(!MULTI_CH_DEGRADE_1CH)
					GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot, szVideoOverlay1,
	                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
	                    10,20,10+24,19+2);
					#endif
					#endif
				
                #else

					#if 1	
					//
					//This special treatment is really wierd,but it works.
					//
					if(gPreviewInitDone)
					{
						
						if(gCOSD2bPosAutoChange)
						{
						#if 1					
							if(COSD2b_X2!=COSD2B_X2_ADJ)//if(COSD2b_Y1!=COSD2B_Y1_ADJ)
							{								
								UI_Adjust_COSD2bPos(1);//change position
								//OSTimeDlyHMSM(0,0,0,500);
								DEBUG_UI("==((@@))==COSD2b_Xx Changed\n");
							}
						#endif	
						}//if(gCOSD2bPosAutoChange)

		                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
		                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
		                    COSD2b_X1,COSD2b_Y1,COSD2b_X2,COSD2b_Y2);
						#if(!MULTI_CH_DEGRADE_1CH)
						//OSTimeDlyHMSM(0,0,0,500);
						GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot, szVideoOverlay1,
							288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
							COSD2b_X1,COSD2b_Y1,COSD2b_X2,COSD2b_Y2);	
						#endif						
				  	}
				  	else
				 	{
						GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, "                    ",
		                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
		                    COSD2b_X1,COSD2b_Y1,COSD2b_X2,COSD2b_Y2);   
						#if(!MULTI_CH_DEGRADE_1CH)
						//OSTimeDlyHMSM(0,0,0,500);
						//DEBUG_UI("==((@@))==COSD2b_CH2\n");
						GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot, "                    ",
							288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
							COSD2b_X1,COSD2b_Y1,COSD2b_X2,COSD2b_Y2);	
						#endif
				 	}
					#else
					GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
						288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
						COSD2b_X1,COSD2b_Y1,COSD2b_X2,COSD2b_Y2);			
					#if(!MULTI_CH_DEGRADE_1CH)
					GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot, szVideoOverlay1,
						288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
						COSD2b_X1,COSD2b_Y1,COSD2b_X2,COSD2b_Y2);	
					#endif
					#endif
				


				#endif
 		
            } else {

				
				#if (CIU_OSD_METHOD_2==0)
					#if(DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)
					{
						//char TxtBuff[32]={0,};	
						char TxtBuff[20+1]="";//19+1
						int fnt_x1=0,fnt_y1=0,fnt_x2=0,fnt_y2=0;
						if(gPreviewInitDone)
						{
							if(uiMenuVideoSizeSetting==UI_MENU_VIDEO_SIZE_1280X720)
							{
							#if (IDUOSD_TIMESTAMP)
							fnt_x1=6;
							fnt_y1=30;
							fnt_x2=6+24;
							fnt_y2=30+1;
							#else
							
							#if(ISU_OVERLAY_XLARGE_FONT_TYPE==ASCII_FONT_32x28x95_2BIT)						
							fnt_x1=3;
							fnt_y1=23;
							fnt_x2=3+24;
							fnt_y2=23+1;								
							#elif(ISU_OVERLAY_XLARGE_FONT_TYPE==ASCII_FONT_32x40x95_2BIT)
							fnt_x1=3;
							fnt_y1=16;
							fnt_x2=3+24;
							fnt_y2=16+1;
							#elif(ISU_OVERLAY_XLARGE_FONT_TYPE==ASCII_FONT_32x36x95_RDI_2BIT)
							fnt_x1=3;
							fnt_y1=18;
							fnt_x2=3+24;
							fnt_y2=18+1;
							#else
							fnt_x1=3;
							fnt_y1=10;
							fnt_x2=3+24;
							fnt_y2=10+1;							
							#endif
							#endif
							}
							else
							{//UI_MENU_VIDEO_SIZE_640x480
							#if(ISU_OVERLAY_LARGE_FONT_TYPE==ASCII_FONT_16x20x95_2BIT)
							fnt_x1=3;
							fnt_y1=21;
							fnt_x2=3+24;
							fnt_y2=21+1;							
							#elif(ISU_OVERLAY_LARGE_FONT_TYPE==ASCII_FONT_16x22x95_COMMAX_2BIT)							
							fnt_x1=3;
							fnt_y1=20;
							fnt_x2=3+24;
							fnt_y2=21;	
							#elif(ISU_OVERLAY_LARGE_FONT_TYPE==ASCII_FONT_32x36x95_RDI_2BIT || ISU_OVERLAY_LARGE_FONT_TYPE==ASCII_FONT_32x28x95_2BIT)							
							fnt_x1=1;
							fnt_y1=10;
							fnt_x2=1+32;
							fnt_y2=10+1;								
							#else
							fnt_x1=3;
							fnt_y1=10;
							fnt_x2=3+32;//24;// (FontW/4*char_W)  should be multiples of 32
							fnt_y2=10+1;							
							#endif
							}

						}
						else
						{
							fnt_x1=2;
							fnt_y1=20;
							fnt_x2=2+24;
							fnt_y2=21;							
						}
						#if (IDUOSD_TIMESTAMP)
							//sprintf(TxtBuff,"%s","                    ");//20 spaces
							_SPACEs(TxtBuff,20)
							GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, TxtBuff,
			                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
			                    fnt_x1,fnt_y1,fnt_x2,fnt_y2); 
						
						#else
						{
							int FONT_W=ASCII_LARGE_FONT_WIDTH,FONT_H=ASCII_LARGE_FONT_HEIGHT;
							if(uiMenuVideoSizeSetting==UI_MENU_VIDEO_SIZE_1280X720)
							{
								FONT_W=ASCII_XLARGE_FONT_WIDTH,FONT_H=ASCII_XLARGE_FONT_HEIGHT;
							}
							else
							{
								FONT_W=ASCII_LARGE_FONT_WIDTH,FONT_H=ASCII_LARGE_FONT_HEIGHT;
							}
							
							_SPACEs(TxtBuff,20)
							GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, TxtBuff,
			                    32,FONT_W, FONT_H,
			                    fnt_x1,fnt_y1,fnt_x2,fnt_y2); 

						}
						#endif
					}
					#else



					
					GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, "                    ",
	                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
	                    10,20,10+24,19+2);
					#if(!MULTI_CH_DEGRADE_1CH)
					//OSTimeDlyHMSM(0,0,0,500);
					//DEBUG_UI("==((@@))==COSD2b_CH2\n");
					GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot, "                    ",
	                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
	                    10,20,10+24,19+2);
					#endif
					#endif


					
                #else
  		
					#if 1	
					//
					//This special treatment is really wierd,but it works.
					//
					if(gPreviewInitDone)
					{
						if(gCOSD2bPosAutoChange)
						{
						#if 1					
							if(COSD2b_X2==COSD2B_X2_ADJ)
							{
								UI_Adjust_COSD2bPos(0);//restored
								//OSTimeDlyHMSM(0,0,0,500);
								DEBUG_UI("==((@@))==COSD2b_Xx restored\n");
							}
						#endif		
						}//if(gCOSD2bPosAutoChange)
					}
					#endif
					

					GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, "                    ",
	                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
	                    COSD2b_X1,COSD2b_Y1,COSD2b_X2,COSD2b_Y2);					
					#if(!MULTI_CH_DEGRADE_1CH)
					//OSTimeDlyHMSM(0,0,0,500);
					//DEBUG_UI("==((@@))==COSD2b_CH2\n");
					GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot, "                    ",
	                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
	                    COSD2b_X1,COSD2b_Y1,COSD2b_X2,COSD2b_Y2);
					#endif
					
				#endif

				
            }

				
          #else
            GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
                288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                20,40,20+96,39+4);
          #endif
        //ASCII_FONT_8x16x95_2BIT
        #endif
        //    GenerateOverlayImage(ScalarOverlayImage, szVideoOverlay1, MAX_OVERLAYSTR, ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT, 640);
        #endif
        }
    }

#endif
}


#if (SUPPORT_TOUCH == 1)
/*

Routine Description:

	Check Touch Key and send UI key

Arguments:

	None.

Return Value:

	0 - Not in touch range.
	1 - Success.
	2 - UI busy

*/


u8  uiFlowCheckTouchKey(int TouchX, int TouchY)
{
    UITOUCH_NODE_EVENT_TBL  *nodeTouchInfo;
    u8  i;
    u32 GetKey = UI_KEY_READY;

    if (UIKey != UI_KEY_READY)
    {
        DEBUG_UI("uiFlowCheckTouchKey busy \n");
        return 2;
    }

    nodeTouchInfo = uiCurrNode->item.NodeData->TouchData;
    if (nodeTouchInfo == NULL)
    {
        DEBUG_UI("Current Node do not have touch range \n");
        return 0;
    }
    for (i = 0; i < nodeTouchInfo->uiCheckEventNum; i++)
    {
        if ((TouchX > nodeTouchInfo->pTouchNodeEvent[i].uiLeftUp_X) &&
            (TouchX < nodeTouchInfo->pTouchNodeEvent[i].uiRightDown_X)&&
            (TouchY > nodeTouchInfo->pTouchNodeEvent[i].uiLeftUp_Y)&&
            (TouchY < nodeTouchInfo->pTouchNodeEvent[i].uiRightDown_Y))
        {
            GetKey = nodeTouchInfo->pTouchNodeEvent[i].uiKeyEvent;
            break;
        }
    }
    if (GetKey == UI_KEY_READY)
    {
        DEBUG_UI("Current Node not in touch range \n");
        return 0;
    }
    if (uiSentKeyToUi(GetKey) == 0)
    {
        DEBUG_UI("Get Touch key %d Fail\n",GetKey);
        return 2;
    }
    else
    {
        DEBUG_UI("Get Touch key %d Success\n",GetKey);
        return 1;
    }
}
#endif
void uiFlowCardReady(u8 CardState)
{

}

#if (HW_BOARD_OPTION == MR6730_AFN)	
#if (CIU_OSD_METHOD_2)


   void UI_Adjust_COSD2bPos(u8 State)
   {
   //State:
   // 1: change setting
   // 0: back to default settings
   
	   if(!sysTVOutOnFlag)//Tv-out only
		   return;
   
	   switch(State)
	   {
	   case 0:
		   {
			   //if(COSD2b_Y1==COSD2B_Y1_ADJ)
			   {
			   //10,20,34,21  
			   COSD2b_X1 = COSD2B_X1_PRE;
			   COSD2b_Y1 = COSD2B_Y1_PRE;
			   COSD2b_X2 = COSD2B_X2_PRE;
			   COSD2b_Y2 = COSD2B_Y2_PRE;
			   //DEBUG_UI("==((@@))==COSD2b_Xx restored\n");		   
			   }
			   
		   }
		   break;  
	   case 1:
		   {
			   //if(COSD2b_Y1!=COSD2B_Y1_ADJ)
			   {
			   //10,27,42,28 
			   COSD2b_X1 = COSD2B_X1_ADJ;
			   COSD2b_Y1 = COSD2B_Y1_ADJ;
			   COSD2b_X2 = COSD2B_X2_ADJ;
			   COSD2b_Y2 = COSD2B_Y2_ADJ;
			   //DEBUG_UI("==((@@))==COSD2b_Xx Changed\n"); 		   
			   }		   
		   }
		   break;
   
   
	   }//switch(State)
   
   }//UI_Adjust_COSD2bPos()
   
#endif

#endif //#if (HW_BOARD_OPTION == MR6730_AFN)

