/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	task.h

Abstract:

   	The declaration related to tasks.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/
 
#ifndef __TASK_H__
#define __TASK_H__

/*------------- External task function -------------*/
extern void mainTask(void*);
extern void debugTask(void*);
extern void siuTask(void*); 
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
extern void ciuTask_CH1(void*); 
extern void ciuTask_CH2(void*); 
extern void ciuTask_CH3(void*); 
extern void ciuTask_CH4(void*); 

extern void nicTask(void*);
extern void rfiu_WrapTx_Task_UnitX(void*);
extern void rfiu_WrapRx_Task_UnitX(void*);

extern void rfiu_RxMpeg4DecTask_UnitX(void* pData);

#endif
extern void uiTask(void*);
extern void sysTask(void*); 
extern void sysbackTask(void*);    //Civic 070822
extern void sysback_Low_Task(void *); //Lucian 090404
extern void sysback_RF_Task(void*);    //Lucian 130702
#if (NIC_SUPPORT == 1)
extern void sysback_Net_Task(void* pData);
#endif
extern void mpeg4Task(void*); 
extern void rfiuTXMpeg4EncTask(void*);
extern void mjpgTask(void*);  //Lsk 090312
extern void VideoTask(void*); //Lsk 100901
extern void iisPlaybackTask(void* pData);
extern void iisTask(void*); 
extern void mscTask(void*);
extern void vcTask(void*);
extern void usbTask(void*); 
extern void rtcTask(void*); 	 //Civic 070919
//extern void wdtTask(void*); 	 //Civic 070919
extern void usbPerfTask(void*);		/* Chi-Lun 20080624 */
extern void uartCmdTask(void*);
extern void hiuCmdTask(void*);
extern void SysTimerTask(void*);
extern void SysTimerTickTask(void*);
#if CDVR_LOG //Process log message
extern void LogStringTask(void* pData);
#endif
extern void UIMovieTask(void*);
extern void UIPlayTask(void*);
extern void UIMenuTask(void*);
#if(USB_HOST==1)
extern void usbHostTask(void*);
#endif
#if(USB_DEVICE==1)
extern void usbDeviceTask(void*);
#endif
extern void StreamingTask(void*); 
extern void WrapRTPTask(void*); 
extern void StreamingRTPTask(void*); 
#if(HOME_RF_SUPPORT)
extern void homeRFTask(void* pData);
#endif

#if(CLOUD_SUPPORT)
extern void CloudServiceTask(void* pData);
#endif

#if ICOMMWIFI_SUPPORT
extern void icomm_init_task(void* args);
#endif 



#if (HW_BOARD_OPTION==MR6730_AFN)	
extern void ui_App_Task(void*);
#endif

/*--------------- External task parameters --------------*/

/* External task stask */
extern OS_STK mainTaskStack[];
extern OS_STK debugTaskStack[];
extern OS_STK siuTaskStack[];
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
extern OS_STK ciuTaskStack_CH1[];
extern OS_STK ciuTaskStack_CH2[];
extern OS_STK ciuTaskStack_CH3[];
extern OS_STK ciuTaskStack_CH4[];

extern OS_STK nicTaskStack[];

extern OS_STK rfiuTaskStack_Unit0[]; 
extern OS_STK rfiuTaskStack_Unit1[]; 
extern OS_STK rfiuTaskStack_Unit2[]; 
extern OS_STK rfiuTaskStack_Unit3[]; 

extern OS_STK rfiuWrapTaskStack_Unit0[]; 
extern OS_STK rfiuWrapTaskStack_Unit1[]; 
extern OS_STK rfiuWrapTaskStack_Unit2[]; 
extern OS_STK rfiuWrapTaskStack_Unit3[]; 

extern OS_STK rfiuDecTaskStack_Unit0[]; 
extern OS_STK rfiuDecTaskStack_Unit1[]; 
extern OS_STK rfiuDecTaskStack_Unit2[]; 
extern OS_STK rfiuDecTaskStack_Unit3[]; 

#endif
extern OS_STK uiTaskStack[];
extern OS_STK sysTaskStack[];
extern OS_STK sysbackTaskStack[];
extern OS_STK sysbackLowTaskStack[];
extern OS_STK mpeg4TaskStack[];
#if MULTI_CHANNEL_VIDEO_REC
extern OS_STK mpeg4TaskStack0[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_STK mpeg4TaskStack1[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_STK mpeg4TaskStack2[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_STK mpeg4TaskStack3[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_STK iisTaskStack0[]; /* Stack of task MultiChannelIISRecordTask() */
extern OS_STK iisTaskStack1[]; /* Stack of task MultiChannelIISRecordTask() */
extern OS_STK iisTaskStack2[]; /* Stack of task MultiChannelIISRecordTask() */
extern OS_STK iisTaskStack3[]; /* Stack of task MultiChannelIISRecordTask() */
#endif
#if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
extern OS_STK iisPlaybackTaskStack[];
#endif
extern OS_STK iisTaskStack[];
extern OS_STK mscTaskStack[];
extern OS_STK vcTaskStack[];
extern OS_STK usbTaskStack[];
extern OS_STK rtcTaskStack[];		 //Civic 070919
extern OS_STK usbPerfTaskStack[];		/* Chi-Lun 20080624 */
#if(USB_HOST==1)
extern OS_STK usbHostTaskStack[];
#endif
#if(USB_DEVICE==1)
extern OS_STK usbDeviceTaskStack[];
#endif
#if UART_COMMAND
extern OS_STK uartCmdTaskStack[];
#endif
#if HIU_COMMAND
extern OS_STK hiuCmdTaskStack[];
#endif
extern OS_STK SysTimerTaskStack[];
#if CDVR_LOG
extern OS_STK LogStringTaskStack[];
#endif
extern OS_STK gTskUIMovieTaskStack[];
extern OS_STK gTskUIPlayTaskStack[];
extern OS_STK gTskUIMenuTaskStack[];
#if(HOME_RF_SUPPORT)
extern OS_STK homeRFStack[];
#endif

#if(CLOUD_SUPPORT)
extern OS_STK cloudServiceStack[];
#endif

#if ICOMMWIFI_SUPPORT
extern OS_STK  icommInitTaskStack[];
#endif

/*-------------- Task function --------------*/
#define MAIN_TASK			mainTask
#define DEBUG_TASK			debugTask
#define SIU_TASK			siuTask
#define CIU_TASK_CH1		ciuTask_CH1
#define CIU_TASK_CH2		ciuTask_CH2
#define CIU_TASK_CH3		ciuTask_CH3
#define CIU_TASK_CH4        ciuTask_CH4
#define NIC_TASK		    nicTask

#define RFIU_TX_TASK_UNITX  rfiu_Tx_Task_UnitX
#define RFIU_RX_TASK_UNITX  rfiu_Rx_Task_UnitX

#define RFIU_TX_WRAP_TASK_UNITX rfiu_WrapTx_Task_UnitX
#define RFIU_RX_WRAP_TASK_UNITX rfiu_WrapRx_Task_UnitX

#define RFIU_RX_DEC_TASK_UNITX rfiu_RxMpeg4DecTask_UnitX

#define RFIU_TXRX_TEST_TASK  marsRfiu_Test    

#define UI_TASK				uiTask
#define UI_SUB_TASK         uiSubTask
#define SYS_TASK			sysTask
#define SYS_BACK_TASK		sysbackTask		  //Civic 070822
#define SYS_BACK_LOW_TASK   sysback_Low_Task  //Lucian 090404
#define SYS_BACK_RF_TASK    sysback_RF_Task   //Lucian 130702
#if (NIC_SUPPORT == 1)
#define SYS_BACK_NET_TASK   sysback_Net_Task
#endif

#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
   #define MPEG4_TASK			rfiuTXMpeg4EncTask
#else
   #define MPEG4_TASK			mpeg4Task
#endif

#if MULTI_CHANNEL_VIDEO_REC
#define SYS_SUB_TASK            sysCaptureVideoSubTask
#define MULTI_CH_MPEG4_ENC_TASK MultiChannelMPEG4EncoderTask
#define MULTI_CH_IIS_REC_TASK   MultiChannelIISRecordTask
#endif
#define MJPG_TASK			mjpgTask      

#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
  #define VIDEO_TASK	        rfiuTXMpeg4EncTask 
#else
  #define VIDEO_TASK	        VideoTask  
#endif

#if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
#define IIS_PLAYBACK_TASK   iisPlaybackTask
#endif
#define IIS_TASK			iisTask
#if IIS_TEST
#define IIS_CH0_TASK		iis_ch0_task
#define IIS_CH1_TASK		iis_ch1_task
#endif
#define USB_MSC_TASK		usbMscTask
#define USB_VC_TASK			usbVcTask
#define USB_TASK			usbTask
#define RTC_TASK			rtcTask
#define USB_PERF_TASK		usbPerfTask		/* Chi-Lun 20080624 */
#if UART_COMMAND
#define UARTCMD_TASK        uartCmdTask
#endif
#if HIU_COMMAND
#define HIUCMD_TASK        	hiuCmdTask
#endif
#define SYSTIMER_TASK        SysTimerTask
#define SYS_TICK_TASK        SysTimerTickTask
#if CDVR_LOG
#define LOGSTRING_TASK       LogStringTask
#endif
#define UIMOVIE_TASK         UIMovieTask
#define UIPLAY_TASK          UIPlayTask
#define UIMENU_TASK          UIMenuTask
#if(USB_HOST==1)
#define USB_HOST_TASK       usbHostTask
#define USB_HOST_MSC_TASK    usbHostMSCTask
#endif
#if(USB_DEVICE==1)
#define USB_DEVICE_TASK       usbDeviceTask
#define USB_DEVICE_MSC_TASK    usbDeviceMSCTask
#endif

#define WRAP_RTP_TASK			WrapRTPTask
#define STREAMING_RTP_TASK			StreamingRTPTask

#if (TUTK_SUPPORT==1)
#define IOTC_LOGIN_TASK      iotcLoginTask
#define IOTC_ROUTINE_TASK    iotcRoutineTask
#endif


#if (HW_BOARD_OPTION==MR6730_AFN)	
#define UI_APP_TASK         ui_App_Task
#endif 


#if(HOME_RF_SUPPORT)
#define HOMERF_TASK          homeRFTask
#endif

#if(CLOUD_SUPPORT)
#define CLOUD_TASK          CloudServiceTask
#endif

#if ICOMMWIFI_SUPPORT   //for icomm 6030p
#define ICOMM_INIT_TASK	icomm_init_task
#endif


/*------------- Task parameter -----------------*/
#define MAIN_TASK_PARAMETER 		(void *) 0
#define DEBUG_TASK_PARAMETER		(void *) 0
#define SIU_TASK_PARAMETER		    (void *) 0

#define CIU_TASK_PARAMETER_CH1		(void *) 0
#define CIU_TASK_PARAMETER_CH2		(void *) 0
#define CIU_TASK_PARAMETER_CH3		(void *) 0
#define CIU_TASK_PARAMETER_CH4		(void *) 0

#define NIC_TASK_PARAMETER		    (void *) 0

#define RFIU_TASK_PARAMETER_UNIT_0  (void *) 0
#define RFIU_TASK_PARAMETER_UNIT_1  (void *) 1
#define RFIU_TASK_PARAMETER_UNIT_2  (void *) 2
#define RFIU_TASK_PARAMETER_UNIT_3  (void *) 3

#define RFIU_WRAP_TASK_PARAMETER_UNIT_0  (void *) 0
#define RFIU_WRAP_TASK_PARAMETER_UNIT_1  (void *) 1
#define RFIU_WRAP_TASK_PARAMETER_UNIT_2  (void *) 2
#define RFIU_WRAP_TASK_PARAMETER_UNIT_3  (void *) 3

#define RFIU_DEC_TASK_PARAMETER_UNIT_0  (void *) 0
#define RFIU_DEC_TASK_PARAMETER_UNIT_1  (void *) 1
#define RFIU_DEC_TASK_PARAMETER_UNIT_2  (void *) 2
#define RFIU_DEC_TASK_PARAMETER_UNIT_3  (void *) 3

#if RF_TX_OPTIMIZE
#define RFIU_ENC_TASK_PARAMETER_UNIT_0  (void *) 0
#define RFIU_ENC_TASK_PARAMETER_UNIT_1  (void *) 1
#define RFIU_ENC_TASK_PARAMETER_UNIT_2  (void *) 2
#define RFIU_ENC_TASK_PARAMETER_UNIT_3  (void *) 3
#endif

#define UI_TASK_PARAMETER		    (void *) 0
#define UI_SUB_TASK_PARAMETER	    (void *) 0
#define SYS_TASK_PARAMETER		    (void *) 0
#define SYSBACK_TASK_PARAMETER		(void *) 0	//Civic 070822
#define SYSBACK_LOW_TASK_PARAMETER  (void *) 0
#define SYSBACK_RF_TASK_PARAMETER   (void *) 0
#if (NIC_SUPPORT == 1)
#define SYSBACK_NET_TASK_PARAMETER   (void *) 0
#endif
#define MPEG4_TASK_PARAMETER		(void *) 0
#define MJPG_TASK_PARAMETER		    (void *) 0//Lsk 090311  
#define VIDEO_TASK_PARAMETER	    (void *) 0//Lsk 100901  
#if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
#define IIS_PLAYBACK_TASK_PARAMETER (void *) 0
#endif
#define IIS_TASK_PARAMETER		    (void *) 0
#if IIS_TEST
#define IIS_CH0_TASK_PARAMETER		(void *) 0
#define IIS_CH1_TASK_PARAMETER		(void *) 0
#endif
#define USB_MSC_TASK_PARAMETER		(void *) 0
#define USB_VC_TASK_PARAMETER		(void *) 0
#define USB_TASK_PARAMETER		    (void *) 0
#define RTC_TASK_PARAMETER		    (void *) 0		//Civic 070919
#define USB_PERF_TASK_PARAMETER		(void *) 0		/* Chi-Lun 20080624 */
#define UARTCMD_TASK_PARAMETER      (void *) 0		/* Peter 20090407 */
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define UART2CMD_TASK_PARAMETER     (void *) 1		/* Peter 20090407 */
#define UART3CMD_TASK_PARAMETER     (void *) 2		/* Peter 20090407 */
#endif
#define HIUCMD_TASK_PARAMETER       (void *) 0		/* Peter 20090407 */
#define SYSTIMER_TASK_PARAMETER     (void *) 0
#define SYSTICK_TASK_PARAMETER     (void *) 0
#if(USB_HOST==1)
#define USB_HOST_TASK_PARAMETER		(void *) 0
#define USB_HOST_MSC_TASK_PARAMETER (void *) 0
#endif
#if(USB_DEVICE==1)
#define USB_DEVICE_TASK_PARAMETER		(void *) 0
#define USB_DEVICE_MSC_TASK_PARAMETER (void *) 0
#endif
#if CDVR_LOG
#define LOGSTRING_TASK_PARAMETER    (void *) 0
#endif
#define UIMOVIE_TASK_PARAMETER      (void *) 0
#define UIPLAY_TASK_PARAMETER       (void *) 0
#define UIMENU_TASK_PARAMETER       (void *) 0
#define STREAMING_RTP_TASK_PARAMETER		(void *) 0
#define WRAP_RTP_TASK_PARAMETER		(void *) 0

#if (HW_BOARD_OPTION==MR6730_AFN)	
#define UI_APP_TASK_PARAMETER		(void *) 0
#endif 

#if(HOME_RF_SUPPORT)
#define HOMERF_TASK_PARAMETER        (void *) 0
#endif

#if(CLOUD_SUPPORT)
#define CLOUD_TASK_PARAMETER        (void *) 0
#endif



/*-------- Task stack size in byte --------*/
#define	MAIN_TASK_STACK_SIZE		        2048
#define DEBUG_TASK_STACK_SIZE		        256
#define SIU_TASK_STACK_SIZE		            2048
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define CIU_TASK_STACK_SIZE_CH1		        1024
#define CIU_TASK_STACK_SIZE_CH2		        1024
#define CIU_TASK_STACK_SIZE_CH3		        1024
#define CIU_TASK_STACK_SIZE_CH4		        1024

#define NIC_TASK_STACK_SIZE		            2048

#define RFIU_TASK_STACK_SIZE_UNIT0          2048
#define RFIU_TASK_STACK_SIZE_UNIT1          2048
#define RFIU_TASK_STACK_SIZE_UNIT2          2048
#define RFIU_TASK_STACK_SIZE_UNIT3          2048

#define RFIU_WRAP_TASK_STACK_SIZE_UNIT0     2048
#define RFIU_WRAP_TASK_STACK_SIZE_UNIT1     2048
#define RFIU_WRAP_TASK_STACK_SIZE_UNIT2     2048
#define RFIU_WRAP_TASK_STACK_SIZE_UNIT3     2048

#define RFIU_DEC_TASK_STACK_SIZE_UNIT0      2048
#define RFIU_DEC_TASK_STACK_SIZE_UNIT1      2048
#define RFIU_DEC_TASK_STACK_SIZE_UNIT2      2048
#define RFIU_DEC_TASK_STACK_SIZE_UNIT3      2048
#endif
#define UI_TASK_STACK_SIZE		            2048 
#define SYS_TASK_STACK_SIZE		            4096	
#define SYSBACK_TASK_STACK_SIZE		        2048	//Lucian: sysback task ­t²ü¤é¯qÄY­«.
#define SYSBACK_RF_TASK_STACK_SIZE          1024
#if (NIC_SUPPORT == 1)
#define SYSBACK_NET_TASK_STACK_SIZE         4096//1024
#endif
#define SYSBACK_LOW_TASK_STACK_SIZE         1024
#define H264_TASK_STACK_SIZE		        1024
#define MPEG4_TASK_STACK_SIZE		        1024
#define MJPG_TASK_STACK_SIZE		        1024//Lsk 090311 
#define VIDEO_TASK_STACK_SIZE		        1024//Lsk 100901
#if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
#define IIS_PLAYBACK_TASK_STACK_SIZE        1024
#endif
#define IIS_TASK_STACK_SIZE		            1024
#if IIS_TEST
#define IIS_CH0_TASK_STACK_SIZE             1024
#define IIS_CH1_TASK_STACK_SIZE             1024
#endif
#define USB_MSC_TASK_STACK_SIZE		        1024	
#define USB_VC_TASK_STACK_SIZE		        1024
#define USB_TASK_STACK_SIZE		            1024
#define RTC_TASK_STACK_SIZE		            1024
#define USB_PERF_TASK_STACK_SIZE	        1024		/* Chi-Lun 20080624 */
#if(USB_HOST==1)
#define USB_HOST_TASK_STACK_SIZE	        1024
#define USB_HOST_MSC_TASK_STACK_SIZE        1024
#endif
#if(USB_DEVICE==1)
#define USB_DEVICE_TASK_STACK_SIZE	        1024
#define USB_DEVICE_MSC_TASK_STACK_SIZE      1024
#endif
#if UART_COMMAND
#define UARTCMD_TASK_STACK_SIZE             1024
#endif
#if HIU_COMMAND
#define HIUCMD_TASK_STACK_SIZE              1024
#endif
#define SYSTIMER_TASK_STACK_SIZE            1024
#define SYSTICK_TASK_STACK_SIZE             256
#if CDVR_LOG
#define LOGSTRING_TASK_STACK_SIZE           1024
#endif
#define UIMOVIE_TASK_STACK_SIZE             256
#define UIPLAY_TASK_STACK_SIZE              256
#define UIMENU_TASK_STACK_SIZE              256
#define STREAMING_RTP_TASK_STACK_SIZE	    1024
#define WRAP_RTP_TASK_STACK_SIZE	        1024
#if (WEB_SERVER_SUPPORT == 1)
#define WEB_TASK_STACK_SIZE                 4096
#define WEB_LISTEN_TASK_STACK_SIZE          4096
#endif

#if (TUTK_SUPPORT==1)
#define IOTC_ROUTINE_STACK_SIZE	            8192
#define IOTC_LOGIN_STACK_SIZE	            1024
#define IOTC_TRANSMISSION_TASK_STACK_SIZE	8192 
#if ICOMMWIFI_SUPPORT		
#define TASK_LOGIN_STACK_SIZE               5120
#define TASK_SESSION_STACK_SIZE             5120
#else
#define TASK_LOGIN_STACK_SIZE               1024
#define TASK_SESSION_STACK_SIZE             16384
#endif
#define TASK_LISTEN_STACK_SIZE              1024
//aher
#define TASK_SPEAKER_STACK_SIZE             1024
#define TASK_P2P_PLAYFILE_STACK_SIZE        1024

#endif
#define LOGO_TASK_STACK_SIZE	            1024
#define UI_SUB_TASK_STACK_SIZE              256

#if (HW_BOARD_OPTION==MR6730_AFN)	
#define UI_APP_TASK_STACK_SIZE             	256
#endif 

#if(HOME_RF_SUPPORT)
#define HOMERF_TASK_STACK_SIZE              256
#endif

#if(CLOUD_SUPPORT)
#define CLOUD_TASK_STACK_SIZE              4096//4096+1024
#endif

#if ICOMMWIFI_SUPPORT
#define ICOMM_INIT_TASK_STACK_SIZE			256//16384
 
#define IOTC_CMD_TASK_STACK_SIZE			5120//16384
#endif
/*---------------- Task stack ------------------*/
#define MAIN_TASK_STACK			&mainTaskStack[MAIN_TASK_STACK_SIZE - 1]
#define DEBUG_TASK_STACK		&debugTaskStack[DEBUG_TASK_STACK_SIZE - 1]
#define SIU_TASK_STACK			&siuTaskStack[SIU_TASK_STACK_SIZE - 1]

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define CIU_TASK_STACK_CH1		&ciuTaskStack_CH1[CIU_TASK_STACK_SIZE_CH1 - 1]
#define CIU_TASK_STACK_CH2		&ciuTaskStack_CH2[CIU_TASK_STACK_SIZE_CH2 - 1]
#define CIU_TASK_STACK_CH3		&ciuTaskStack_CH3[CIU_TASK_STACK_SIZE_CH3 - 1]
#define CIU_TASK_STACK_CH4		&ciuTaskStack_CH4[CIU_TASK_STACK_SIZE_CH4 - 1]

#define NIC_TASK_STACK		    &nicTaskStack[NIC_TASK_STACK_SIZE - 1]

#define RFIU_TASK_STACK_UNIT0   &rfiuTaskStack_Unit0[RFIU_TASK_STACK_SIZE_UNIT0]
#define RFIU_TASK_STACK_UNIT1   &rfiuTaskStack_Unit1[RFIU_TASK_STACK_SIZE_UNIT1]
#define RFIU_TASK_STACK_UNIT2   &rfiuTaskStack_Unit2[RFIU_TASK_STACK_SIZE_UNIT2]
#define RFIU_TASK_STACK_UNIT3   &rfiuTaskStack_Unit3[RFIU_TASK_STACK_SIZE_UNIT3]

#define RFIU_WRAP_TASK_STACK_UNIT0   &rfiuWrapTaskStack_Unit0[RFIU_WRAP_TASK_STACK_SIZE_UNIT0]
#define RFIU_WRAP_TASK_STACK_UNIT1   &rfiuWrapTaskStack_Unit1[RFIU_WRAP_TASK_STACK_SIZE_UNIT1]
#define RFIU_WRAP_TASK_STACK_UNIT2   &rfiuWrapTaskStack_Unit2[RFIU_WRAP_TASK_STACK_SIZE_UNIT2]
#define RFIU_WRAP_TASK_STACK_UNIT3   &rfiuWrapTaskStack_Unit3[RFIU_WRAP_TASK_STACK_SIZE_UNIT3]

#define RFIU_DEC_TASK_STACK_UNIT0   &rfiuDecTaskStack_Unit0[RFIU_DEC_TASK_STACK_SIZE_UNIT0]
#define RFIU_DEC_TASK_STACK_UNIT1   &rfiuDecTaskStack_Unit1[RFIU_DEC_TASK_STACK_SIZE_UNIT1]
#define RFIU_DEC_TASK_STACK_UNIT2   &rfiuDecTaskStack_Unit2[RFIU_DEC_TASK_STACK_SIZE_UNIT2]
#define RFIU_DEC_TASK_STACK_UNIT3   &rfiuDecTaskStack_Unit3[RFIU_DEC_TASK_STACK_SIZE_UNIT3]
#endif

#define UI_TASK_STACK			&uiTaskStack[UI_TASK_STACK_SIZE - 1]
#define SYS_TASK_STACK			&sysTaskStack[SYS_TASK_STACK_SIZE - 1]
#define SYSBACK_TASK_STACK		&sysbackTaskStack[SYSBACK_TASK_STACK_SIZE - 1]	     //civic 070822
#define SYSBACK_LOW_TASK_STACK  &sysbackLowTaskStack[SYSBACK_LOW_TASK_STACK_SIZE-1]
#define SYSBACK_RF_TASK_STACK   &sysback_RF_TaskStack[SYSBACK_RF_TASK_STACK_SIZE-1]
#define MPEG4_TASK_STACK		&mpeg4TaskStack[MPEG4_TASK_STACK_SIZE - 1]
#if MULTI_CHANNEL_VIDEO_REC
#define SYS_SUB_TASK_STACK0         &sysSubTaskStack0[SYS_TASK_STACK_SIZE - 1]
#define SYS_SUB_TASK_STACK1         &sysSubTaskStack1[SYS_TASK_STACK_SIZE - 1]
#define SYS_SUB_TASK_STACK2         &sysSubTaskStack2[SYS_TASK_STACK_SIZE - 1]
#define SYS_SUB_TASK_STACK3         &sysSubTaskStack3[SYS_TASK_STACK_SIZE - 1]
#define MULTI_CH_MPEG4_TASK_STACK0  &mpeg4TaskStack0[MPEG4_TASK_STACK_SIZE - 1]
#define MULTI_CH_MPEG4_TASK_STACK1  &mpeg4TaskStack1[MPEG4_TASK_STACK_SIZE - 1]
#define MULTI_CH_MPEG4_TASK_STACK2  &mpeg4TaskStack2[MPEG4_TASK_STACK_SIZE - 1]
#define MULTI_CH_MPEG4_TASK_STACK3  &mpeg4TaskStack3[MPEG4_TASK_STACK_SIZE - 1]
#define MULTI_CH_IIS_TASK_STACK0    &iisTaskStack0[IIS_TASK_STACK_SIZE - 1]
#define MULTI_CH_IIS_TASK_STACK1    &iisTaskStack1[IIS_TASK_STACK_SIZE - 1]
#define MULTI_CH_IIS_TASK_STACK2    &iisTaskStack2[IIS_TASK_STACK_SIZE - 1]
#define MULTI_CH_IIS_TASK_STACK3    &iisTaskStack3[IIS_TASK_STACK_SIZE - 1]
#endif
#define MJPG_TASK_STACK			&mjpgTaskStack[MJPG_TASK_STACK_SIZE - 1]	//Lsk 090311  
#define VIDEO_TASK_STACK	    &VideoTaskStack[VIDEO_TASK_STACK_SIZE - 1]	//Lsk 090311  
#if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
#define IIS_PLAYBACK_TASK_STACK &iisPlaybackTaskStack[IIS_PLAYBACK_TASK_STACK_SIZE - 1]
#endif
#define IIS_TASK_STACK			&iisTaskStack[IIS_TASK_STACK_SIZE - 1]
#if IIS_TEST
#define IIS_CH0_TASK_STACK		&iisCh0TaskStack[IIS_CH0_TASK_STACK_SIZE - 1]
#define IIS_CH1_TASK_STACK		&iisCh1TaskStack[IIS_CH1_TASK_STACK_SIZE - 1]
#endif
#define USB_MSC_TASK_STACK		&usbMscTaskStack[USB_MSC_TASK_STACK_SIZE - 1]
#if USB2WIFI_SUPPORT
#define USB_VC_TASK_STACK		&usbVcTaskStack[USB_VC_TASK_STACK_SIZE - 1]
#endif
#define USB_TASK_STACK			&usbTaskStack[USB_TASK_STACK_SIZE - 1]
#define RTC_TASK_STACK			&rtcTaskStack[RTC_TASK_STACK_SIZE - 1]
#define USB_PERF_TASK_STACK		&usbPerfTaskStack[USB_PERF_TASK_STACK_SIZE - 1]		/* Chi-Lun 20080624 */
#if(USB_HOST==1)
#define USB_HOST_TASK_STACK	    &usbHostTaskStack[USB_HOST_TASK_STACK_SIZE - 1]
#define USB_HOST_MSC_TASK_STACK	&usbHostMscTaskStack[USB_HOST_MSC_TASK_STACK_SIZE - 1]
#endif
#if(USB_DEVICE==1)
#define USB_DEVICE_TASK_STACK	     &usbDeviceTaskStack[USB_DEVICE_TASK_STACK_SIZE - 1]
#define USB_DEVICE_MSC_TASK_STACK	 & usbDeviceMscTaskStack[USB_DEVICE_MSC_TASK_STACK_SIZE - 1]
#endif
#if UART_COMMAND
#define UARTCMD_TASK_STACK      &uartCmdTaskStack[UARTCMD_TASK_STACK_SIZE-1]
    #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
        (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    #define UART2CMD_TASK_STACK      &uart2CmdTaskStack[UARTCMD_TASK_STACK_SIZE-1]
    #define UART3CMD_TASK_STACK      &uart3CmdTaskStack[UARTCMD_TASK_STACK_SIZE-1]
    #endif
#endif
#if HIU_COMMAND
#define HIUCMD_TASK_STACK       &hiuCmdTaskStack[HIUCMD_TASK_STACK_SIZE-1]
#endif
#define SYSTIMER_TASK_STACK     &SysTimerTaskStack[SYSTIMER_TASK_STACK_SIZE-1]
#define SYSTICK_TASK_STACK     &SysTickTaskStack[SYSTICK_TASK_STACK_SIZE-1]
#if CDVR_LOG
#define LOGSTRING_TASK_STACK    &LogStringTaskStack[LOGSTRING_TASK_STACK_SIZE-1]
#endif
#define UIMOVIE_TASK_STACK       &gTskUIMovieTaskStack[UIMOVIE_TASK_STACK_SIZE-1]
#define UIPLAY_TASK_STACK        &gTskUIPlayTaskStack[UIPLAY_TASK_STACK_SIZE-1]
#define UIMENU_TASK_STACK        &gTskUIMenuTaskStack[UIMENU_TASK_STACK_SIZE-1]
#define STREAMING_RTP_TASK_STACK &StreamingRTPTaskStack[STREAMING_RTP_TASK_STACK_SIZE-1]
#define WRAP_RTP_TASK_STACK      &WRAPRTPTaskStack[WRAP_RTP_TASK_STACK_SIZE-1]
#if (WEB_SERVER_SUPPORT == 1)
#define WEB_TASK_STACK           &webStack[WEB_TASK_STACK_SIZE-1]
#define WEB_LISTEN_TASK_STACK    &webListenStack[WEB_LISTEN_TASK_STACK_SIZE-1]
#endif

#if (TUTK_SUPPORT==1)
#define IOTC_ROUTINE_TASK_STACK  &gIotcRoutineTaskStack[IOTC_ROUTINE_STACK_SIZE-1]
#define IOTC_LOGIN_TASK_STACK    &gIotcLoginTaskStack[IOTC_LOGIN_STACK_SIZE-1]
#define IOTC_TRANSMISSION_TASK_STACK &gIotcTransmissionTaskStack[IOTC_TRANSMISSION_TASK_STACK_SIZE-1]
#define IOTC_CMD_TASK_STACK   	&IOTCcmdTaskStack[IOTC_CMD_TASK_STACK_SIZE-1]
#define SESSION_TASK_STACK  &gSessionTaskStack[TASK_SESSION_STACK_SIZE-1]
#define LOGIN_TASK_STACK    &gLoginTaskStack[TASK_LOGIN_STACK_SIZE-1]
#define LISTEN_TASK_STACK    &gListenTaskStack[TASK_LISTEN_STACK_SIZE-1]
//aher
#define SPEAKER_TASK_STACK &gSpeakerTaskStack[TASK_SPEAKER_STACK_SIZE-1]
#define P2P_PLAYFILE_TASK_STACK &gP2PPlayfileTaskStack[TASK_P2P_PLAYFILE_STACK_SIZE-1]

#endif
#define LOGO_TASK_STACK &LogoTaskStack[LOGO_TASK_STACK_SIZE-1]
#define UI_SUB_TASK_STACK  &uiSubTaskStack[UI_SUB_TASK_STACK_SIZE-1]

#if (NIC_SUPPORT == 1)
#define SYSBACK_NET_TASK_STACK  &sysbackNetTaskStack[SYSBACK_NET_TASK_STACK_SIZE-1]
#endif

#if (HW_BOARD_OPTION==MR6730_AFN)	
#define UI_APP_TASK_STACK       &gTskUIAppTaskStack[UI_APP_TASK_STACK_SIZE-1]
#endif 

#if(HOME_RF_SUPPORT)
#define HOME_RF_TASK_STACK      &homeRFTaskStack[HOMERF_TASK_STACK_SIZE-1]
#endif

#if(CLOUD_SUPPORT)
#define CLOUD_TASK_STACK      &cloudServiceTaskStack[CLOUD_TASK_STACK_SIZE-1]
#endif

#if ICOMMWIFI_SUPPORT
#define ICOMM_INIT_TASK_STACK   &icommInitTaskStack[ICOMM_INIT_TASK_STACK_SIZE-1]
#endif
/*--------------------------- Task priority-------------------------------- */

#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) ||(SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFTX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M))
enum
{
    RFIU_TASK_PRIORITY_HIGH = 11,
    MAIN_TASK_PRIORITY_START,
#if IIS_TEST
    IIS_CH0_TASK_PRIORITY,
    IIS_CH1_TASK_PRIORITY,
#endif
    TIMER_TICK_TASK_PRIORITY,
    /*RF*/
    RFIU_TASK_PRIORITY_UNIT1,
    RFIU_TASK_PRIORITY_UNIT0,
    RFIU_TASK_PRIORITY_UNIT2,
    RFIU_TASK_PRIORITY_UNIT3,

    SYSTIMER_TASK_PRIORITY,
    
#if USB2WIFI_SUPPORT
    USB_TASK_PRIORITY,
    USB_DEVICE_TASK_PRIORITY,
#endif
    /*IIS*/
    IIS_TASK_PRIORITY_UNIT0,
    IIS_TASK_PRIORITY_UNIT1,
    IIS_TASK_PRIORITY_UNIT2,
    IIS_TASK_PRIORITY_UNIT3,
#if AUDIO_IN_TO_OUT
    IIS_PLAYBACK_TASK_PRIORITY,
#endif

    /*mpeg4*/
    MPEG4_TASK_PRIORITY_UNIT0,
    MPEG4_TASK_PRIORITY_UNIT1,
    MPEG4_TASK_PRIORITY_UNIT2,
    MPEG4_TASK_PRIORITY_UNIT3,

    UI_TASK_PRIORITY,
    UARTCMD_TASK1_PRIORITY,
    UARTCMD_TASK2_PRIORITY,
    

    /*preview*/
    SIU_TASK_PRIORITY_PVW,
    CIU_TASK_PRIORITY_CH1,
    CIU_TASK_PRIORITY_CH2,
    CIU_TASK_PRIORITY_CH3,
    CIU_TASK_PRIORITY_CH4,
    /*RF*/
    RFIU_WRAP_TASK_PRIORITY_UNIT0,
    RFIU_WRAP_TASK_PRIORITY_UNIT1,
    RFIU_WRAP_TASK_PRIORITY_UNIT2,
    RFIU_WRAP_TASK_PRIORITY_UNIT3,

    /*asf packer*/
    SYS_SUB_TASK_PRIORITY_UNIT0,
    SYS_SUB_TASK_PRIORITY_UNIT1,
    SYS_SUB_TASK_PRIORITY_UNIT2,
    SYS_SUB_TASK_PRIORITY_UNIT3,

    UI_SUB_TASK_PRIORITY,
    
    /*system*/
    SYS_TASK_PRIORITY,
    SYSBACK_TASK_PRIORITY,
    SYSBACK_RF_TASK_PRIORITY,
    SYSBACK_LOW_TASK_PRIORITY,
    MAIN_TASK_PRIORITY_END
};

#define IIS_TASK_PRIORITY               IIS_TASK_PRIORITY_UNIT0
#define MJPG_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define MPEG4_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define VIDEO_TASK_PRIORITY             MPEG4_TASK_PRIORITY_UNIT3
#define UARTCMD_TASK3_PRIORITY          UARTCMD_TASK1_PRIORITY
#define DISPLAY_LOGO_PRIORITY           RFIU_TASK_PRIORITY_HIGH

    /*no use, for build code */
#define DEBUG_TASK_PRIORITY             0
#if USB2WIFI_SUPPORT
#define USB_MSC_TASK_PRIORITY           USB_DEVICE_TASK_PRIORITY
#define USB_VC_TASK_PRIORITY            USB_DEVICE_TASK_PRIORITY
#else
#define USB_MSC_TASK_PRIORITY           0
#define USB_VC_TASK_PRIORITY            0
#endif
#define USB_PERF_TASK_PRIORITY          0
#define USB_TASK_PRIORITY               0
#define RTC_TASK_PRIORITY               0
#define NIC_TASK_PRIORITY               0
#define T_LWIPENTRY_PRIOR               0
#define T_LWIP_THREAD_START_PRIO        0
#define STREAMING_RTP_TASK_PRIORITY     0
#define WRAP_RTP_TASK_PRIORITY          0
#define	T_LWIP_THREAD_STKSIZE           1024
#define	T_LWIP_THREAD_MAX_NB	        4
#define T_ETHERNETIF_INPUT_PRIO         0
#define RFIU_DEC_TASK_PRIORITY_UNIT0    0
#define RFIU_DEC_TASK_PRIORITY_UNIT1    0
#define RFIU_DEC_TASK_PRIORITY_UNIT2    0
#define RFIU_DEC_TASK_PRIORITY_UNIT3    0

#elif(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
enum
{
    RFIU_TASK_PRIORITY_HIGH = 4,
    MAIN_TASK_PRIORITY_START,
#if IIS_TEST
    IIS_CH0_TASK_PRIORITY,
    IIS_CH1_TASK_PRIORITY,
#endif
    TIMER_TICK_TASK_PRIORITY,
    /*RF*/
    RFIU_TASK_PRIORITY_UNIT0,

    SYSTIMER_TASK_PRIORITY,
    
#if USB2WIFI_SUPPORT
    USB_TASK_PRIORITY,
    USB_DEVICE_TASK_PRIORITY,
#endif
    /*IIS*/
    IIS_TASK_PRIORITY_UNIT0,
#if AUDIO_IN_TO_OUT
    IIS_PLAYBACK_TASK_PRIORITY,
#endif

    /*mpeg4*/
    MPEG4_TASK_PRIORITY_UNIT3,

    /*RF*/
    RFIU_WRAP_TASK_PRIORITY_UNIT0,
    SYSBACK_RF_TASK_PRIORITY,	//20

#if ICOMMWIFI_SUPPORT
	/*Icomm uesd 14 tasks*/
	ICOMM_TASK_01_PRIORITY	,
	ICOMM_TASK_02_PRIORITY	,
	ICOMM_TASK_03_PRIORITY ,
	ICOMM_TASK_04_PRIORITY ,
	ICOMM_TASK_05_PRIORITY ,  //14
	ICOMM_TASK_06_PRIORITY ,
	ICOMM_TASK_07_PRIORITY ,
	ICOMM_TASK_08_PRIORITY ,
	ICOMM_TASK_09_PRIORITY ,
	ICOMM_TASK_10_PRIORITY ,
	ICOMM_TASK_11_PRIORITY ,
	ICOMM_TASK_12_PRIORITY ,
	ICOMM_TASK_13_PRIORITY ,
	ICOMM_TASK_14_PRIORITY ,	//25
#endif						
		
	SESSION_TASK_PRIORITY,		//32

#if TUTK_SUPPORT /* Use for icomm*/
	IOTC_CMD_TASK_PRIORITY,
	SPEAKER_TASK_PRIORITY,		//27
	P2P_PLAYFILE_TASK_PRIORITY, //28
	IOTC_LOGIN_TASK_PRIORITY,	//29
	LISTEN_TASK_PRIORITY,		//30
	LOGIN_TASK_PRIORITY,		//31
#endif


    UI_TASK_PRIORITY,
    UARTCMD_TASK1_PRIORITY,
    UARTCMD_TASK2_PRIORITY,
    
    /*preview*/
    SIU_TASK_PRIORITY_PVW,
    CIU_TASK_PRIORITY_CH1,

				
				
				
#if (NIC_SUPPORT == 1)
#if 0//WEB_SERVER_SUPPORT
	WEB_LISTEN_TASK_PRIORITY,
	WEB_TASK_PRIORITY,
	WEB_TASK2_PRIORITY,
	WEB_TASK3_PRIORITY,
	WEB_TASK4_PRIORITY,
	WEB_LISTEN_SSL_TASK_PRIORITY,
#endif
				
#if 0 //Sean
	/*RTP*/
	WRAP_RTP_TASK_PRIORITY,
	STREAMING_RTP_TASK_PRIORITY,
	TELNET_THREAD_PRIO_t,	  /*keep in last of network*/
#endif
#endif
    UI_SUB_TASK_PRIORITY,		//17
    
    /*system*/
    SYS_TASK_PRIORITY,			//18
    SYSBACK_TASK_PRIORITY,		//19

	/*SPI wireless dongle*/
	ICOMM_INIT_TASK_PRIORITY,	//26

IOTC_ROUTINE_TASK_PRIORITY, 		//10
IOTC_TRANSMISSION_TASK_PRIORITY,  //11


#if (NIC_SUPPORT == 1)
    SYSBACK_NET_TASK_PRIORITY,	//44
#endif  
    SYSBACK_LOW_TASK_PRIORITY,	//45
    MAIN_TASK_PRIORITY_END		//46
};

#define IIS_TASK_PRIORITY               IIS_TASK_PRIORITY_UNIT0
#define MJPG_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define MPEG4_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define VIDEO_TASK_PRIORITY             MPEG4_TASK_PRIORITY_UNIT3
#define UARTCMD_TASK3_PRIORITY          UARTCMD_TASK1_PRIORITY
#define DISPLAY_LOGO_PRIORITY           RFIU_TASK_PRIORITY_HIGH

    /*no use, for build code */
#define DEBUG_TASK_PRIORITY             0
#if USB2WIFI_SUPPORT
#define USB_MSC_TASK_PRIORITY           USB_DEVICE_TASK_PRIORITY
#define USB_VC_TASK_PRIORITY            USB_DEVICE_TASK_PRIORITY
#else
#define USB_MSC_TASK_PRIORITY           0
#define USB_VC_TASK_PRIORITY            0
#endif
#define USB_PERF_TASK_PRIORITY          0
#define USB_TASK_PRIORITY               0
#define RTC_TASK_PRIORITY               0
#define NIC_TASK_PRIORITY               0
#define T_LWIPENTRY_PRIOR               0
#define T_LWIP_THREAD_START_PRIO        0
#define STREAMING_RTP_TASK_PRIORITY     0
#define WRAP_RTP_TASK_PRIORITY          0
#define	T_LWIP_THREAD_STKSIZE           1024
#define	T_LWIP_THREAD_MAX_NB	        4
#define T_ETHERNETIF_INPUT_PRIO         0
#define RFIU_DEC_TASK_PRIORITY_UNIT0    0
#define RFIU_DEC_TASK_PRIORITY_UNIT1    0
#define RFIU_DEC_TASK_PRIORITY_UNIT2    0
#define RFIU_DEC_TASK_PRIORITY_UNIT3    0

#define RFIU_TASK_PRIORITY_UNIT1        0
#define RFIU_TASK_PRIORITY_UNIT2        0
#define RFIU_TASK_PRIORITY_UNIT3        0

#define RFIU_WRAP_TASK_PRIORITY_UNIT1   0
#define RFIU_WRAP_TASK_PRIORITY_UNIT2   0
#define RFIU_WRAP_TASK_PRIORITY_UNIT3   0

    /*asf packer*/
#define SYS_SUB_TASK_PRIORITY_UNIT0     0
#define SYS_SUB_TASK_PRIORITY_UNIT1     0
#define SYS_SUB_TASK_PRIORITY_UNIT2     0
#define SYS_SUB_TASK_PRIORITY_UNIT3     0

#define MPEG4_TASK_PRIORITY_UNIT0       0
#define MPEG4_TASK_PRIORITY_UNIT1       0
#define MPEG4_TASK_PRIORITY_UNIT2       0

#define IIS_TASK_PRIORITY_UNIT1         0
#define IIS_TASK_PRIORITY_UNIT2         0
#define IIS_TASK_PRIORITY_UNIT3         0

#define CIU_TASK_PRIORITY_CH2           CIU_TASK_PRIORITY_CH1
#define CIU_TASK_PRIORITY_CH3           CIU_TASK_PRIORITY_CH1
#define CIU_TASK_PRIORITY_CH4           CIU_TASK_PRIORITY_CH1

#if !ICOMMWIFI_SUPPORT
    /*Icomm uesd 14 tasks*/
#define     ICOMM_TASK_01_PRIORITY 64
#define     ICOMM_TASK_02_PRIORITY 64
#define     ICOMM_TASK_03_PRIORITY 64
#define     ICOMM_TASK_04_PRIORITY 64
#define     ICOMM_TASK_05_PRIORITY 64
#define     ICOMM_TASK_06_PRIORITY 64
#define     ICOMM_TASK_07_PRIORITY 64
#define     ICOMM_TASK_08_PRIORITY 64
#define     ICOMM_TASK_09_PRIORITY 64
#define     ICOMM_TASK_10_PRIORITY 64
#define     ICOMM_TASK_11_PRIORITY 64
#define     ICOMM_TASK_12_PRIORITY 64
#define     ICOMM_TASK_13_PRIORITY 64
#define     ICOMM_TASK_14_PRIORITY 64

    /*SPI wireless dongle*/
#define 	ICOMM_INIT_TASK_PRIORITY	64
#endif


#elif ( (SW_APPLICATION_OPTION ==DVP_RF_SELFTEST) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFU) ||(SW_APPLICATION_OPTION == MR8200_RFCAM_RX1)||\
        (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFRX1)  ) 
enum
{
    RFIU_TASK_PRIORITY_HIGH = 4,
#if IIS_TEST   
    IIS_CH1_TASK_PRIORITY,
    IIS_CH0_TASK_PRIORITY,
#endif
    TIMER_TICK_TASK_PRIORITY,

    /*RF*/
    RFIU_TASK_PRIORITY_UNIT0,
    RFIU_TASK_PRIORITY_UNIT1,
    RFIU_TASK_PRIORITY_UNIT2,
    RFIU_TASK_PRIORITY_UNIT3,

    SYSTIMER_TASK_PRIORITY,
    MAIN_TASK_PRIORITY_START,

    /*IIS*/
    IIS_TASK_PRIORITY_UNIT0,
    IIS_TASK_PRIORITY_UNIT1,
    IIS_TASK_PRIORITY_UNIT2,
    IIS_TASK_PRIORITY_UNIT3,

    /*network*/
    T_LWIP_THREAD_START_PRIO,
#if TUTK_SUPPORT
    IOTC_ROUTINE_TASK_PRIORITY,
    IOTC_TRANSMISSION_TASK_PRIORITY,
    
#endif
    TCPIP_THREAD_PRIO_t,
    T_ETHERNETIF_INPUT_PRIO,

    /*RF*/
    RFIU_DEC_TASK_PRIORITY_UNIT0,
    RFIU_DEC_TASK_PRIORITY_UNIT1,
    RFIU_DEC_TASK_PRIORITY_UNIT2,
    RFIU_DEC_TASK_PRIORITY_UNIT3,

    /*mpeg4*/
    MPEG4_TASK_PRIORITY_UNIT0,
    MPEG4_TASK_PRIORITY_UNIT1,
    MPEG4_TASK_PRIORITY_UNIT2,
    MPEG4_TASK_PRIORITY_UNIT3,

#if(HW_BOARD_OPTION != MR8200_RX_TRANWO_SMH101_HA)
    USB_TASK_PRIORITY,
    USB_DEVICE_TASK_PRIORITY,
    USB_PERF_TASK_PRIORITY,
#endif

    /*USB*/
#if CDVR_LOG
    LOGSTRING_TASK_PRIORITY,
#endif
    UI_TASK_PRIORITY,
    UARTCMD_TASK3_PRIORITY,
    UARTCMD_TASK2_PRIORITY,
    UARTCMD_TASK1_PRIORITY,
    /*RTP*/
    WRAP_RTP_TASK_PRIORITY,
    STREAMING_RTP_TASK_PRIORITY,
    
    SIU_TASK_PRIORITY_PVW,
    CIU_TASK_PRIORITY_CH1,
    CIU_TASK_PRIORITY_CH2,
    CIU_TASK_PRIORITY_CH3,
 #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )   
    CIU_TASK_PRIORITY_CH4,
 #endif
/*RF*/
    RFIU_WRAP_TASK_PRIORITY_UNIT0,
    RFIU_WRAP_TASK_PRIORITY_UNIT1,
    RFIU_WRAP_TASK_PRIORITY_UNIT2,
    RFIU_WRAP_TASK_PRIORITY_UNIT3,
#if TUTK_SUPPORT
    //aher 
    SPEAKER_TASK_PRIORITY,
    P2P_PLAYFILE_TASK_PRIORITY,
    IOTC_LOGIN_TASK_PRIORITY,
    LISTEN_TASK_PRIORITY,
    LOGIN_TASK_PRIORITY,
#endif
    /*Change Video record priority by aher 20130107*/
    /*asf*/
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    SYS_SUB_TASK_PRIORITY_UNIT0,
    SYS_SUB_TASK_PRIORITY_UNIT1,
    SYS_SUB_TASK_PRIORITY_UNIT2,
    SYS_SUB_TASK_PRIORITY_UNIT3,
#endif

#if TUTK_SUPPORT
    SESSION_TASK_PRIORITY,
#endif

#if(HOME_RF_SUPPORT)
    HOMERF_TASK_PRIORITY,
#endif

#if CLOUD_SUPPORT
	CLOUD_TASK_PRIORITY,
#endif
    
    /*system*/
    SYS_TASK_PRIORITY,
    SYSBACK_RF_TASK_PRIORITY,
  #if (NIC_SUPPORT == 1)
    SYSBACK_NET_TASK_PRIORITY,
  #endif
    SYSBACK_TASK_PRIORITY,
    SYSBACK_LOW_TASK_PRIORITY,

    /*UI */
    UI_SUB_TASK_PRIORITY,
    
    MAIN_TASK_PRIORITY_END
};

#define IIS_TASK_PRIORITY               IIS_TASK_PRIORITY_UNIT0
#define T_LWIPENTRY_PRIOR               TCPIP_THREAD_PRIO_t
#define NIC_TASK_PRIORITY               T_ETHERNETIF_INPUT_PRIO
#define DISPLAY_LOGO_PRIORITY           RFIU_TASK_PRIORITY_HIGH

#if(HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA)
#define USB_TASK_PRIORITY				64
#define USB_DEVICE_TASK_PRIORITY		64
#define USB_PERF_TASK_PRIORITY			64
#endif

/*mpeg4*/
//aher
#define	USB_DEVICE_MSC_TASK_PRIORITY	USB_TASK_PRIORITY
#define	USB_HOST_TASK_PRIORITY	        USB_TASK_PRIORITY
#define	USB_VC_TASK_PRIORITY            USB_DEVICE_TASK_PRIORITY
#define	USB_MSC_TASK_PRIORITY	        USB_DEVICE_TASK_PRIORITY
#define	USB_HOST_MSC_TASK_PRIORITY	    USB_DEVICE_TASK_PRIORITY

#define	T_LWIP_THREAD_MAX_NB 		    2
#define	T_LWIP_THREAD_STKSIZE	        1024 	
#define	T_LWIPENTRY_STKSIZE		        2048

#define MJPG_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define MPEG4_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define VIDEO_TASK_PRIORITY             MPEG4_TASK_PRIORITY_UNIT3

/*no use, for build code */
//aher
#define DEBUG_TASK_PRIORITY             64
#define USB_PERF_TASK_PRIORITY          64
#define USB_TASK_PRIORITY               0
#define RTC_TASK_PRIORITY               64
//aher
/*
#define SIU_TASK_PRIORITY_PVW           64
#define CIU_TASK_PRIORITY_CH1           64
#define CIU_TASK_PRIORITY_CH2           64
#define CIU_TASK_PRIORITY_CH3           64
*/
#elif ( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) )
enum
{
    RFIU_TASK_PRIORITY_HIGH = 10,
    MAIN_TASK_PRIORITY_START,
#if IIS_TEST   
    IIS_CH1_TASK_PRIORITY,
    IIS_CH0_TASK_PRIORITY,
#endif
    
    TIMER_TICK_TASK_PRIORITY,
    /*RF*/
    RFIU_TASK_PRIORITY_UNIT1,
    RFIU_TASK_PRIORITY_UNIT0,
    RFIU_TASK_PRIORITY_UNIT2,
    RFIU_TASK_PRIORITY_UNIT3,

    SYSTIMER_TASK_PRIORITY,

    /*IIS*/
    IIS_TASK_PRIORITY_UNIT0,
    IIS_TASK_PRIORITY_UNIT1,
    IIS_TASK_PRIORITY_UNIT2,
    IIS_TASK_PRIORITY_UNIT3,
#if AUDIO_IN_TO_OUT
    IIS_PLAYBACK_TASK_PRIORITY,
#endif

    /*RF*/
    RFIU_DEC_TASK_PRIORITY_UNIT0,
    RFIU_DEC_TASK_PRIORITY_UNIT1,
    RFIU_DEC_TASK_PRIORITY_UNIT2,
    RFIU_DEC_TASK_PRIORITY_UNIT3,

    /*mpeg4*/
    MPEG4_TASK_PRIORITY_UNIT0,
    MPEG4_TASK_PRIORITY_UNIT1,
    MPEG4_TASK_PRIORITY_UNIT2,
    MPEG4_TASK_PRIORITY_UNIT3,

    /*USB*/
    USB_HOST_MSC_TASK_PRIORITY,

    UI_TASK_PRIORITY,
    UARTCMD_TASK1_PRIORITY,
    UARTCMD_TASK2_PRIORITY,
    /*RF*/
    RFIU_WRAP_TASK_PRIORITY_UNIT0,
    RFIU_WRAP_TASK_PRIORITY_UNIT1,
    RFIU_WRAP_TASK_PRIORITY_UNIT2,
    RFIU_WRAP_TASK_PRIORITY_UNIT3,

    /*asf*/
    SYS_SUB_TASK_PRIORITY_UNIT0,
    SYS_SUB_TASK_PRIORITY_UNIT1,
    SYS_SUB_TASK_PRIORITY_UNIT2,
    SYS_SUB_TASK_PRIORITY_UNIT3,

    

    /*system*/
    SYS_TASK_PRIORITY,
    SYSBACK_TASK_PRIORITY,
    SYSBACK_RF_TASK_PRIORITY,
    SYSBACK_LOW_TASK_PRIORITY,

    /*UI */
    UI_SUB_TASK_PRIORITY,


    MAIN_TASK_PRIORITY_END
};

#define IIS_TASK_PRIORITY               IIS_TASK_PRIORITY_UNIT0

#define MJPG_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define MPEG4_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define VIDEO_TASK_PRIORITY             MPEG4_TASK_PRIORITY_UNIT3


#define UARTCMD_TASK3_PRIORITY          UARTCMD_TASK1_PRIORITY
#define DISPLAY_LOGO_PRIORITY           RFIU_TASK_PRIORITY_HIGH

    /*no use, for build code */
#define DEBUG_TASK_PRIORITY             64
#define USB_MSC_TASK_PRIORITY           64
#define USB_VC_TASK_PRIORITY            64
#define USB_PERF_TASK_PRIORITY          64
#define USB_TASK_PRIORITY               64
#define RTC_TASK_PRIORITY               64
#define SIU_TASK_PRIORITY_PVW           64
#define CIU_TASK_PRIORITY_CH1           64
#define CIU_TASK_PRIORITY_CH2           64
#define CIU_TASK_PRIORITY_CH3           64
#define CIU_TASK_PRIORITY_CH4           64

#define NIC_TASK_PRIORITY               64
#define T_LWIPENTRY_PRIOR               64
#define T_LWIP_THREAD_START_PRIO        64
#define STREAMING_RTP_TASK_PRIORITY     64
#define WRAP_RTP_TASK_PRIORITY          64
#define	T_LWIP_THREAD_STKSIZE           1024
#define	T_LWIP_THREAD_MAX_NB	        4
#define T_ETHERNETIF_INPUT_PRIO         0

#elif ( (SW_APPLICATION_OPTION == MR6730_CARDVR_2CH) || (SW_APPLICATION_OPTION == MR9670_DOORPHONE))
enum
{
    MAIN_TASK_PRIORITY_START = 4,
#if IIS_TEST   
    IIS_CH1_TASK_PRIORITY,
    IIS_CH0_TASK_PRIORITY,
#endif
    
    TIMER_TICK_TASK_PRIORITY,
    SYSTIMER_TASK_PRIORITY,

    /*IIS*/
    IIS_TASK_PRIORITY_UNIT0,
    IIS_TASK_PRIORITY_UNIT1,
    IIS_TASK_PRIORITY_UNIT2,
    IIS_TASK_PRIORITY_UNIT3,
#if AUDIO_IN_TO_OUT
    IIS_PLAYBACK_TASK_PRIORITY,
#endif

    RFIU_TASK_PRIORITY_HIGH,
    RFIU_TASK_PRIORITY_UNIT1,
    RFIU_TASK_PRIORITY_UNIT0,

#if (NIC_SUPPORT == 1)
    /*network*/
    T_LWIPENTRY_PRIOR,
    TCPIP_THREAD_PRIO_t,
    T_ETHERNETIF_INPUT_PRIO,
    #if (WEB_SERVER_SUPPORT == 1)
        WEB_LISTEN_TASK_PRIORITY,
        WEB_TASK_PRIORITY,
        WEB_TASK2_PRIORITY,
        WEB_TASK3_PRIORITY,
        WEB_TASK4_PRIORITY,
        WEB_LISTEN_SSL_TASK_PRIORITY,
    #endif
    #if TUTK_SUPPORT
        IOTC_ROUTINE_TASK_PRIORITY,
        IOTC_LOGIN_TASK_PRIORITY,
        LISTEN_TASK_PRIORITY,
        LOGIN_TASK_PRIORITY,
        SESSION_TASK_PRIORITY,
    #endif
    /*RTP*/
    WRAP_RTP_TASK_PRIORITY,
    STREAMING_RTP_TASK_PRIORITY,
    TELNET_THREAD_PRIO_t,     /*keep in last of network*/
#endif

    /*mpeg4*/
    MPEG4_TASK_PRIORITY_UNIT0,
    MPEG4_TASK_PRIORITY_UNIT1,
    MPEG4_TASK_PRIORITY_UNIT2,
    MPEG4_TASK_PRIORITY_UNIT3,

    /*USB*/
    USB_HOST_MSC_TASK_PRIORITY,

    UI_TASK_PRIORITY,
    UARTCMD_TASK1_PRIORITY,

    /*RF*/
    RFIU_WRAP_TASK_PRIORITY_UNIT0,
    RFIU_WRAP_TASK_PRIORITY_UNIT1,

    /*preview*/
    SIU_TASK_PRIORITY_PVW,
    CIU_TASK_PRIORITY_CH1,
    CIU_TASK_PRIORITY_CH2,
    CIU_TASK_PRIORITY_CH3,

    /*asf*/
    SYS_SUB_TASK_PRIORITY_UNIT0,
    SYS_SUB_TASK_PRIORITY_UNIT1,
    SYS_SUB_TASK_PRIORITY_UNIT2,
    SYS_SUB_TASK_PRIORITY_UNIT3,
 

    /*system*/
    SYS_TASK_PRIORITY,
    SYSBACK_TASK_PRIORITY,
    SYSBACK_RF_TASK_PRIORITY,
    SYSBACK_LOW_TASK_PRIORITY,

    /*UI */
    UI_SUB_TASK_PRIORITY,


#if (HW_BOARD_OPTION==MR6730_AFN)	
	/*UI Application*/
	UI_APP_TASK_PRIORITY,
#endif 


    MAIN_TASK_PRIORITY_END
};

#define IIS_TASK_PRIORITY               IIS_TASK_PRIORITY_UNIT0

#define MJPG_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define MPEG4_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define VIDEO_TASK_PRIORITY             MPEG4_TASK_PRIORITY_UNIT3

#define UARTCMD_TASK2_PRIORITY          UARTCMD_TASK1_PRIORITY
#define UARTCMD_TASK3_PRIORITY          UARTCMD_TASK1_PRIORITY
#define NIC_TASK_PRIORITY               T_ETHERNETIF_INPUT_PRIO
#define DISPLAY_LOGO_PRIORITY           0

    /*no use, for build code */
#define DEBUG_TASK_PRIORITY             64
#define USB_MSC_TASK_PRIORITY           64
#define USB_VC_TASK_PRIORITY            64
#define USB_PERF_TASK_PRIORITY          64
#define USB_TASK_PRIORITY               64
#define RTC_TASK_PRIORITY               64
#define RFIU_WRAP_TASK_PRIORITY_UNIT2   64
#define RFIU_WRAP_TASK_PRIORITY_UNIT3   64
#if (NIC_SUPPORT == 0)
#define T_LWIPENTRY_PRIOR               64
#define TCPIP_THREAD_PRIO_t             64
#define T_ETHERNETIF_INPUT_PRIO         64
#define WEB_LISTEN_TASK_PRIORITY        64
#define WEB_TASK_PRIORITY               64
#define WEB_TASK2_PRIORITY              64
#define WEB_TASK3_PRIORITY              64
#define WEB_TASK4_PRIORITY              64
#define WEB_LISTEN_SSL_TASK_PRIORITY    64
#define IOTC_ROUTINE_TASK_PRIORITY      64
#define IOTC_LOGIN_TASK_PRIORITY        64
#define LISTEN_TASK_PRIORITY            64
#define LOGIN_TASK_PRIORITY             64
#define SESSION_TASK_PRIORITY           64
#define WRAP_RTP_TASK_PRIORITY          64
#define STREAMING_RTP_TASK_PRIORITY     64
#define TELNET_THREAD_PRIO_t            64
#endif

#elif ( (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_CIU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_DIU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_IDU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_SIU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFU)  || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_H264)|| (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_PIP) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
enum
{
    RFIU_TASK_PRIORITY_HIGH = 10,
    MAIN_TASK_PRIORITY_START,
#if IIS_TEST   
    IIS_CH1_TASK_PRIORITY,
    IIS_CH0_TASK_PRIORITY,
#endif

    TIMER_TICK_TASK_PRIORITY,
    /*RF*/
    RFIU_TASK_PRIORITY_UNIT1,
    RFIU_TASK_PRIORITY_UNIT0,
    RFIU_TASK_PRIORITY_UNIT2,
    RFIU_TASK_PRIORITY_UNIT3,

    SYSTIMER_TASK_PRIORITY,

    /*IIS*/
    IIS_TASK_PRIORITY_UNIT0,
    IIS_TASK_PRIORITY_UNIT1,
    IIS_TASK_PRIORITY_UNIT2,
    IIS_TASK_PRIORITY_UNIT3,
#if AUDIO_IN_TO_OUT
    IIS_PLAYBACK_TASK_PRIORITY,
#endif

    /*RF*/
    RFIU_DEC_TASK_PRIORITY_UNIT0,
    RFIU_DEC_TASK_PRIORITY_UNIT1,
    RFIU_DEC_TASK_PRIORITY_UNIT2,
    RFIU_DEC_TASK_PRIORITY_UNIT3,

    /*mpeg4*/
    MPEG4_TASK_PRIORITY_UNIT0,
    MPEG4_TASK_PRIORITY_UNIT1,
    MPEG4_TASK_PRIORITY_UNIT2,
    MPEG4_TASK_PRIORITY_UNIT3,

    /*USB*/
    USB_HOST_MSC_TASK_PRIORITY,

    UI_TASK_PRIORITY,
    UARTCMD_TASK1_PRIORITY,
    UARTCMD_TASK2_PRIORITY,
    UARTCMD_TASK3_PRIORITY,

    /*preview*/
    SIU_TASK_PRIORITY_PVW,
    CIU_TASK_PRIORITY_CH1,
    CIU_TASK_PRIORITY_CH2,
    CIU_TASK_PRIORITY_CH3,
    CIU_TASK_PRIORITY_CH4,
    /*RF*/
    RFIU_WRAP_TASK_PRIORITY_UNIT0,
    RFIU_WRAP_TASK_PRIORITY_UNIT1,
    RFIU_WRAP_TASK_PRIORITY_UNIT2,
    RFIU_WRAP_TASK_PRIORITY_UNIT3,

    /*asf packer*/
    SYS_SUB_TASK_PRIORITY_UNIT0,
    SYS_SUB_TASK_PRIORITY_UNIT1,
    SYS_SUB_TASK_PRIORITY_UNIT2,
    SYS_SUB_TASK_PRIORITY_UNIT3,
     

    /*system*/
    SYS_TASK_PRIORITY,
    SYSBACK_TASK_PRIORITY,
    SYSBACK_RF_TASK_PRIORITY,
    SYSBACK_LOW_TASK_PRIORITY,

    /*UI */
    UI_SUB_TASK_PRIORITY,

    MAIN_TASK_PRIORITY_END
};

#define IIS_TASK_PRIORITY               IIS_TASK_PRIORITY_UNIT0

#define MJPG_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define MPEG4_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define VIDEO_TASK_PRIORITY             MPEG4_TASK_PRIORITY_UNIT3
#define DISPLAY_LOGO_PRIORITY           RFIU_TASK_PRIORITY_HIGH

    /*no use, for build code */
#define DEBUG_TASK_PRIORITY             64
#define USB_MSC_TASK_PRIORITY           64
#define USB_VC_TASK_PRIORITY            64
#define USB_PERF_TASK_PRIORITY          64
#define USB_TASK_PRIORITY               64
#define RTC_TASK_PRIORITY               64
#define NIC_TASK_PRIORITY               64
#define T_LWIPENTRY_PRIOR               64
#define T_LWIP_THREAD_START_PRIO        64
#define STREAMING_RTP_TASK_PRIORITY     64
#define WRAP_RTP_TASK_PRIORITY          64
#define	T_LWIP_THREAD_STKSIZE           1024
#define	T_LWIP_THREAD_MAX_NB	        4
#define T_ETHERNETIF_INPUT_PRIO         64
#define LISTEN_TASK_PRIORITY            64
#define SESSION_TASK_PRIORITY           64
#define LOGIN_TASK_PRIORITY             64
#elif (SW_APPLICATION_OPTION == MR8211_IPCAM)

enum
{
    RFIU_TASK_PRIORITY_HIGH = 4,
    MAIN_TASK_PRIORITY_START,
#if IIS_TEST   
    IIS_CH1_TASK_PRIORITY,
    IIS_CH0_TASK_PRIORITY,
#endif
    
    TIMER_TICK_TASK_PRIORITY,
    SYSTIMER_TASK_PRIORITY,

    /*IIS*/
    IIS_TASK_PRIORITY_UNIT0,
    IIS_TASK_PRIORITY_UNIT1,
    IIS_TASK_PRIORITY_UNIT2,
    IIS_TASK_PRIORITY_UNIT3,
#if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
    IIS_PLAYBACK_TASK_PRIORITY,
#endif

    /*network*/
    T_LWIP_THREAD_START_PRIO,
#if TUTK_SUPPORT
    IOTC_ROUTINE_TASK_PRIORITY,
    IOTC_TRANSMISSION_TASK_PRIORITY,
#endif
    TCPIP_THREAD_PRIO_t,
    T_ETHERNETIF_INPUT_PRIO,

    /*mpeg4*/
    MPEG4_TASK_PRIORITY_UNIT0,
    MPEG4_TASK_PRIORITY_UNIT1,
    MPEG4_TASK_PRIORITY_UNIT2,
    MPEG4_TASK_PRIORITY_UNIT3,

    USB_TASK_PRIORITY,
    USB_DEVICE_TASK_PRIORITY,
    USB_PERF_TASK_PRIORITY,

    /*USB*/
#if CDVR_LOG
    LOGSTRING_TASK_PRIORITY,
#endif
    UI_TASK_PRIORITY,
    UARTCMD_TASK3_PRIORITY,
    UARTCMD_TASK2_PRIORITY,
    UARTCMD_TASK1_PRIORITY,

    /*RTP*/
    WRAP_RTP_TASK_PRIORITY,
    STREAMING_RTP_TASK_PRIORITY,
    
    SIU_TASK_PRIORITY_PVW,
    CIU_TASK_PRIORITY_CH1,
    CIU_TASK_PRIORITY_CH2,
    CIU_TASK_PRIORITY_CH3,

#if TUTK_SUPPORT
    //aher 
    IOTC_LOGIN_TASK_PRIORITY,
    LISTEN_TASK_PRIORITY,
    LOGIN_TASK_PRIORITY,
#endif
    /*Change Video record priority by aher 20130107*/
    /*asf*/
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    SYS_SUB_TASK_PRIORITY_UNIT0,
    SYS_SUB_TASK_PRIORITY_UNIT1,
    SYS_SUB_TASK_PRIORITY_UNIT2,
    SYS_SUB_TASK_PRIORITY_UNIT3,
#endif

#if TUTK_SUPPORT
 	SPEAKER_TASK_PRIORITY,
    P2P_PLAYFILE_TASK_PRIORITY,
    SESSION_TASK_PRIORITY,
#endif
    
    /*system*/
    SYS_TASK_PRIORITY,
    SYSBACK_TASK_PRIORITY,
    SYSBACK_RF_TASK_PRIORITY,
  #if (NIC_SUPPORT == 1)
    SYSBACK_NET_TASK_PRIORITY,
  #endif
    SYSBACK_LOW_TASK_PRIORITY,

    /*UI */
    UI_SUB_TASK_PRIORITY,
    
    MAIN_TASK_PRIORITY_END
};

#define IIS_TASK_PRIORITY               IIS_TASK_PRIORITY_UNIT0
#define T_LWIPENTRY_PRIOR               TCPIP_THREAD_PRIO_t
#define NIC_TASK_PRIORITY               T_ETHERNETIF_INPUT_PRIO
#define DISPLAY_LOGO_PRIORITY           RFIU_TASK_PRIORITY_HIGH

/*mpeg4*/
//aher
#define	USB_DEVICE_MSC_TASK_PRIORITY	USB_TASK_PRIORITY
#define	USB_HOST_TASK_PRIORITY	        USB_TASK_PRIORITY
#define	USB_VC_TASK_PRIORITY            USB_DEVICE_TASK_PRIORITY
#define	USB_MSC_TASK_PRIORITY	        USB_DEVICE_TASK_PRIORITY
#define	USB_HOST_MSC_TASK_PRIORITY	    USB_DEVICE_TASK_PRIORITY

#define	T_LWIP_THREAD_MAX_NB 		    2
#define	T_LWIP_THREAD_STKSIZE	        1024 	
#define	T_LWIPENTRY_STKSIZE		        2048

#define MJPG_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define MPEG4_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define VIDEO_TASK_PRIORITY             MPEG4_TASK_PRIORITY_UNIT3

/*no use, for build code */
//aher
#define DEBUG_TASK_PRIORITY             64
#define USB_PERF_TASK_PRIORITY          64
#define USB_TASK_PRIORITY               64
#define RTC_TASK_PRIORITY               64
//aher

/*RF*/
#define RFIU_WRAP_TASK_PRIORITY_UNIT0   64
#define RFIU_WRAP_TASK_PRIORITY_UNIT1   64
#define RFIU_WRAP_TASK_PRIORITY_UNIT2   64
#define RFIU_WRAP_TASK_PRIORITY_UNIT3   64

/*
#define SIU_TASK_PRIORITY_PVW           64
#define CIU_TASK_PRIORITY_CH1           64
#define CIU_TASK_PRIORITY_CH2           64
#define CIU_TASK_PRIORITY_CH3           64
*/

#elif (SW_APPLICATION_OPTION == Standalone_Test)
enum
{
    RFIU_TASK_PRIORITY_HIGH = 10,
    MAIN_TASK_PRIORITY_START,
#if IIS_TEST   
    IIS_CH1_TASK_PRIORITY,
    IIS_CH0_TASK_PRIORITY,
#endif

    TIMER_TICK_TASK_PRIORITY,
    /*RF*/
    RFIU_TASK_PRIORITY_UNIT1,
    RFIU_TASK_PRIORITY_UNIT0,
    RFIU_TASK_PRIORITY_UNIT2,
    RFIU_TASK_PRIORITY_UNIT3,

    SYSTIMER_TASK_PRIORITY,

    /*IIS*/
    IIS_TASK_PRIORITY_UNIT0,
    IIS_TASK_PRIORITY_UNIT1,
    IIS_TASK_PRIORITY_UNIT2,
    IIS_TASK_PRIORITY_UNIT3,
#if AUDIO_IN_TO_OUT
    IIS_PLAYBACK_TASK_PRIORITY,
#endif

    /*RF*/
    RFIU_DEC_TASK_PRIORITY_UNIT0,
    RFIU_DEC_TASK_PRIORITY_UNIT1,
    RFIU_DEC_TASK_PRIORITY_UNIT2,
    RFIU_DEC_TASK_PRIORITY_UNIT3,

    /*mpeg4*/
    MPEG4_TASK_PRIORITY_UNIT0,
    MPEG4_TASK_PRIORITY_UNIT1,
    MPEG4_TASK_PRIORITY_UNIT2,
    MPEG4_TASK_PRIORITY_UNIT3,

    /*USB*/
    USB_HOST_MSC_TASK_PRIORITY,

    UI_TASK_PRIORITY,
    UARTCMD_TASK1_PRIORITY,
    UARTCMD_TASK2_PRIORITY,
    UARTCMD_TASK3_PRIORITY,

    /*preview*/
    SIU_TASK_PRIORITY_PVW,
    CIU_TASK_PRIORITY_CH1,
    CIU_TASK_PRIORITY_CH2,
    CIU_TASK_PRIORITY_CH3,

    /*RF*/
    RFIU_WRAP_TASK_PRIORITY_UNIT0,
    RFIU_WRAP_TASK_PRIORITY_UNIT1,
    RFIU_WRAP_TASK_PRIORITY_UNIT2,
    RFIU_WRAP_TASK_PRIORITY_UNIT3,

    /*asf packer*/
    SYS_SUB_TASK_PRIORITY_UNIT0,
    SYS_SUB_TASK_PRIORITY_UNIT1,
    SYS_SUB_TASK_PRIORITY_UNIT2,
    SYS_SUB_TASK_PRIORITY_UNIT3,
     

    /*system*/
    SYS_TASK_PRIORITY,
    SYSBACK_TASK_PRIORITY,
    SYSBACK_RF_TASK_PRIORITY,
    SYSBACK_LOW_TASK_PRIORITY,

    /*UI */
    UI_SUB_TASK_PRIORITY,

    MAIN_TASK_PRIORITY_END
};

#define IIS_TASK_PRIORITY               IIS_TASK_PRIORITY_UNIT0

#define MJPG_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define MPEG4_TASK_PRIORITY		        MPEG4_TASK_PRIORITY_UNIT3
#define VIDEO_TASK_PRIORITY             MPEG4_TASK_PRIORITY_UNIT3
#define DISPLAY_LOGO_PRIORITY           RFIU_TASK_PRIORITY_HIGH

    /*no use, for build code */
#define DEBUG_TASK_PRIORITY             64
#define USB_MSC_TASK_PRIORITY           64
#define USB_VC_TASK_PRIORITY            64
#define USB_PERF_TASK_PRIORITY          64
#define USB_TASK_PRIORITY               64
#define RTC_TASK_PRIORITY               64
#define NIC_TASK_PRIORITY               64
#define T_LWIPENTRY_PRIOR               64
#define T_LWIP_THREAD_START_PRIO        64
#define STREAMING_RTP_TASK_PRIORITY     64
#define WRAP_RTP_TASK_PRIORITY          64
#define	T_LWIP_THREAD_STKSIZE           1024
#define	T_LWIP_THREAD_MAX_NB	        4
#define T_ETHERNETIF_INPUT_PRIO         64
#define LISTEN_TASK_PRIORITY            64
#define SESSION_TASK_PRIORITY           64
#define LOGIN_TASK_PRIORITY             64
#else

#endif

   #ifndef	__DEBUG__
     #define	__DEBUG__					0
   #endif 

#endif


