/*
 * AVIOCTRLDEFs.h
 *	Define AVIOCTRL Message Type and Context
 *  Created on: 2011-08-12
 *  Author: TUTK
 *
 */

//Change Log:
//
//  2013-03-10 - 1> Add flow information collection mechanism.
//						Add IOTYPE_USER_IPCAM_GET_FLOWINFO_REQ
//							Device request client to collect flow information.
//						Add IOTYPE_USER_IPCAM_GET_FLOWINFO_RESP
//							Client acknowledge device that request is received.
//						Add IOTYPE_USER_IPCAM_CURRENT_FLOWINFO
//							Client send collected flow information to device.
//				 2> Add IOTYPE_USER_IPCAM_RECEIVE_FIRST_IFRAME command.
//
//	2013-02-19 - 1> Add more detail of status of SWifiAp
//				 2> Add more detail description of STimeDay
//
//	2012-10-26 - 1> SMsgAVIoctrlGetEventConfig
//						Add field: externIoOutIndex, externIoInIndex
//				 2> SMsgAVIoctrlSetEventConfig, SMsgAVIoctrlGetEventCfgResp
//						Add field: externIoOutStatus, externIoInStatus
//
//	2012-10-19 - 1> SMsgAVIoctrlGetWifiResp: -->SMsgAVIoctrlGetWifiResp2
//						Add status description
//				 2> SWifiAp:
//				 		Add status 4: selected but not connected
//				 3> WI-FI Password 32bit Change to 64bit
//				 4> ENUM_AP_ENCTYPE: Add following encryption types
//				 		AVIOTC_WIFIAPENC_WPA_PSK_TKIP		= 0x07,
//						AVIOTC_WIFIAPENC_WPA_PSK_AES		= 0x08,
//						AVIOTC_WIFIAPENC_WPA2_PSK_TKIP		= 0x09,
//						AVIOTC_WIFIAPENC_WPA2_PSK_AES		= 0x0A,
//
//				 5> IOTYPE_USER_IPCAM_SETWIFI_REQ_2:
//						Add struct SMsgAVIoctrlSetWifiReq2
//				 6> IOTYPE_USER_IPCAM_GETWIFI_RESP_2:
//						Add struct SMsgAVIoctrlGetWifiResp2

//  2012-07-18 - added: IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ, IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP
//	2012-05-29 - IOTYPE_USER_IPCAM_DEVINFO_RESP: Modify firmware version
//	2012-05-24 - SAvEvent: Add result type
//

#ifndef _AVIOCTRL_DEFINE_H_
#define _AVIOCTRL_DEFINE_H_

/////////////////////////////////////////////////////////////////////////////////
/////////////////// Message Type Define//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// AVIOCTRL Message Type
typedef enum 
{
	IOTYPE_USER_IPCAM_START 					= 0x01FF,
	IOTYPE_USER_IPCAM_STOP	 					= 0x02FF,
	IOTYPE_USER_IPCAM_AUDIOSTART 				= 0x0300,
	IOTYPE_USER_IPCAM_AUDIOSTOP 				= 0x0301,

	IOTYPE_USER_IPCAM_SPEAKERSTART 				= 0x0350,
	IOTYPE_USER_IPCAM_SPEAKERSTOP 				= 0x0351,

	IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ			= 0x0320,
	IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP		= 0x0321,
	IOTYPE_USER_IPCAM_GETSTREAMCTRL_REQ			= 0x0322,
	IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP		= 0x0323,

	IOTYPE_USER_IPCAM_SETMOTIONDETECT_REQ		= 0x0324,
	IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP		= 0x0325,
	IOTYPE_USER_IPCAM_GETMOTIONDETECT_REQ		= 0x0326,
	IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP		= 0x0327,
	
	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_REQ		= 0x0328,	// Get Support Stream
	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_RESP		= 0x0329,	

	IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ		= 0x032A,
	IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP	= 0x032B,	
    
	IOTYPE_USER_IPCAM_DEVINFO_REQ				= 0x0330,
	IOTYPE_USER_IPCAM_DEVINFO_RESP				= 0x0331,

	IOTYPE_USER_IPCAM_SETPASSWORD_REQ			= 0x0332,
	IOTYPE_USER_IPCAM_SETPASSWORD_RESP			= 0x0333,

	IOTYPE_USER_IPCAM_LISTWIFIAP_REQ			= 0x0340,
	IOTYPE_USER_IPCAM_LISTWIFIAP_RESP			= 0x0341,
	IOTYPE_USER_IPCAM_SETWIFI_REQ				= 0x0342,
	IOTYPE_USER_IPCAM_SETWIFI_RESP				= 0x0343,
	IOTYPE_USER_IPCAM_GETWIFI_REQ				= 0x0344,
	IOTYPE_USER_IPCAM_GETWIFI_RESP				= 0x0345,
	IOTYPE_USER_IPCAM_SETWIFI_REQ_2				= 0x0346,
	IOTYPE_USER_IPCAM_GETWIFI_RESP_2			= 0x0347,

	IOTYPE_USER_IPCAM_SETRECORD_REQ				= 0x0310,
	IOTYPE_USER_IPCAM_SETRECORD_RESP			= 0x0311,
	IOTYPE_USER_IPCAM_GETRECORD_REQ				= 0x0312,
	IOTYPE_USER_IPCAM_GETRECORD_RESP			= 0x0313,

	IOTYPE_USER_IPCAM_SETRCD_DURATION_REQ		= 0x0314,
	IOTYPE_USER_IPCAM_SETRCD_DURATION_RESP  	= 0x0315,
	IOTYPE_USER_IPCAM_GETRCD_DURATION_REQ		= 0x0316,
	IOTYPE_USER_IPCAM_GETRCD_DURATION_RESP  	= 0x0317,

	IOTYPE_USER_IPCAM_LISTEVENT_REQ				= 0x0318,
	IOTYPE_USER_IPCAM_LISTEVENT_RESP			= 0x0319,
	
	IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL 		= 0x031A,
	IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP 	= 0x031B,
	
	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_REQ		= 0x0400,	// Get Event Config Msg Request
	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_RESP		= 0x0401,	// Get Event Config Msg Response
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_REQ		= 0x0402,	// Set Event Config Msg req
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_RESP		= 0x0403,	// Set Event Config Msg resp

	IOTYPE_USER_IPCAM_GET_FWCHECK_REQ			= 0x0406,
	IOTYPE_USER_IPCAM_GET_FWCHECK_RESP			= 0x0407,
	IOTYPE_USER_IPCAM_SET_FWUPDATE_REQ			= 0x0408,
	IOTYPE_USER_IPCAM_SET_FWUPDATE_RESP			= 0x0409,

	IOTYPE_USER_IPCAM_SET_ENVIRONMENT_REQ		= 0x0360,
	IOTYPE_USER_IPCAM_SET_ENVIRONMENT_RESP		= 0x0361,
	IOTYPE_USER_IPCAM_GET_ENVIRONMENT_REQ		= 0x0362,
	IOTYPE_USER_IPCAM_GET_ENVIRONMENT_RESP		= 0x0363,
	
	IOTYPE_USER_IPCAM_SET_VIDEOMODE_REQ			= 0x0370,	// Set Video Flip Mode
	IOTYPE_USER_IPCAM_SET_VIDEOMODE_RESP		= 0x0371,
	IOTYPE_USER_IPCAM_GET_VIDEOMODE_REQ			= 0x0372,	// Get Video Flip Mode
	IOTYPE_USER_IPCAM_GET_VIDEOMODE_RESP		= 0x0373,
	
	IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ		= 0x0380,	// Format external storage
	IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_RESP		= 0x0381,	
	
	IOTYPE_USER_IPCAM_PTZ_COMMAND				= 0x1001,	// P2P PTZ Command Msg

	IOTYPE_USER_IPCAM_EVENT_REPORT				= 0x1FFF,	// Device Event Report Msg
	IOTYPE_USER_IPCAM_RECEIVE_FIRST_IFRAME		= 0x1002,	// Send from client, used to talk to device that															// client had received the first I frame
	
#if(HW_BOARD_OPTION == MR8211_ZINWELL)
	IOTYPE_USER_IPCAM_SET_MDREGION_REQ          = 0x0390,   // Send region coordinate of motion detection
    IOTYPE_USER_IPCAM_SET_MDREGION_RESP         = 0x0391,   // To know the region setting is success or fail
    IOTYPE_USER_IPCAM_GET_MDREGION_REQ          = 0x0392,   // Send request to get coordinates of selected regions
    IOTYPE_USER_IPCAM_GET_MDREGION_RESP         = 0x0393,   // Receive all coordinates
#else
	IOTYPE_USER_IPCAM_GET_FLOWINFO_REQ			= 0x0390,
	IOTYPE_USER_IPCAM_GET_FLOWINFO_RESP			= 0x0391,
	IOTYPE_USER_IPCAM_CURRENT_FLOWINFO			= 0x0392,

#endif
    
    IOTYPE_USER_IPCAM_GETPASSWORD_REQ           = 0x0394,   // Get camera password
    IOTYPE_USER_IPCAM_GETPASSWORD_RESP          = 0x0395,   // Get camera password

	/* Zinwell */	
	IOTYPE_ZW_PLUGCAM_SET_SDCARDMUM_REQ         = 0xFF000A00,   // Set mount/unmount SDCard request
	IOTYPE_ZW_PLUGCAM_SET_SDCARDMUM_RESP        = 0xFF000A01,   // Set mount/unmount SDCard response
	IOTYPE_ZW_PLUGCAM_GET_SDCARDMUM_REQ         = 0xFF000A02,   // Get mount/unmount SDCard request
	IOTYPE_ZW_PLUGCAM_GET_SDCARDMUM_RESP        = 0xFF000A03,   // Get mount/unmount SDCard response
   
	IOTYPE_ZW_PLUGCAM_SET_FWUPDATE_REQ          = 0xFF000A04,   // Set firmware update request
	IOTYPE_ZW_PLUGCAM_SET_FWUPDATE_RESP         = 0xFF000A05,   // Set firmware update response
	IOTYPE_ZW_PLUGCAM_GET_FWUPDATE_REQ          = 0xFF000A06,   // Get firmware update request
	IOTYPE_ZW_PLUGCAM_GET_FWUPDATE_RESP         = 0xFF000A07,   // Get firmware update response

	IOTYPE_ZW_PLUGCAM_SET_NIGHTMODE_REQ        = 0xFF000A08,
	IOTYPE_ZW_PLUGCAM_SET_NIGHTMODE_RESP          = 0xFF000A09,
	IOTYPE_ZW_PLUGCAM_GET_NIGHTMODE_REQ         = 0xFF000A0A,
	IOTYPE_ZW_PLUGCAM_GET_NIGHTMODE_RESP          = 0xFF000A0B,

	IOTYPE_ZW_PLUGCAM_SET_FREQUENCY_REQ        = 0xFF000A0C,
	IOTYPE_ZW_PLUGCAM_SET_FREQUENCY_RESP          = 0xFF000A0D,
	IOTYPE_ZW_PLUGCAM_GET_FREQUENCY_REQ         = 0xFF000A0E,
	IOTYPE_ZW_PLUGCAM_GET_FREQUENCY_RESP          = 0xFF000A0F,	

	IOTYPE_ZW_PLUGCAM_SET_LIGHTDETECTION_REQ            = 0xFF000A10,
	IOTYPE_ZW_PLUGCAM_SET_LIGHTDETECTION_RESP            = 0xFF000A11,
	IOTYPE_ZW_PLUGCAM_GET_LIGHTDETECTION_REQ            = 0xFF000A12,
	IOTYPE_ZW_PLUGCAM_GET_LIGHTDETECTION_RESP            = 0xFF000A13,

	IOTYPE_ZW_PLUGCAM_GETFWVERSION_REQ          = 0xFF000A14,
	IOTYPE_ZW_PLUGCAM_GETFWVERSION_RESP         = 0xFF000A15,
	
	IOTYPE_ZW_PLUGCAM_SETRESET_REQ              = 0xFF000A16,   // Reset & Reboot
	IOTYPE_ZW_PLUGCAM_SETRESET_RESP             = 0xFF000A17,

	IOTYPE_ZW_PLUGCAM_SETSHOWTIME_REQ           = 0xFF000A18,   // Show Time & Date
	IOTYPE_ZW_PLUGCAM_SETSHOWTIME_RESP          = 0xFF000A19,
	IOTYPE_ZW_PLUGCAM_GETSHOWTIME_REQ           = 0xFF000A1A,   // Show Time & Date
	IOTYPE_ZW_PLUGCAM_GETSHOWTIME_RESP          = 0xFF000A1B,
// thermal process,  added by Deming
	IOTYPE_ZW_PLUGCAM_GETCURRENTTEMP_REQ          = 0xFF000A20,
	IOTYPE_ZW_PLUGCAM_GETCURRENTTEMP_RESP         = 0xFF000A21,
	IOTYPE_ZW_PLUGCAM_GETTEMPHIGHMARGIN_REQ       = 0xFF000A22,
	IOTYPE_ZW_PLUGCAM_GETTEMPHIGHMARGIN_RESP      = 0xFF000A23,
	IOTYPE_ZW_PLUGCAM_GETTEMPLOWMARGIN_REQ       = 0xFF000A24,
	IOTYPE_ZW_PLUGCAM_GETTEMPLOWMARGIN_RESP      = 0xFF000A25,
	IOTYPE_ZW_PLUGCAM_SETTEMPHIGHMARGIN_REQ       = 0xFF000A26,
	IOTYPE_ZW_PLUGCAM_SETTEMPHIGHMARGIN_RESP      = 0xFF000A27,
	IOTYPE_ZW_PLUGCAM_SETTEMPLOWMARGIN_REQ       = 0xFF000A28,
	IOTYPE_ZW_PLUGCAM_SETTEMPLOWMARGIN_RESP      = 0xFF000A29,
// humility process, added by Deming
	IOTYPE_ZW_PLUGCAM_GETCURRENTHUMI_REQ          = 0xFF000A30,
	IOTYPE_ZW_PLUGCAM_GETCURRENTHUMI_RESP         = 0xFF000A31,
	IOTYPE_ZW_PLUGCAM_GETHUMIHIGHMARGIN_REQ       = 0xFF000A32,
	IOTYPE_ZW_PLUGCAM_GETHUMIHIGHMARGIN_RESP      = 0xFF000A33,
	IOTYPE_ZW_PLUGCAM_GETHUMILOWMARGIN_REQ       = 0xFF000A34,
	IOTYPE_ZW_PLUGCAM_GETHUMILOWMARGIN_RESP      = 0xFF000A35,
	IOTYPE_ZW_PLUGCAM_SETHUMIHIGHMARGIN_REQ       = 0xFF000A36,
	IOTYPE_ZW_PLUGCAM_SETHUMIHIGHMARGIN_RESP      = 0xFF000A37,
	IOTYPE_ZW_PLUGCAM_SETHUMILOWMARGIN_REQ       = 0xFF000A38,
	IOTYPE_ZW_PLUGCAM_SETHUMILOWMARGIN_RESP      = 0xFF000A39,

	IOTYPE_ZW_PLUGCAM_SET_FILERECYCLE_REQ 		= 0xFF000A40,
	IOTYPE_ZW_PLUGCAM_SET_FILERECYCLE_RESP             = 0xFF000A41,
	IOTYPE_ZW_PLUGCAM_GET_FILERECYCLE_REQ              = 0xFF000A42,
	IOTYPE_ZW_PLUGCAM_GET_FILERECYCLE_RESP             = 0xFF000A43,

// added by Deming
	IOTYPE_ZW_PLUGCAM_GET_LIGHT_REQ              = 0xFF000A44,
	IOTYPE_ZW_PLUGCAM_GET_LIGHT_RESP             = 0xFF000A45,
	IOTYPE_ZW_PLUGCAM_SET_LIGHT_REQ              = 0xFF000A46,
	IOTYPE_ZW_PLUGCAM_SET_LIGHT_RESP             = 0xFF000A47,
	IOTYPE_ZW_PLUGCAM_SET_REBOOT_REQ              = 0xFF000A48,
	IOTYPE_ZW_PLUGCAM_SET_REBOOT_RESP             = 0xFF000A49,
	IOTYPE_ZW_PLUGCAM_SET_NOISEALERT_REQ       = 0xFF000A50,
	IOTYPE_ZW_PLUGCAM_SET_NOISEALERT_RESP      = 0xFF000A51,
	IOTYPE_ZW_PLUGCAM_GET_NOISEALERT_REQ       = 0xFF000A52,
	IOTYPE_ZW_PLUGCAM_GET_NOISEALERT_RESP      = 0xFF000A53,
	IOTYPE_ZW_PLUGCAM_SET_TEMPALERT_REQ       = 0xFF000A54,
	IOTYPE_ZW_PLUGCAM_SET_TEMPALERT_RESP      = 0xFF000A55,
	IOTYPE_ZW_PLUGCAM_GET_TEMPALERT_REQ       = 0xFF000A56,
	IOTYPE_ZW_PLUGCAM_GET_TEMPALERT_RESP      = 0xFF000A57,

	IOTYPE_USER_IPCAM_GET_TIMEZONE_REQ          = 0x3A0,
	IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP         = 0x3A1,
	IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ          = 0x3B0,
	IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP         = 0x3B1,
	
	IOTYPE_USER_IPCAM_SETSOUNDDETECT_REQ		= 0x03B2,
	IOTYPE_USER_IPCAM_SETSOUNDDETECT_RESP		= 0x03B3,
	IOTYPE_USER_IPCAM_GETSOUNDDETECT_REQ		= 0x03B4,
	IOTYPE_USER_IPCAM_GETSOUNDDETECT_RESP		= 0x03B5,
	IOTYPE_USER_IPCAM_SETDECTECTDURATION_REQ		= 0x03B6,
	IOTYPE_USER_IPCAM_SETDECTECTDURATION_RESP		= 0x03B7,
	IOTYPE_USER_IPCAM_GETDECTECTDURATION_REQ		= 0x03B8,
	IOTYPE_USER_IPCAM_GETDECTECTDURATION_RESP		= 0x03B9,
	
	
	// Mars & RDI
	//Schedule setting
	IOTYPE_USER_IPCAM_GET_SCHEDULESETTING_REQ		= 0x0410,
	IOTYPE_USER_IPCAM_GET_SCHEDULESETTING_RESP		= 0x0411,	
	IOTYPE_USER_IPCAM_SET_SCHEDULESETTING_REQ		= 0x0412,	
	IOTYPE_USER_IPCAM_SET_SCHEDULESETTING_RESP		= 0x0413,
	
	//file overwriting
	IOTYPE_USER_IPCAM_GET_OVERWRITING_REQ		= 0x0414,
	IOTYPE_USER_IPCAM_GET_OVERWRITING_RESP		= 0x0415,	
	IOTYPE_USER_IPCAM_SET_OVERWRITING_REQ		= 0x0416,	
	IOTYPE_USER_IPCAM_SET_OVERWRITING_RESP		= 0x0417,
	
	//section duration
	IOTYPE_USER_IPCAM_SET_RECORDINGDURATION_REQ		= 0x0418,
	IOTYPE_USER_IPCAM_SET_RECORDINGDURATION_RESP	= 0x0419,
	IOTYPE_USER_IPCAM_GET_RECORDINGDURATION_REQ		= 0x041A,
	IOTYPE_USER_IPCAM_GET_RECORDINGDURATION_RESP	= 0x041B,
	
	//pairing
	IOTYPE_USER_IPCAM_SET_PAIRING_REQ	= 0x041C,
	IOTYPE_USER_IPCAM_SET_PAIRING_RESP	= 0x041D,
	IOTYPE_USER_IPCAM_GET_PAIRING_REQ	= 0x0420,
	IOTYPE_USER_IPCAM_GET_PAIRING_RESP	= 0x0421,

	//recording setup
	IOTYPE_USER_IPCAM_GET_RECORDINGSETUP_REQ	= 0x0422,
	IOTYPE_USER_IPCAM_GET_RECORDINGSETUP_RESP	= 0x0423,
	IOTYPE_USER_IPCAM_SET_RECORDINGSETUP_REQ	= 0x0424,
	IOTYPE_USER_IPCAM_SET_RECORDINGSETUP_RESP	= 0x0425,

	//Brightness
	IOTYPE_USER_IPCAM_SET_BRIGHTNESS_REQ	= 0x0426,
	IOTYPE_USER_IPCAM_SET_BRIGHTNESS_RESP	= 0x0427,
	IOTYPE_USER_IPCAM_GET_BRIGHTNESS_REQ	= 0x0428,
	IOTYPE_USER_IPCAM_GET_BRIGHTNESS_RESP	= 0x0429,	
	
	//Resolution
	IOTYPE_USER_IPCAM_SET_RESOLUTION_REQ	= 0x042A,
	IOTYPE_USER_IPCAM_SET_RESOLUTION_RESP	= 0x042B,
	IOTYPE_USER_IPCAM_GET_RESOLUTION_REQ	= 0x042C,
	IOTYPE_USER_IPCAM_GET_RESOLUTION_RESP	= 0x042D,	
	
	//Device Alarm
	IOTYPE_USER_IPCAM_SET_DEVICEALARM_REQ	= 0x0430,
	IOTYPE_USER_IPCAM_SET_DEVICEALARM_RESP	= 0x0431,
	IOTYPE_USER_IPCAM_GET_DEVICEALARM_REQ	= 0x0432,
	IOTYPE_USER_IPCAM_GET_DEVICEALARM_RESP	= 0x0433,
	
	//Video out
	IOTYPE_USER_IPCAM_SET_VIDEOOUT_REQ	= 0x0434,
	IOTYPE_USER_IPCAM_SET_VIDEOOUT_RESP	= 0x0435,
	IOTYPE_USER_IPCAM_GET_VIDEOOUT_REQ	= 0x0436,
	IOTYPE_USER_IPCAM_GET_VIDEOOUT_RESP	= 0x0437,
	
	//Device Show
	IOTYPE_USER_IPCAM_SET_DEVICESHOW_REQ	= 0x0438,
	IOTYPE_USER_IPCAM_SET_DEVICESHOW_RESP	= 0x0439,
	IOTYPE_USER_IPCAM_GET_DEVICESHOW_REQ	= 0x043A,
	IOTYPE_USER_IPCAM_GET_DEVICESHOW_RESP	= 0x043B,
    
    //Device Buzzer
    IOTYPE_USER_IPCAM_SET_DEVICEBUZZER_REQ  = 0x0440,
    IOTYPE_USER_IPCAM_SET_DEVICEBUZZER_RESP = 0x0441,
    IOTYPE_USER_IPCAM_GET_DEVICEBUZZER_REQ  = 0x0442,
    IOTYPE_USER_IPCAM_GET_DEVICEBUZZER_RESP = 0x0443,

    //HomeAutoMation APP Verify Version
    IOTYPE_USER_IPCAM_VERIFYVERSION_REQ     = 0x0446,
    IOTYPE_USER_IPCAM_VERIFYVERSION_RESP    = 0x0447,
        
	//Keep Alive
	IOTYPE_USER_IPCAM_KEEPALIVE_REQ 		= 0x0448,
	IOTYPE_USER_IPCAM_KEEPALIVE_RESP		= 0x0449,    
}ENUM_AVIOCTRL_MSGTYPE;

#define IOTYPE_RDI_GETLIGHTSTATUS_REQ 				0x701
#define IOTYPE_RDI_GETLIGHTSTATUS_RESP 				0x702
#define IOTYPE_RDI_SETLIGHTSTATUS_REQ 				0x703
#define IOTYPE_RDI_SETLIGHTSTATUS_RESP				0x704
#define IOTYPE_RDI_GETLIGHTCONFIG_REQ 				0x705
#define IOTYPE_RDI_GETLIGHTCONFIG_RESP 				0x706
#define IOTYPE_RDI_SETLIGHTCONFIG_REQ 				0x707
#define IOTYPE_RDI_SETLIGHTCONFIG_RESP 				0x708
#define IOTYPE_RDI_GETLIGHTCHECK_REQ 				0x709
#define IOTYPE_RDI_GETLIGHTCHECK_RESP 				0x70A


/////////////////////////////////////////////////////////////////////////////////
/////////////////// Type ENUM Define ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	AVIOCTRL_OK					= 0x00,
	AVIOCTRL_ERR				= -0x01,
	AVIOCTRL_ERR_PASSWORD		= AVIOCTRL_ERR - 0x01,
	AVIOCTRL_ERR_STREAMCTRL		= AVIOCTRL_ERR - 0x02,
	AVIOCTRL_ERR_MONTIONDETECT	= AVIOCTRL_ERR - 0x03,
	AVIOCTRL_ERR_DEVICEINFO		= AVIOCTRL_ERR - 0x04,
	AVIOCTRL_ERR_LOGIN			= AVIOCTRL_ERR - 5,
	AVIOCTRL_ERR_LISTWIFIAP		= AVIOCTRL_ERR - 6,
	AVIOCTRL_ERR_SETWIFI		= AVIOCTRL_ERR - 7,
	AVIOCTRL_ERR_GETWIFI		= AVIOCTRL_ERR - 8,
	AVIOCTRL_ERR_SETRECORD		= AVIOCTRL_ERR - 9,
	AVIOCTRL_ERR_SETRCDDURA		= AVIOCTRL_ERR - 10,
	AVIOCTRL_ERR_LISTEVENT		= AVIOCTRL_ERR - 11,
	AVIOCTRL_ERR_PLAYBACK		= AVIOCTRL_ERR - 12,

	AVIOCTRL_ERR_INVALIDCHANNEL	= AVIOCTRL_ERR - 0x20,
}ENUM_AVIOCTRL_ERROR; //APP don't use it now


// ServType, unsigned long, 32 bits, is a bit mask for function declareation
// bit value "0" means function is valid or enabled
// in contract, bit value "1" means function is invalid or disabled.
// ** for more details, see "ServiceType Definitation for AVAPIs"
// 
// Defined bits are listed below:
//----------------------------------------------
// bit		fuction
// 0		Audio in, from Device to Mobile
// 1		Audio out, from Mobile to Device 
// 2		PT function
// 3		Event List function
// 4		Play back function (require Event List function)
// 5		Wi-Fi setting function
// 6		Event Setting Function
// 7		Recording Setting function
// 8		SDCard formattable function
// 9		Video flip function
// 10		Environment mode
// 11		Multi-stream selectable
// 12		Audio out encoding format

// The original enum below is obsoleted.
typedef enum
{
	SERVTYPE_IPCAM_DWH					= 0x00,
	SERVTYPE_RAS_DWF					= 0x01,
	SERVTYPE_IOTCAM_8125				= 0x10,
	SERVTYPE_IOTCAM_8125PT				= 0x11,
	SERVTYPE_IOTCAM_8126				= 0x12,
	SERVTYPE_IOTCAM_8126PT				= 0x13,	
}ENUM_SERVICE_TYPE;

// AVIOCTRL Quality Type
typedef enum 
{
	AVIOCTRL_QUALITY_UNKNOWN			= 0x00,	
	AVIOCTRL_QUALITY_MAX				= 0x01,	// ex. 640*480, 15fps, 320kbps (or 1280x720, 5fps, 320kbps)
	AVIOCTRL_QUALITY_HIGH				= 0x02,	// ex. 640*480, 10fps, 256kbps
	AVIOCTRL_QUALITY_MIDDLE				= 0x03,	// ex. 320*240, 15fps, 256kbps
	AVIOCTRL_QUALITY_LOW				= 0x04, // ex. 320*240, 10fps, 128kbps
	AVIOCTRL_QUALITY_MIN				= 0x05,	// ex. 160*120, 10fps, 64kbps
}ENUM_QUALITY_LEVEL;


typedef enum
{
	AVIOTC_WIFIAPMODE_NULL				= 0x00,
	AVIOTC_WIFIAPMODE_MANAGED			= 0x01,
	AVIOTC_WIFIAPMODE_ADHOC				= 0x02,
}ENUM_AP_MODE;


typedef enum
{
	AVIOTC_WIFIAPENC_INVALID			= 0x00, 
	AVIOTC_WIFIAPENC_NONE				= 0x01, //
	AVIOTC_WIFIAPENC_WEP				= 0x02, //WEP, for no password
	AVIOTC_WIFIAPENC_WPA_TKIP			= 0x03, 
	AVIOTC_WIFIAPENC_WPA_AES			= 0x04, 
	AVIOTC_WIFIAPENC_WPA2_TKIP			= 0x05, 
	AVIOTC_WIFIAPENC_WPA2_AES			= 0x06,

	AVIOTC_WIFIAPENC_WPA_PSK_TKIP  = 0x07,
	AVIOTC_WIFIAPENC_WPA_PSK_AES   = 0x08,
	AVIOTC_WIFIAPENC_WPA2_PSK_TKIP = 0x09,
	AVIOTC_WIFIAPENC_WPA2_PSK_AES  = 0x0A,

}ENUM_AP_ENCTYPE;


// AVIOCTRL Event Type
typedef enum 
{
	AVIOCTRL_EVENT_ALL					= 0x00,	// all event type(general APP-->IPCamera)
	AVIOCTRL_EVENT_MOTIONDECT			= 0x01,	// motion detect start//==s==
	AVIOCTRL_EVENT_VIDEOLOST			= 0x02,	// video lost alarm
	AVIOCTRL_EVENT_IOALARM				= 0x03, // io alarmin start //---s--

	AVIOCTRL_EVENT_MOTIONPASS			= 0x04, // motion detect end  //==e==
	AVIOCTRL_EVENT_VIDEORESUME			= 0x05,	// video resume
	AVIOCTRL_EVENT_IOALARMPASS			= 0x06, // IO alarmin end   //---e--

	AVIOCTRL_EVENT_EXPT_REBOOT			= 0x10, // system exception reboot
	AVIOCTRL_EVENT_SDFAULT				= 0x11, // sd record exception
}ENUM_EVENTTYPE;

// AVIOCTRL Record Type
typedef enum
{
	AVIOTC_RECORDTYPE_OFF				= 0x00,
	AVIOTC_RECORDTYPE_FULLTIME			= 0x01,
	AVIOTC_RECORDTYPE_ALARM				= 0x02,
	AVIOTC_RECORDTYPE_MANUAL			= 0x03,
}ENUM_RECORD_TYPE;

// AVIOCTRL Play Record Command
typedef enum 
{
	AVIOCTRL_RECORD_PLAY_PAUSE			= 0x00,
	AVIOCTRL_RECORD_PLAY_STOP			= 0x01,
	AVIOCTRL_RECORD_PLAY_STEPFORWARD	= 0x02, //now, APP no use
	AVIOCTRL_RECORD_PLAY_STEPBACKWARD	= 0x03, //now, APP no use
	AVIOCTRL_RECORD_PLAY_FORWARD		= 0x04, //now, APP no use
	AVIOCTRL_RECORD_PLAY_BACKWARD		= 0x05, //now, APP no use
	AVIOCTRL_RECORD_PLAY_SEEKTIME		= 0x06, //now, APP no use
	AVIOCTRL_RECORD_PLAY_END			= 0x07,
	AVIOCTRL_RECORD_PLAY_START			= 0x10,
}ENUM_PLAYCONTROL;

// AVIOCTRL Environment Mode
typedef enum
{
	AVIOCTRL_ENVIRONMENT_INDOOR_50HZ 	= 0x00,
	AVIOCTRL_ENVIRONMENT_INDOOR_60HZ	= 0x01,
	AVIOCTRL_ENVIRONMENT_OUTDOOR		= 0x02,
	AVIOCTRL_ENVIRONMENT_NIGHT			= 0x03,	
}ENUM_ENVIRONMENT_MODE;

// AVIOCTRL Video Flip Mode
typedef enum
{
	AVIOCTRL_VIDEOMODE_NORMAL 			= 0x00,
	AVIOCTRL_VIDEOMODE_FLIP				= 0x01,
	AVIOCTRL_VIDEOMODE_MIRROR			= 0x02,
	AVIOCTRL_VIDEOMODE_FLIP_MIRROR 		= 0x03,
}ENUM_VIDEO_MODE;

/* Zinwell */
// AVIOCTRL SDCard Mode
typedef enum
{
    AVIOCTRL_SDCARD_MOUNT               = 0x00,
    AVIOCTRL_SDCARD_UNMOUNT             = 0x01,
    AVIOCTRL_SDCARD_FORMATING           = 0x02,
}ENUM_SDCARDMUM_MODE;

// AVIOCTRL Reset Mode
typedef enum
{
    AVIOCTRL_RESET                      = 0x00,
}ENUM_RESET_MODE;

// AVIOCTRL FW Update Mode
typedef enum
{
    AVIOCTRL_DOWNLOAD_OK                = 0x00,
    AVIOCTRL_CHECKSUM_DOWNLOAD_ERROR    = 0x01,
    AVIOCTRL_FWBIN_DOWNLOAD_ERROR       = 0x02,
    AVIOCTRL_CHECKSUM_ERROR             = 0x03,    
}ENUM_FWUpdate_MODE;

// AVIOCTRL Night Vision Mode
typedef enum
{
    AVIOCTRL_NIGHT_ON                = 0x00,
    AVIOCTRL_NIGHT_OFF    = 0x01,
    AVIOCTRL_NIGHT_AUTO       = 0x02,
}ENUM_NIGHT_MODE;

// AVIOCTRL Frequency Mode
typedef enum
{
	AVIOCTRL_FREQUENCY_50HZ 	= 0x00,
	AVIOCTRL_FREQUENCY_60HZ	= 0x01,	
}ENUM_FREQUENCY_MODE;

// AVIOCTRL Light Detection Mode
typedef enum
{
	AVIOCTRL_LIGHTDETECT_3_7 	= 0,
	AVIOCTRL_LIGHTDETECT_11_15 	= 1,
	AVIOCTRL_LIGHTDETECT_21_31 	= 2,
	AVIOCTRL_LIGHTDETECT_35_50 	= 3,
	AVIOCTRL_LIGHTDETECT_54_75 	= 4,
}ENUM_LIGHTDETECTION_MODE;

// AVIOCTRL PTZ Command Value
typedef enum 
{
	AVIOCTRL_PTZ_STOP					= 0,
	AVIOCTRL_PTZ_UP						= 1,
	AVIOCTRL_PTZ_DOWN					= 2,
	AVIOCTRL_PTZ_LEFT					= 3,
	AVIOCTRL_PTZ_LEFT_UP				= 4,
	AVIOCTRL_PTZ_LEFT_DOWN				= 5,
	AVIOCTRL_PTZ_RIGHT					= 6, 
	AVIOCTRL_PTZ_RIGHT_UP				= 7, 
	AVIOCTRL_PTZ_RIGHT_DOWN				= 8, 
	AVIOCTRL_PTZ_AUTO					= 9, 
	AVIOCTRL_PTZ_SET_POINT				= 10,
	AVIOCTRL_PTZ_CLEAR_POINT			= 11,
	AVIOCTRL_PTZ_GOTO_POINT				= 12,

	AVIOCTRL_PTZ_SET_MODE_START			= 13,
	AVIOCTRL_PTZ_SET_MODE_STOP			= 14,
	AVIOCTRL_PTZ_MODE_RUN				= 15,

	AVIOCTRL_PTZ_MENU_OPEN				= 16, 
	AVIOCTRL_PTZ_MENU_EXIT				= 17,
	AVIOCTRL_PTZ_MENU_ENTER				= 18,

	AVIOCTRL_PTZ_FLIP					= 19,
	AVIOCTRL_PTZ_START					= 20,

	AVIOCTRL_LENS_APERTURE_OPEN			= 21,
	AVIOCTRL_LENS_APERTURE_CLOSE		= 22,

	AVIOCTRL_LENS_ZOOM_IN				= 23, 
	AVIOCTRL_LENS_ZOOM_OUT				= 24,

	AVIOCTRL_LENS_FOCAL_NEAR			= 25,
	AVIOCTRL_LENS_FOCAL_FAR				= 26,

	AVIOCTRL_AUTO_PAN_SPEED				= 27,
	AVIOCTRL_AUTO_PAN_LIMIT				= 28,
	AVIOCTRL_AUTO_PAN_START				= 29,

	AVIOCTRL_PATTERN_START				= 30,
	AVIOCTRL_PATTERN_STOP				= 31,
	AVIOCTRL_PATTERN_RUN				= 32,

	AVIOCTRL_SET_AUX					= 33,
	AVIOCTRL_CLEAR_AUX					= 34,
	AVIOCTRL_MOTOR_RESET_POSITION		= 35,
}ENUM_PTZCMD;



/////////////////////////////////////////////////////////////////////////////
///////////////////////// Message Body Define ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
IOTYPE_USER_IPCAM_START 				= 0x01FF,
IOTYPE_USER_IPCAM_STOP	 				= 0x02FF,
IOTYPE_USER_IPCAM_AUDIOSTART 			= 0x0300,
IOTYPE_USER_IPCAM_AUDIOSTOP 			= 0x0301,
IOTYPE_USER_IPCAM_SPEAKERSTART 			= 0x0350,
IOTYPE_USER_IPCAM_SPEAKERSTOP 			= 0x0351,
** @struct SMsgAVIoctrlAVStream
*/
typedef struct
{
	unsigned int channel; // Camera Index
	unsigned char reserved[4];
} SMsgAVIoctrlAVStream;


/*
IOTYPE_USER_IPCAM_GETSTREAMCTRL_REQ		= 0x0322,
** @struct SMsgAVIoctrlGetStreamCtrlReq
*/
typedef struct
{
	unsigned int channel;	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetStreamCtrlReq;

/*
IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ		= 0x0320,
IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP	= 0x0323,
** @struct SMsgAVIoctrlSetStreamCtrlReq, SMsgAVIoctrlGetStreamCtrlResq
*/
typedef struct
{
	unsigned int  channel;	// Camera Index
	unsigned char quality;	//refer to ENUM_QUALITY_LEVEL
	unsigned char reserved[3];
} SMsgAVIoctrlSetStreamCtrlReq, SMsgAVIoctrlGetStreamCtrlResq;

/*
IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP	= 0x0321,
** @struct SMsgAVIoctrlSetStreamCtrlResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetStreamCtrlResp;


/*
IOTYPE_USER_IPCAM_GETMOTIONDETECT_REQ	= 0x0326,
IOTYPE_USER_IPCAM_GETSOUNDDETECT_REQ	= 0x03B4,
IOTYPE_USER_IPCAM_GETDECTECTDURATION_REQ = 0x03B8,
** @struct SMsgAVIoctrlGetMotionDetectReq
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetMotionDetectReq, SMsgAVIoctrlGetSoundDetectReq, SMsgAVIoctrlGetDectectDurationReq;


/*
IOTYPE_USER_IPCAM_SETMOTIONDETECT_REQ		= 0x0324,
IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP		= 0x0327,
IOTYPE_USER_IPCAM_SETSOUNDDETECT_REQ		= 0x03B2,
IOTYPE_USER_IPCAM_GETSOUNDDETECT_RESP		= 0x03B5,
** @struct SMsgAVIoctrlSetMotionDetectReq, SMsgAVIoctrlGetMotionDetectResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned int sensitivity; 	// 0(Disabled) ~ 100(MAX):
								// index		sensitivity value
								// 0			0
								// 1			25
								// 2			50
								// 3			75
								// 4			100
}SMsgAVIoctrlSetMotionDetectReq, SMsgAVIoctrlGetMotionDetectResp, SMsgAVIoctrlSetSoundDetectReq, SMsgAVIoctrlGetSoundDetectResp;


/*
IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP	= 0x0325,
IOTYPE_USER_IPCAM_SETSOUNDDETECT_RESP	= 0x03B3,
IOTYPE_USER_IPCAM_SETDECTECTDURATION_RESP	= 0x03B7,
** @struct SMsgAVIoctrlSetMotionDetectResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetMotionDetectResp, SMsgAVIoctrlSetSoundDetectResp, SMsgAVIoctrlSetDectectDurationResp;


/*
IOTYPE_USER_IPCAM_DEVINFO_REQ			= 0x0330,
** @struct SMsgAVIoctrlDeviceInfoReq
*/
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlDeviceInfoReq;


/*
IOTYPE_USER_IPCAM_DEVINFO_RESP			= 0x0331,
** @struct SMsgAVIoctrlDeviceInfo
*/
typedef struct
{
	unsigned char model[16];	// IPCam mode
	unsigned char vendor[16];	// IPCam manufacturer
	unsigned int version;		// IPCam firmware version	ex. v1.2.3.4 => 0x01020304;  v1.0.0.2 => 0x01000002
	unsigned int channel;		// Camera index
	unsigned int total;			// 0: No cards been detected or an unrecognizeable sdcard that could not be re-formatted.
								// -1: if camera detect an unrecognizable sdcard, and could be re-formatted
								// otherwise: return total space size of sdcard (MBytes)								
								
	unsigned int free;			// Free space size of sdcard (MBytes)
	unsigned char reserved[8];	// reserved
}SMsgAVIoctrlDeviceInfoResp;

/*
IOTYPE_USER_IPCAM_SETPASSWORD_REQ		= 0x0332,
** @struct SMsgAVIoctrlSetPasswdReq
*/
typedef struct
{
	char oldpasswd[32];			// The old security code
	char newpasswd[32];			// The new security code
}SMsgAVIoctrlSetPasswdReq;


/*
IOTYPE_USER_IPCAM_SETPASSWORD_RESP		= 0x0333,
** @struct SMsgAVIoctrlSetPasswdResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetPasswdResp;

/*	IOTYPE_USER_IPCAM_GETPASSWORD_REQ				= 0x0394,
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetPasswdReq;

/*	IOTYPE_USER_IPCAM_GETPASSWORD_RESP				= 0x0395,
 */
typedef struct
{
	char passwd[32];			// camera password
}SMsgAVIoctrlGetPasswdResp;


/*	IOTYPE_USER_IPCAM_LISTWIFIAP_REQ				= 0x0340,
*/
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlListWifiApReq;

typedef struct
{
	char ssid[32]; 				// WiFi ssid
	char mode;	   				// refer to ENUM_AP_MODE
	char enctype;  				// refer to ENUM_AP_ENCTYPE
	char signal;   				// signal intensity 0--100%
	char status;   				// 0 : invalid ssid or disconnected
								// 1 : connected with default gateway
								// 2 : unmatched password
								// 3 : weak signal and connected
								// 4 : selected:
								//		- password matched and
								//		- disconnected or connected but not default gateway
}SWifiAp;

/*
IOTYPE_USER_IPCAM_LISTWIFIAP_RESP		= 0x0341,
** @struct SMsgAVIoctrlListWifiApResp
*/
typedef struct
{
	unsigned int number; // MAX number: 1024(IOCtrl packet size) / 36(bytes) = 28
	SWifiAp stWifiAp[1];
}SMsgAVIoctrlListWifiApResp;

/*
IOTYPE_USER_IPCAM_SETWIFI_REQ			= 0x0342,
** @struct SMsgAVIoctrlSetWifiReq
*/
typedef struct
{
	unsigned char ssid[32];			//WiFi ssid
	unsigned char password[32];		//if exist, WiFi password
	unsigned char mode;				//refer to ENUM_AP_MODE
	unsigned char enctype;			//refer to ENUM_AP_ENCTYPE
	unsigned char reserved[10];
}SMsgAVIoctrlSetWifiReq;

//IOTYPE_USER_IPCAM_SETWIFI_REQ_2		= 0x0346,
typedef struct
{
	unsigned char ssid[32];		// WiFi ssid
	unsigned char password[64];	// if exist, WiFi password
	unsigned char mode;			// refer to ENUM_AP_MODE
	unsigned char enctype;		// refer to ENUM_AP_ENCTYPE
	unsigned char reserved[10];
}SMsgAVIoctrlSetWifiReq2;

/*
IOTYPE_USER_IPCAM_SETWIFI_RESP			= 0x0343,
** @struct SMsgAVIoctrlSetWifiResp
*/
typedef struct
{
	int result; //0: wifi connected; 1: failed to connect
	unsigned char reserved[4];
}SMsgAVIoctrlSetWifiResp;

/*
IOTYPE_USER_IPCAM_GETWIFI_REQ			= 0x0344,
** @struct SMsgAVIoctrlGetWifiReq
*/
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlGetWifiReq;

/*
IOTYPE_USER_IPCAM_GETWIFI_RESP			= 0x0345,
** @struct SMsgAVIoctrlGetWifiResp //if no wifi connected, members of SMsgAVIoctrlGetWifiResp are all 0
*/
typedef struct
{
	unsigned char ssid[32];		// WiFi ssid
	unsigned char password[32]; // WiFi password if not empty
	unsigned char mode;			// refer to ENUM_AP_MODE
	unsigned char enctype;		// refer to ENUM_AP_ENCTYPE
	unsigned char signal;		// signal intensity 0--100%
	unsigned char status;		// refer to "status" of SWifiAp
}SMsgAVIoctrlGetWifiResp;

//changed: WI-FI Password 32bit Change to 64bit 
//IOTYPE_USER_IPCAM_GETWIFI_RESP_2    = 0x0347,
typedef struct
{
 unsigned char ssid[32];	 // WiFi ssid
 unsigned char password[64]; // WiFi password if not empty
 unsigned char mode;	// refer to ENUM_AP_MODE
 unsigned char enctype; // refer to ENUM_AP_ENCTYPE
 unsigned char signal;  // signal intensity 0--100%
 unsigned char status;  // refer to "status" of SWifiAp
}SMsgAVIoctrlGetWifiResp2;

/*
IOTYPE_USER_IPCAM_GETRECORD_REQ			= 0x0312,
** @struct SMsgAVIoctrlGetRecordReq
*/
typedef struct
{
	unsigned int channel; // Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetRecordReq;

/*
IOTYPE_USER_IPCAM_SETRECORD_REQ			= 0x0310,
IOTYPE_USER_IPCAM_GETRECORD_RESP		= 0x0313,
** @struct SMsgAVIoctrlSetRecordReq, SMsgAVIoctrlGetRecordResq
*/
typedef struct
{
	unsigned int channel;		// Camera Index
	unsigned int recordType;	// Refer to ENUM_RECORD_TYPE
	unsigned char reserved[4];
}SMsgAVIoctrlSetRecordReq, SMsgAVIoctrlGetRecordResq;

/*
IOTYPE_USER_IPCAM_SETRECORD_RESP		= 0x0311,
** @struct SMsgAVIoctrlSetRecordResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetRecordResp;


/*
IOTYPE_USER_IPCAM_GETRCD_DURATION_REQ	= 0x0316,
** @struct SMsgAVIoctrlGetRcdDurationReq
*/
typedef struct
{
	unsigned int channel; // Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetRcdDurationReq;

/*
IOTYPE_USER_IPCAM_SETRCD_DURATION_REQ	= 0x0314,
IOTYPE_USER_IPCAM_GETRCD_DURATION_RESP  = 0x0317,
** @struct SMsgAVIoctrlSetRcdDurationReq, SMsgAVIoctrlGetRcdDurationResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned int presecond; 	// pre-recording (sec)
	unsigned int durasecond;	// recording (sec)
}SMsgAVIoctrlSetRcdDurationReq, SMsgAVIoctrlGetRcdDurationResp;


/*
IOTYPE_USER_IPCAM_SETRCD_DURATION_RESP  = 0x0315,
** @struct SMsgAVIoctrlSetRcdDurationResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetRcdDurationResp;


typedef struct
{
	unsigned short year;	// The number of year.
	unsigned char month;	// The number of months since January, in the range 1 to 12.
	unsigned char day;		// The day of the month, in the range 1 to 31.
	unsigned char wday;		// The number of days since Sunday, in the range 0 to 6. (Sunday = 0, Monday = 1, ...)
	unsigned char hour;     // The number of hours past midnight, in the range 0 to 23.
	unsigned char minute;   // The number of minutes after the hour, in the range 0 to 59.
	unsigned char second;   // The number of seconds after the minute, in the range 0 to 59.
}STimeDay;

/*
IOTYPE_USER_IPCAM_LISTEVENT_REQ			= 0x0318,
** @struct SMsgAVIoctrlListEventReq
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	STimeDay stStartTime; 		// Search event from ...
	STimeDay stEndTime;	  		// ... to (search event)
	unsigned char event;  		// event type, refer to ENUM_EVENTTYPE
	unsigned char status; 		// 0x00: Recording file exists, Event unreaded
								// 0x01: Recording file exists, Event readed
								// 0x02: No Recording file in the event
	unsigned char reserved[2];
}SMsgAVIoctrlListEventReq;


typedef struct
{
	STimeDay stTime;
	unsigned char event;
	unsigned char status;	// 0x00: Recording file exists, Event unreaded
							// 0x01: Recording file exists, Event readed
							// 0x02: No Recording file in the event
    unsigned char CameraCH;  // Camera Channel
	unsigned char reserved[1];
}SAvEvent;
	
/*
IOTYPE_USER_IPCAM_LISTEVENT_RESP		= 0x0319,
** @struct SMsgAVIoctrlListEventResp
*/
typedef struct
{
	unsigned int  channel;		// Camera Index
	unsigned int  total;		// Total event amount in this search session
	unsigned char index;		// package index, 0,1,2...; 
								// because avSendIOCtrl() send package up to 1024 bytes one time, you may want split search results to serveral package to send.
	unsigned char endflag;		// end flag; endFlag = 1 means this package is the last one.
	unsigned char count;		// how much events in this package
	unsigned char reserved[1];
	SAvEvent stEvent[1];		// The first memory address of the events in this package
}SMsgAVIoctrlListEventResp;

	
/*
IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL 	= 0x031A,
** @struct SMsgAVIoctrlPlayRecord
*/
typedef struct
{
	unsigned int channel;	// Camera Index
	unsigned int command;	// play record command. refer to ENUM_PLAYCONTROL
	unsigned int Param;		// command param, that the user defined
	STimeDay stTimeDay;		// Event time from ListEvent
	unsigned char reserved[4];
} SMsgAVIoctrlPlayRecord;

/*
IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP 	= 0x031B,
** @struct SMsgAVIoctrlPlayRecordResp
*/
typedef struct
{
	unsigned int command;	// Play record command. refer to ENUM_PLAYCONTROL
	unsigned int result; 	// Depends on command
							// when is AVIOCTRL_RECORD_PLAY_START:
							//	result>=0   real channel no used by device for playback
							//	result <0	error
							//			-1	playback error
							//			-2	exceed max allow client amount
	unsigned char reserved[4];
} SMsgAVIoctrlPlayRecordResp; // only for play record start command


/*
IOTYPE_USER_IPCAM_PTZ_COMMAND	= 0x1001,	// P2P Ptz Command Msg 
** @struct SMsgAVIoctrlPtzCmd
*/
typedef struct
{
	unsigned char control;	// PTZ control command, refer to ENUM_PTZCMD
	unsigned char speed;	// PTZ control speed
	unsigned char point;	// no use in APP so far. preset position, for RS485 PT
	unsigned char limit;	// no use in APP so far. 
	unsigned char aux;		// no use in APP so far. auxiliary switch, for RS485 PT
	unsigned char channel;	// camera index
	unsigned char reserve[2];
} SMsgAVIoctrlPtzCmd;

/*
IOTYPE_USER_IPCAM_EVENT_REPORT	= 0x1FFF,	// Device Event Report Msg 
*/
/** @struct SMsgAVIoctrlEvent
 */
typedef struct
{
	STimeDay stTime;
	unsigned long time; 	// UTC Time
	unsigned int  channel; 	// Camera Index
	unsigned int  event; 	// Event Type
	unsigned char reserved[4];
} SMsgAVIoctrlEvent;



#if 0

/* 	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_REQ	= 0x0400,	// Get Event Config Msg Request 
 */
/** @struct SMsgAVIoctrlGetEventConfig
 */
typedef struct
{
	unsigned int	channel; 		  //Camera Index
	unsigned char   externIoOutIndex; //extern out index: bit0->io0 bit1->io1 ... bit7->io7;=1: get this io value or not get
    unsigned char   externIoInIndex;  //extern in index: bit0->io0 bit1->io1 ... bit7->io7; =1: get this io value or not get
	char reserved[2];
} SMsgAVIoctrlGetEventConfig;
 
/*
	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_RESP	= 0x0401,	// Get Event Config Msg Response 
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_REQ	= 0x0402,	// Set Event Config Msg req 
*/
/* @struct SMsgAVIoctrlSetEventConfig
 * @struct SMsgAVIoctrlGetEventCfgResp
 */
typedef struct
{
	unsigned int    channel;        // Camera Index
	unsigned char   mail;           // enable send email
	unsigned char   ftp;            // enable ftp upload photo
	unsigned char   externIoOutStatus;   // enable extern io output //bit0->io0 bit1->io1 ... bit7->io7; 1:on; 0:off
	unsigned char   p2pPushMsg;			 // enable p2p push msg
	unsigned char   externIoInStatus;    // enable extern io input  //bit0->io0 bit1->io1 ... bit7->io7; 1:on; 0:off
	char            reserved[3];
}SMsgAVIoctrlSetEventConfig, SMsgAVIoctrlGetEventCfgResp;

/*
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_RESP	= 0x0403,	// Set Event Config Msg resp 
*/
/** @struct SMsgAVIoctrlSetEventCfgResp
 */
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned int result;	// 0: success; otherwise: failed.
}SMsgAVIoctrlSetEventCfgResp;

#endif


/*
IOTYPE_USER_IPCAM_SET_ENVIRONMENT_REQ		= 0x0360,
** @struct SMsgAVIoctrlSetEnvironmentReq
*/
typedef struct
{
	unsigned int channel;		// Camera Index
	unsigned char mode;			// refer to ENUM_ENVIRONMENT_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetEnvironmentReq;


/*
IOTYPE_USER_IPCAM_SET_ENVIRONMENT_RESP		= 0x0361,
** @struct SMsgAVIoctrlSetEnvironmentResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char result;		// 0: success; otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlSetEnvironmentResp;


/*
IOTYPE_USER_IPCAM_GET_ENVIRONMENT_REQ		= 0x0362,
** @struct SMsgAVIoctrlGetEnvironmentReq
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetEnvironmentReq;

/*
IOTYPE_USER_IPCAM_GET_ENVIRONMENT_RESP		= 0x0363,
** @struct SMsgAVIoctrlGetEnvironmentResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char mode;			// refer to ENUM_ENVIRONMENT_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlGetEnvironmentResp;


/*
IOTYPE_USER_IPCAM_SET_VIDEOMODE_REQ			= 0x0370,
** @struct SMsgAVIoctrlSetVideoModeReq
*/
typedef struct
{
	unsigned int channel;	// Camera Index
	unsigned char mode;		// refer to ENUM_VIDEO_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetVideoModeReq;


/*
IOTYPE_USER_IPCAM_SET_VIDEOMODE_RESP		= 0x0371,
** @struct SMsgAVIoctrlSetVideoModeResp
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char result;	// 0: success; otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlSetVideoModeResp;


/*
IOTYPE_USER_IPCAM_GET_VIDEOMODE_REQ			= 0x0372,
** @struct SMsgAVIoctrlGetVideoModeReq
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetVideoModeReq;


/*
IOTYPE_USER_IPCAM_GET_VIDEOMODE_RESP		= 0x0373,
** @struct SMsgAVIoctrlGetVideoModeResp
*/
typedef struct
{
	unsigned int  channel; 	// Camera Index
	unsigned char mode;		// refer to ENUM_VIDEO_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlGetVideoModeResp;


/*
/IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ			= 0x0380,
** @struct SMsgAVIoctrlFormatExtStorageReq
*/
typedef struct
{
	unsigned int storage; 	// Storage index (ex. sdcard slot = 0, internal flash = 1, ...)
	unsigned char reserved[4];
}SMsgAVIoctrlFormatExtStorageReq;


/*
IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ		= 0x0381,
** @struct SMsgAVIoctrlFormatExtStorageResp
*/
typedef struct
{
	unsigned int  storage; 	// Storage index
	unsigned char result;	// 0: success;
							// -1: format command is not supported.
							// otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlFormatExtStorageResp;


typedef struct
{
	unsigned short index;		// the stream index of camera
	unsigned short channel;		// the channel index used in AVAPIs, that is ChID in avServStart2(...,ChID)
	char reserved[4];
}SStreamDef;


/*	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_REQ			= 0x0328,
 */
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlGetSupportStreamReq;


/*	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_RESP			= 0x0329,
 */
typedef struct
{
	unsigned int number; 		// the quanity of supported audio&video stream or video stream
	SStreamDef streams[4];
}SMsgAVIoctrlGetSupportStreamResp;


/* IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ			= 0x032A, //used to speak. but once camera is connected by App, send this at once.
 */
typedef struct
{
	unsigned int channel;		// camera index
	char reserved[4];
}SMsgAVIoctrlGetAudioOutFormatReq;

/* IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP			= 0x032B,
 */
typedef struct
{
	unsigned int channel;		// camera index
	int format;					// refer to ENUM_CODECID in AVFRAMEINFO.h
	char reserved[4];
}SMsgAVIoctrlGetAudioOutFormatResp;

/* IOTYPE_USER_IPCAM_RECEIVE_FIRST_IFRAME			= 0x1002,
 */
typedef struct
{
	unsigned int channel;		// camera index
	char reserved[4];
}SMsgAVIoctrlReceiveFirstIFrame;

/* IOTYPE_USER_IPCAM_GET_FLOWINFO_REQ              = 0x390
 */
typedef struct
{
	unsigned int channel;			// camera index
	unsigned int collect_interval;	// seconds of interval to collect flow information
									// send 0 indicates stop collecting.
}SMsgAVIoctrlGetFlowInfoReq;

/* IOTYPE_USER_IPCAM_GET_FLOWINFO_RESP            = 0x391
 */
typedef struct
{
	unsigned int channel;			// camera index
	unsigned int collect_interval;	// seconds of interval client will collect flow information
}SMsgAVIoctrlGetFlowInfoResp;

/* IOTYPE_USER_IPCAM_CURRENT_FLOWINFO              = 0x392
 */
typedef struct
{
	unsigned int channel;						// camera index
	unsigned int total_frame_count;				// Total frame count in the specified interval
	unsigned int lost_incomplete_frame_count;	// Total lost and incomplete frame count in the specified interval
	unsigned int total_expected_frame_size;		// Total expected frame size from avRecvFrameData2()
	unsigned int total_actual_frame_size;		// Total actual frame size from avRecvFrameData2()
	unsigned int timestamp_ms;					// Timestamp in millisecond of this report.
	char reserved[8];
}SMsgAVIoctrlCurrentFlowInfo;

/* IOTYPE_USER_IPCAM_GET_TIMEZONE_REQ               = 0x3A0
 * IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP              = 0x3A1
 * IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ               = 0x3B0
 * IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP              = 0x3B1
 */
typedef struct
{
	int cbSize;							// the following package size in bytes, should be sizeof(SMsgAVIoctrlTimeZone)
	int nIsSupportTimeZone;
	int nGMTDiff;						// the difference between GMT in hours
	char szTimeZoneString[256];			// the timezone description string in multi-bytes char format
}SMsgAVIoctrlTimeZone;

/*
IOTYPE_USER_IPCAM_SETDECTECTDURATION_REQ		= 0x03B6,
IOTYPE_USER_IPCAM_GETDECTECTDURATION_RESP		= 0x03B9,
** @struct SMsgAVIoctrlSetDectectDurationReq, SMsgAVIoctrlGetDectectDurationResp
*/
typedef struct
{
    unsigned int channel;      // Camera Index
    unsigned int duration;     // duration in seconds (sec)
}SMsgAVIoctrlSetDectectDurationReq, SMsgAVIoctrlGetDectectDurationResp;


// Mars & RDI
/*
	IOTYPE_USER_IPCAM_GET_SCHEDULESETTING_REQ		= 0x0410,
	IOTYPE_USER_IPCAM_GET_SCHEDULESETTING_RESP		= 0x0411,	
	IOTYPE_USER_IPCAM_SET_SCHEDULESETTING_REQ		= 0x0412,	
	IOTYPE_USER_IPCAM_SET_SCHEDULESETTING_RESP		= 0x0413,
*/
typedef struct
{
    unsigned int dayOfWeek;       // 0~6 Mon=0,Tue=1,Wed=2....
    
} SMsgAVIoctrlGetScheduleSettingReq;

typedef struct
{
    unsigned int dayOfWeek;       // 0~6 Mon=0,Tue=1,Wed=2....
    unsigned int rec[24][4];      // [0~23]=0~24??[0~3]=Channel1~4 (0=‰∏çÈ?ÂΩ±„Ä?=?®Ê??ÑÂΩ±??=‰ΩçÁßª?µÊ∏¨)

} SMsgAVIoctrlGetScheduleSettingResp;

typedef struct
{
    unsigned int channel[4];      // Camera Channel
    unsigned int week[7];         // Day of week
	unsigned int hour[24];        // Hour of day
	unsigned int isSet; 		  // 0‰ª?°®‰∏çÈ?ÂΩ±„Ä?‰ª?°®?∞Â??®Ê??ÑÂΩ±??‰ª?°®?∞Â?‰ΩçÁßª?µÊ∏¨?ÑÂΩ±
} SMsgAVIoctrlSetScheduleSettingReq,SMsgAVIoctrlSetScheduleSettingResp;


/*	//file overwriting
	IOTYPE_USER_IPCAM_GET_OVERWRITING_REQ		= 0x0414,
	IOTYPE_USER_IPCAM_GET_OVERWRITING_RESP		= 0x0415,	
	IOTYPE_USER_IPCAM_SET_OVERWRITING_REQ		= 0x0416,	
	IOTYPE_USER_IPCAM_SET_OVERWRITING_RESP		= 0x0417,
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned int isOverWriting;
    //isOverWriting				0		OFF
    //							1		ON
    //?≥ÈÄÅÂÄºÁÇ∫0 ?∫‰? overwriting ???∫overwriting
}SMsgAVIoctrlGetOverWritingReq,SMsgAVIoctrlGetOverWritingResp,
SMsgAVIoctrlSetOverWritingReq,SMsgAVIoctrlSetOverWritingResp;


/*
	//section duration
	IOTYPE_USER_IPCAM_SET_RECORDINGDURATION_REQ		= 0x0418,
	IOTYPE_USER_IPCAM_SET_RECORDINGDURATION_RESP	= 0x0419,
	IOTYPE_USER_IPCAM_GET_RECORDINGDURATION_REQ		= 0x041A,
	IOTYPE_USER_IPCAM_GET_RECORDINGDURATION_RESP	= 0x041B,
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	//unsigned char recordingDuration [4];
    unsigned int recordingDuration;
    // reserved[4]		0		5
    //					1		10
    //					2		15
    // recordingDuration?≥ÈÄÅÁ??ºÁÇ∫(5/10/15)
}SMsgAVIoctrlSetRecordingDurationReq,SMsgAVIoctrlSetRecordingDurationResp,
SMsgAVIoctrlGetRecordingDurationReq,SMsgAVIoctrlGetRecordingDurationResp;


/*	//pairing
	IOTYPE_USER_IPCAM_SET_PAIRING_REQ	= 0x041C,
	IOTYPE_USER_IPCAM_SET_PAIRING_RESP	= 0x041D, //?à‰?‰øùÁ?
	IOTYPE_USER_IPCAM_GET_PAIRING_REQ	= 0x0420, //?à‰?‰øùÁ?
	IOTYPE_USER_IPCAM_GET_PAIRING_RESP	= 0x0421, //?à‰?‰øùÁ?
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned int isParing;  // ?ºÁÇ∫1
} SMsgAVIoctrlSetPairingReq;


/*
	//recording setup
	IOTYPE_USER_IPCAM_GET_RECORDINGSETUP_REQ	= 0x0422,
	IOTYPE_USER_IPCAM_GET_RECORDINGSETUP_RESP	= 0x0423,
	IOTYPE_USER_IPCAM_SET_RECORDINGSETUP_REQ	= 0x0424,
	IOTYPE_USER_IPCAM_SET_RECORDINGSETUP_RESP	= 0x0425,
*/
typedef struct
{
	unsigned int channel; 	    // Camera Index
	unsigned int isCameraOn;
    // isCameraOn		0		Off
    //                  1		On
    // ?≥ÈÄÅÂÄºÁÇ∫0 ‰∏çÂ??®È?ÂΩ±Ë®≠ÂÆ? /  1?üÁî®?ÑÂΩ±Ë®≠Â?
}SMsgAVIoctrlGetRecordingSetupReq,SMsgAVIoctrlGetRecordingSetupResp,
SMsgAVIoctrlSetRecordingSetupReq,SMsgAVIoctrlSetRecordingSetupResp;


/*
	//Brightness
	IOTYPE_USER_IPCAM_SET_BRIGHTNESS_REQ	= 0x0426,
	IOTYPE_USER_IPCAM_SET_BRIGHTNESS_RESP	= 0x0427,
	IOTYPE_USER_IPCAM_GET_BRIGHTNESS_REQ	= 0x0428,
	IOTYPE_USER_IPCAM_GET_BRIGHTNESS_RESP	= 0x0429,	
*/
typedef struct
{
	unsigned int channel; 	      // Camera Index
	unsigned int brightnessValue; // 1~6    1?∫Ê???6?∫Ê?‰∫?
    // brightnessValue	0	1
    //                  1	2
    //                  2	3
    //                  3	4
    //                  4	5
    //                  5	6
}SMsgAVIoctrlGetBrightnessReq,SMsgAVIoctrlGetBrightnessResp,
SMsgAVIoctrlSetBrightnessReq,SMsgAVIoctrlSetBrightnessResp;


/*
	//Resolution
	IOTYPE_USER_IPCAM_SET_RESOLUTION_REQ	= 0x042A,
	IOTYPE_USER_IPCAM_SET_RESOLUTION_RESP	= 0x042B,
	IOTYPE_USER_IPCAM_GET_RESOLUTION_REQ	= 0x042C,
	IOTYPE_USER_IPCAM_GET_RESOLUTION_RESP	= 0x042D,
*/
typedef struct
{
	unsigned int channel;     // Camera Index
	unsigned int resolution;  // 0 ??VGA (640*480) / 1?∫HD(1280*720)
}SMsgAVIoctrlGetResolutionReq,SMsgAVIoctrlGetResolutionResp,
SMsgAVIoctrlSetResolutionReq,SMsgAVIoctrlSetResolutionResp;


/*
	//Device Alarm
	IOTYPE_USER_IPCAM_SET_DEVICEALARM_REQ	= 0x0430,
	IOTYPE_USER_IPCAM_SET_DEVICEALARM_RESP	= 0x0431,
	IOTYPE_USER_IPCAM_GET_DEVICEALARM_REQ	= 0x0432,
	IOTYPE_USER_IPCAM_GET_DEVICEALARM_RESP	= 0x0433,
*/
typedef struct
{
	unsigned int channel;   // Camera Index
	unsigned int isDeviceAlarm;
    //isDeviceAlarm   0  off
    //                1  1?ÜÈ?
    //            ?? 99  99?ÜÈ?
    //?≥ÈÄÅÂÄ?0 ?á‰ª£Ë°?off / 1~99 ??1~99?ÜÈ?
}SMsgAVIoctrlGetDeviceAlarmReq,SMsgAVIoctrlGetDeviceAlarmResp,
SMsgAVIoctrlSetDeviceAlarmReq,SMsgAVIoctrlSetDeviceAlarmResp;


/*
    //Device Buzzer
    IOTYPE_USER_IPCAM_SET_DEVICEBUZZER_REQ  = 0x0440,
    IOTYPE_USER_IPCAM_SET_DEVICEBUZZER_RESP = 0x0441,
    IOTYPE_USER_IPCAM_GET_DEVICEBUZZER_REQ  = 0x0442,
    IOTYPE_USER_IPCAM_GET_DEVICEBUZZER_RESP = 0x0443,
*/
typedef struct
{
	unsigned int channel;   // Camera Index
	unsigned int isDeviceBuzzer;
    //isDeviceBuzzer		0		Off
    //						1		On
    //?≥ÈÄÅÂÄ?0 ?á‰ª£Ë°?Off / 1 ‰ª?°® On
}SMsgAVIoctrlGetDeviceBuzzerReq,SMsgAVIoctrlGetDeviceBuzzerResp,
SMsgAVIoctrlSetDeviceBuzzerReq,SMsgAVIoctrlSetDeviceBuzzerResp;


/*
	//Video out
	IOTYPE_USER_IPCAM_SET_VIDEOOUT_REQ	= 0x0434,
	IOTYPE_USER_IPCAM_SET_VIDEOOUT_RESP	= 0x0435,
	IOTYPE_USER_IPCAM_GET_VIDEOOUT_REQ	= 0x0436,
	IOTYPE_USER_IPCAM_GET_VIDEOOUT_RESP	= 0x0437,
*/
typedef struct
{
	unsigned int channel; 	    // Camera Index
	unsigned int videoOut;
    //videoOut          0		ÂÆ§ÂÖßÊ®°Â?(50hz)
    //                  1		ÂÆ§ÂÖßÊ®°Â?(60hz)
}SMsgAVIoctrlGetVideoOutReq,SMsgAVIoctrlGetVideoOutResp,
SMsgAVIoctrlSetVideoOutReq,SMsgAVIoctrlSetVideoOutResp;

/*
	//Device Show
	IOTYPE_USER_IPCAM_SET_DEVICESHOW_REQ	= 0x0438, //?à‰?‰øùÁ?
	IOTYPE_USER_IPCAM_SET_DEVICESHOW_RESP	= 0x0439, //?à‰?‰øùÁ?
	IOTYPE_USER_IPCAM_GET_DEVICESHOW_REQ	= 0x043A,
	IOTYPE_USER_IPCAM_GET_DEVICESHOW_RESP	= 0x043B,
*/
typedef struct
{
	unsigned int channel; 	      // Camera Index
}SMsgAVIoctrlGetDeviceShowReq;

typedef struct
{
	unsigned int channel; 	      // Camera Index
	unsigned char ipAddress[16];  // ‰æãÂ?192.168.100.100 Ôºå‰?Ë∂?5‰ΩçÂ?Ë£úÁ©∫
	unsigned char getwayAddress[16];
    unsigned char netMask[16];
    
} SMsgAVIoctrlGetDeviceShowResp;

/* Zinwell */
/* IOTYPE_ZW_PLUGCAM_SET_SDCARDMUM_REQ         = 0xFF000A00,
 */
typedef struct
{
    unsigned int storage;		// Storage index (ex. sdcard slot = 0, internal flash = 1, ...)
	unsigned char mode;			// refer to ENUM_SDCARDMUM_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetMUMSDCardReq;

/* IOTYPE_ZW_PLUGCAM_SET_SDCARDMUM_RESP             = 0xFF000A01,
 */
typedef struct
{
    unsigned int storage;		// Storage index (ex. sdcard slot = 0, internal flash = 1, ...)
	char result;	        	// 0: success; otherwise: failed
	unsigned char reserved[3];
}SMsgAVIoctrlSetMUMSDCardResp;


/* IOTYPE_ZW_PLUGCAM_GET_SDCARDMUM_REQ              = 0xFF000A02,
 */
typedef struct
{
    unsigned int storage;		// Storage index (ex. sdcard slot = 0, internal flash = 1, ...)
	unsigned char reserved[4];
}SMsgAVIoctrlGetMUMSDCardReq;

/* IOTYPE_ZW_PLUGCAM_GET_SDCARDMUM_RESP              = 0xFF000A03,
 */
typedef struct
{
    unsigned int storage;		// Storage index (ex. sdcard slot = 0, internal flash = 1, ...)
	unsigned char mode;			// refer to ENUM_SDCARDMUM_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlGetMUMSDCardResp;

/*	IOTYPE_ZW_PLUGCAM_SET_FWUPDATE_REQ          = 0xFF000A04,
 */
typedef struct
{
	unsigned int channel; // Camera index
    char url[256]; // linux image
    char url2[256]; // p2pcam.xml
    char checksum[32];
	unsigned char reserved[4];
}SMsgAVIoctrlFirmwareUpdateReq;

/*IOTYPE_ZW_PLUGCAM_SET_FWUPDATE_RESP         = 0xFF000A05,
 */
typedef struct
{
    unsigned char result;  //ENUM_FWUpdate_MODE
    unsigned char reserved[3];
}SMsgAVIoctrlFirmwareUpdateResp;

/*	IOTYPE_ZW_PLUGCAM_SET_NIGHTMODE_REQ          = 0xFF000A08,
 */
typedef struct
{
	unsigned int channel;		// Camera Index
	unsigned char mode;			// refer to ENUM_NIGHT_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetNightModeReq;


/*	IOTYPE_ZW_PLUGCAM_SET_NIGHTMODE_RESP          = 0xFF000A09,
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char result;		// 0: success; otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlSetNightModeResp;

/*	IOTYPE_ZW_PLUGCAM_GET_NIGHTMODE_REQ         = 0xFF000A0A,
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetNightModeReq;

/*	IOTYPE_ZW_PLUGCAM_GET_NIGHTMODE_RESP          = 0xFF000A0B,
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char mode;			// refer to ENUM_NIGHT_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlGetNightModeResp;

/*	IOTYPE_ZW_PLUGCAM_SET_FREQUENCY_REQ          = 0xFF000A0C,
 */
typedef struct
{
	unsigned int channel;		// Camera Index
	unsigned char mode;			// refer to ENUM_FREQUENCY_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetFrequencyReq;


/*	IOTYPE_ZW_PLUGCAM_SET_FREQUENCY_RESP          = 0xFF000A0D,
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char result;		// 0: success; otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlSetFrequencyResp;

/*	IOTYPE_ZW_PLUGCAM_GET_FREQUENCY_REQ         = 0xFF000A0E,
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetFrequencyReq;

/*	IOTYPE_ZW_PLUGCAM_GET_FREQUENCY_RESP          = 0xFF000A0F,
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char mode;			// refer to ENUM_FREQUENCY_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlGetFrequencyResp;

/*	IOTYPE_ZW_PLUGCAM_SET_LIGHTDETECTION_REQ		= 0xFF000A10,
 */
typedef struct
{
	unsigned int channel;		// Camera Index
	unsigned char mode;			// refer to ENUM_LIGHTDETECTION_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetLightDetectionReq;


/*  IOTYPE_ZW_PLUGCAM_SET_LIGHTDETECTION_RESP		= 0xFF000A11,
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char result;		// 0: success; otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlSetLightDetectionResp;

/*  IOTYPE_ZW_PLUGCAM_GET_LIGHTDETECTION_REQ		= 0xFF000A12,
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetLightDetectionReq;


/*  IOTYPE_ZW_PLUGCAM_GET_LIGHTDETECTION_RESP		= 0xFF000A13,
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char mode;			// refer to ENUM_LIGHTDETECTION_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlGetLightDetectionResp;

/* IOTYPE_ZW_PLUGCAM_GETFWVERSION_REQ          = 0xFF000A14,
  */ 
typedef struct
{
    unsigned int channel;  // Camera index
    unsigned char reserved[3];
}SMsgAVIoctrlGetFWVersionReq;
 
/* IOTYPE_ZW_PLUGCAM_GETFWVERSION_RESP         = 0xFF000A15,
*/ 
typedef struct
{
    char version[16];  // Camera FW version
    unsigned char reserved[3];
}SMsgAVIoctrlGetFWVersionResp;

/*	IOTYPE_ZW_PLUGCAM_SET_ADC_REQ					= 0xFF000009,
 */
/*typedef struct
{
	unsigned char off2on;			
	unsigned char on2off;	
	unsigned char reserved[4];
}SMsgAVIoctrlSetADCReq;*/


/*  IOTYPE_ZW_PLUGCAM_SET_ADC_RESP					= 0xFF00000A,
 */
/*typedef struct
{
	int result; 				// 0:success; otherwise: failed
	unsigned char reserved[4];
}SMsgAVIoctrlSetADCResp;*/


/*  IOTYPE_ZW_PLUGCAM_GET_ADC_REQ					= 0xFF00000B,
 */
/*typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlGetADCReq;*/


/*  IOTYPE_ZW_PLUGCAM_GET_ADC_RESP					= 0xFF00000C,
 */
/*typedef struct
{
	unsigned char off2on;
	unsigned char on2off;
	unsigned char reserved[4];
}SMsgAVIoctrlGetADCResp;*/

/* IOTYPE_ZW_PLUGCAM_SETRESET_REQ              = 0xFF000A16, 
 */

typedef struct
{
    unsigned int channel;		// Camera index
	unsigned char mode;			// refer to ENUM_RESET_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetResetReq;

/* IOTYPE_ZW_PLUGCAM_SETRESET_RESP             = 0xFF000A17,
 */

typedef struct
{
    unsigned int channel;		    // Camera index
	unsigned char result;			// 0: success; otherwise: failed
	unsigned char reserved[4];
}SMsgAVIoctrlSetResetResp;

/*	IOTYPE_ZW_PLUGCAM_SET_REBOOT_REQ              = 0xFF000A48,
	IOTYPE_ZW_PLUGCAM_SET_REBOOT_RESP             = 0xFF000A49,
*/
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char reserved[4];
}SMsgAVIoctrlSetRebootReq;
typedef struct
{
    unsigned int channel;		    // Camera index
    unsigned char result;			// 0: success; otherwise: failed
    unsigned char reserved[3];
}SMsgAVIoctrlSetRebootResp;


/* IOTYPE_ZW_PLUGCAM_SETSHOWTIME_REQ           = 0xFF000A18,
   IOTYPE_ZW_PLUGCAM_GETSHOWTIME_RESP          = 0xFF000A1B,
 */
typedef struct
{
    unsigned int channel;		// Camera index
	unsigned char mode;			// 0:Off,1:On
	unsigned char reserved[3];
}SMsgAVIoctrlSetShowTimeReq,SMsgAVIoctrlGetShowTimeResp;

/* IOTYPE_ZW_PLUGCAM_SETSHOWTIME_RESP          = 0xFF000A19,
 */
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char result;       // 0:success,otherwise:failed
	unsigned char reserved[4];
}SMsgAVIoctrlSetShowTimeResp;

/* IOTYPE_ZW_PLUGCAM_GETSHOWTIME_REQ           = 0xFF000A1A,
 */
typedef struct
{
    unsigned int channel;		// Camera index
	unsigned char reserved[3];
}SMsgAVIoctrlGetShowTimeReq;

typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char reserved[4];
}SMsgAVIoctrlGetCurrentTempReq;

typedef struct
{
    unsigned int channel;		// Camera index
    float  CurrentTemp;   // real number
    unsigned char reserved[4];
}SMsgAVIoctrlGetCurrentTempResp;

/* IOTYPE_ZW_PLUGCAM_GETTEMPHIGHMARGIN_REQ       = 0xFF000A22,
	IOTYPE_ZW_PLUGCAM_GETTEMPHIGHMARGIN_RESP      = 0xFF000A23,
*/
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char reserved[4];
}SMsgAVIoctrlGetTempHighMarginReq;

typedef struct
{
    unsigned int channel;		// Camera index
    float  CurrentTemp;   // real number
    unsigned char reserved[4];
}SMsgAVIoctrlGetTempHighMarginResp;

/*	IOTYPE_ZW_PLUGCAM_GETTEMPLOWMARGIN_REQ       = 0xFF000A24,
	IOTYPE_ZW_PLUGCAM_GETTEMPLOWMARGIN_RESP      = 0xFF000A25,
*/
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char reserved[4];
}SMsgAVIoctrlGetTempLowMarginReq;

typedef struct
{
    unsigned int channel;		// Camera index
    float  CurrentTemp;   // real number
    unsigned char reserved[4];
}SMsgAVIoctrlGetTempLowMarginResp;

/*	IOTYPE_ZW_PLUGCAM_SETTEMPHIGHMARGIN_REQ       = 0xFF000A26,
	IOTYPE_ZW_PLUGCAM_SETTEMPHIGHMARGIN_RESP      = 0xFF000A27,
*/
typedef struct
{
    unsigned int channel;		// Camera index
    float  CurrentTemp;   // real number
    unsigned char reserved[4];
}SMsgAVIoctrlSetTempHighMarginReq;

typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char reserved[4];
}SMsgAVIoctrlSetTempHighMarginResp;


/*	IOTYPE_ZW_PLUGCAM_SETTEMPLOWMARGIN_REQ       = 0xFF000A28,
	IOTYPE_ZW_PLUGCAM_SETTEMPLOWMARGIN_RESP      = 0xFF000A29,
*/
typedef struct
{
    unsigned int channel;		// Camera index
    float  CurrentTemp;   // real number
    unsigned char reserved[4];
}SMsgAVIoctrlSetTempLowMarginReq;

typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char reserved[4];
}SMsgAVIoctrlSetTempLowMarginResp;

// IOTYPE_ZW_PLUGCAM_SET_FILERECYCLE_REQ   = 0xFF000A40,
typedef struct
{
    unsigned int channel;  // Camera index
    unsigned char  Status;   // 0x01: allow recycled,  0x02:do not allow recycled 
    unsigned char reserved[3];
}SMsgAVIoctrlSetRecycleReq;

// IOTYPE_ZW_PLUGCAM_SET_FILERECYCLE_RESP             = 0xFF000A41,
typedef struct
{
    unsigned int channel;  // Camera index
    unsigned char  Status;   // 0x03: command successfully,   0x04: command failed
    unsigned char reserved[3];
}SMsgAVIoctrlSetRecycleResp;

// IOTYPE_ZW_PLUGCAM_GET_FILERECYCLE_REQ              = 0xFF000A42,
typedef struct
{
    unsigned int channel;  // Camera index
    unsigned char reserved[4];
}SMsgAVIoctrlGetRecycleReq;

// IOTYPE_ZW_PLUGCAM_GET_FILERECYCLE_RESP             = 0xFF000A43,
typedef struct
{
    unsigned int channel;  // Camera index
    unsigned char  Status;   // 0x01: allow recycled,  0x02:do not allow recycled
    unsigned char reserved[3];
}SMsgAVIoctrlGetRecycleResp;


//	IOTYPE_ZW_PLUGCAM_GET_LIGHT_REQ              = 0xFF000A44,
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char reserved[4];
}SMsgAVIoctrlGetLightReq;

//	IOTYPE_ZW_PLUGCAM_GET_LIGHT_RESP             = 0xFF000A45,
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char CurrentValueR;   // which value RED
    unsigned char CurrentValueG;   // which value GREEN
    unsigned char CurrentValueB;   // which value BLUE
    unsigned char CurrentValueL;   // which value Light bightness
    unsigned char  CurrentStatus;   // 0x01: OFF, 0x02: Random, 0x03: value assigned
    unsigned char reserved[3];
}SMsgAVIoctrlGetLightResp;

//	IOTYPE_ZW_PLUGCAM_SET_LIGHT_REQ              = 0xFF000A46,
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char CurrentValueR;   // which value RED
    unsigned char CurrentValueG;   // which value GREEN
    unsigned char CurrentValueB;   // which value BLUE
    unsigned char CurrentValueL;   // which value Light bightness
    unsigned char  CurrentStatus;   // 0x01: OFF, 0x02: Random, 0x03: value assigned
    unsigned char reserved[3];
}SMsgAVIoctrlSetLightReq;

//	IOTYPE_ZW_PLUGCAM_SET_LIGHT_RESP             = 0xFF000A47,
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char Status;   // 0x01: successfully,  0x02:failed 
    unsigned char reserved[3];
}SMsgAVIoctrlSetLightResp;

/*	IOTYPE_ZW_PLUGCAM_SET_NOISEALERT_REQ       = 0xFF000A50,
	IOTYPE_ZW_PLUGCAM_SET_NOISEALERT_RESP      = 0xFF000A51,
*/
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char Status;   // 0x01: ON,  0x02:OFF 
    unsigned char reserved[3];
}SMsgAVIoctrlSetNoiseAlertReq;
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char Status;   // 0x00: successfully,  0x01:failed 
    unsigned char reserved[3];
}SMsgAVIoctrlSetNoiseAlertResp;

/*	IOTYPE_ZW_PLUGCAM_GET_NOISEALERT_REQ       = 0xFF000A52,
	IOTYPE_ZW_PLUGCAM_GET_NOISEALERT_RESP      = 0xFF000A53,
*/
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char reserved[4];
}SMsgAVIoctrlGetNoiseAlertReq;
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char Status;   // 0x01: ON,  0x02:OFF  
    unsigned char reserved[3];
}SMsgAVIoctrlGetNoiseAlertResp;

/*	IOTYPE_ZW_PLUGCAM_SET_TEMPALERT_REQ       = 0xFF000A54,
	IOTYPE_ZW_PLUGCAM_SET_TEMPALERT_RESP      = 0xFF000A55,
*/
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char Status;   // 0x01: ON,  0x02:OFF 
    unsigned char reserved[3];
}SMsgAVIoctrlSetTempAlertReq;
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char Status;   // 0x00: successfully,  0x01:failed 
    unsigned char reserved[3];
}SMsgAVIoctrlSetTempAlertResp;

/*	IOTYPE_ZW_PLUGCAM_GET_TEMPALERT_REQ       = 0xFF000A56,
	IOTYPE_ZW_PLUGCAM_GET_TEMPALERT_RESP      = 0xFF000A57,
*/
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char reserved[4];
}SMsgAVIoctrlGetTempAlertReq;
typedef struct
{
    unsigned int channel;		// Camera index
    unsigned char Status;   // 0x01: ON,  0x02:OFF  
    unsigned char reserved[3];
}SMsgAVIoctrlGetTempAlertResp;

// ----------------------
// •H§U¨∞ Light ≥]∏m 20150917 Sean
// ----------------------

typedef struct{
	unsigned int channel;       // Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetLightStatusReq;

typedef struct{
	unsigned int channel;       // Camera Index
	int status; //0°G√ˆøO ,  1°G∂}øO
} SMsgAVIoctrlGetLightStatusResp;

typedef struct{
	unsigned int channel;       // Camera Index
	int status;//0°G√ˆøO ,  1°G∂}øO
} SMsgAVIoctrlSetLightStatusReq;

typedef struct{
	char result; //0x00 set lightStatus successfully,  or not failed
	unsigned char reserved[3];
} SMsgAVIoctrlSetLightStatusResp;

typedef struct{
	unsigned int channel;       // Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetLightConfigReq;

typedef struct{
	unsigned int  lightonHour; 		//∂}©lÆ…∂° §pÆ…
	unsigned int  lightonMinute;   //∂}©lÆ…∂° §¿ƒ¡
	unsigned int  lightoffHour; 		//√ˆ≥¨Æ…∂° §pÆ…
	unsigned int  lightoffMinute;   //√ˆ≥¨Æ…∂° §¿ƒ¡
	unsigned char repeat[7];//repeat[0]¨∞1™Ì•‹©P§È≠´Œ`°Bß_´h™Ì•‹©P§È§£≠´Œ`°F repeat[1]¨∞1™Ì   •‹©P§@≠´Œ`°Bß_´h™Ì•‹©P§@§£≠´Œ`°F•H¶π√˛±¿°C
	unsigned char reserved[1];
}STimer;

typedef struct{
	unsigned int channel;       // Camera Index
	int duration;          
	int dimmer;           
	STimer st;
	unsigned char reserved[4];
} SMsgAVIoctrlGetLightConfigResp;

typedef struct{
	unsigned int channel;       // Camera Index
	int duration;    
	int dimmer;    
	STimer st;
	unsigned char reserved[4];
}SMsgAVIoctrlSetLightConfigReq;

typedef struct{
	char result;	// 0: success; otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlSetLightConfigResp;

typedef struct{
	unsigned char channel;	// Camera Index ,255 means all
	unsigned char reserved[3];
}SMsgAVIoctrlGetLightSupportReq;

typedef struct{
	char lightsupport;	// 1: Yes; 0: No.
	unsigned char reserved[3];
}SMsgAVIoctrlGetLightSupportResp;

////////////////////////////////////////
// ----------------------
// •H§U¨∞ VerifyVersion ≥]∏m 20151208 Sean
// ----------------------
typedef struct
{
    unsigned char version[16];//app™©•ª´HÆß°F•H°u.°v§¿πj°F¶p°GIOS_1.3.0©ŒAND_1.3.0.5
} SMsgAVIoctrlVerifyVersionReq;

typedef struct
{
    unsigned int result;	// 0:Normal ; 1:UpdateAPP ; 2:UpdateFW
} SMsgAVIoctrlVerifyVersionResp;

////////////////////////////////////////
// ----------------------
// •H§U¨∞ Wifi ≥]∏m 20170913 Sean
// ----------------------
/*
IOTYPE_USER_IPCAM_SET_WIFISETTING_REQ		= 0x0450,
IOTYPE_USER_IPCAM_SET_WIFISETTING_RESP		= 0x0451,
*/
typedef struct
{
	unsigned char ssid[32]; 	// WiFi ssid
	unsigned char password[64]; // WiFi password if not empty
	unsigned char Language[4]; 	// Phone language. Refer to ISO 639 table
} SMsgAVIoctrlWifiSetSettingReq;

typedef struct
{
    unsigned int result;	// 0: success; otherwise: failed.
} SMsgAVIoctrlWifiSetSettingResp;

/*
IOTYPE_USER_IPCAM_GET_WIFISETTING_REQ		= 0x0452,
IOTYPE_USER_IPCAM_GET_WIFISETTING_RESP		= 0x0453,
*/
//typedef struct
//{
//} SMsgAVIoctrlWifiGetSettingReq;

typedef struct
{
	unsigned char ssid[32]; 	// WiFi ssid
	unsigned char password[64]; // WiFi password if not empty
} SMsgAVIoctrlWifiGetSettingResp;

////////////////////////////////////////
// ----------------------
// •H§U¨∞ Wifi Ωu§WßÛ∑s≥]∏m 20180123 Sean
// ----------------------
/*
IOTYPE_USER_IPCAM_GET_FWCHECK_REQ			= 0x0406,
IOTYPE_USER_IPCAM_GET_FWCHECK_RESP			= 0x0407,
*/

typedef struct
{
    unsigned int result;		// 0: success; otherwise: failed.
} SMsgAVIoctrlGetFWCheckResp;

/*
IOTYPE_USER_IPCAM_SET_FWUPDATE_REQ			= 0x0408,
IOTYPE_USER_IPCAM_SET_FWUPDATE_RESP 		= 0x0409,
*/

typedef struct
{
    unsigned int result;		// 0: success; otherwise: failed.
} SMsgAVIoctrlSetFWUpdateResp;



#endif
