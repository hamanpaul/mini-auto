/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	p2pserver_api.h

Abstract:

   	The application interface of P2P server.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2013/08/30	AHER Created

*/

#ifndef __P2PSERVER_API_H__
#define  __P2PSERVER_API_H__

#if CLOUD_SUPPORT
#define TARGET_DATA		0
#define TARGET_EVENT	1
#endif

extern u8	JOIN_DEFAULT_SSID[32];
extern u8	JOIN_DEFAULT_PSK[64];

extern u8	JOIN_DEFAULT_SSID_CHECK[32];
extern u8	JOIN_DEFAULT_PSK_CHECK[64];

/*Event Define*/
#define EVENT_MOTIONDECT    0
#define EVENT_LOWBATTERY    1
#define EVENT_PIR			2
#define EVENT_DOOR			3
#define EVENT_SIREN			4
#define EVENT_SOS			5
#define EVENT_DOOR_BELL     6


/*NTP Settings*/
#define NTP_SWITCH_ON	1
#define NTP_SWITCH_OFF	0

/*Network status*/
#define NET_LINK_ON	0
#define NET_LINK_OFF 1

/*Data struct*/
typedef struct search_time{
	unsigned short YMD;
	unsigned short HMS;
};
/*Check firmware version.*/
#define Check_by_net	1
#define Check_by_local	0

/*P2P connection status*/
#define P2P_STATUS_OK 0x07

/*The status when executing firmware downloading*/
#define FW_UPGRADE_COMPLETE 0
#define	FW_DOWNLOADING 1
#define FW_DOWNLOAD_FINISH 2
#define FW_DOWNLOAD_FAIL 3

extern u8 Reminder_FW_Upgrade; /*The latest F/W is ready on the server.*/

/* Function prototype */
extern int StreamingCH_by_APP(); /*Return which channel was streaming by APP.*/
extern void Check_P2P_info(int *p2p_info);/*if p2p_info equals P2P_STATUS_OK(0x07), means the Device have connected to the P2P server.*/
extern void P2PSendEvent(u32 camidx,u32 eventType);
/* Below only for MR8211.
 event_type = 1    --- Motion Dectection
 event_type = 2    --- Free
 event_type = 3    --- 溫度過高 
 event_type = 4    --- 溫度過低
 event_type = 5    --- 濕度過高 
 event_type = 6    --- 濕度過低 
 event_type = 7    --- Noise
 event_type = 8    --- Free
 event_type = 9    --- Online update start 
 event_type = 10   --- Online update finish
*/
extern void LoadP2PPassword(char *password);
extern void SetPushMsgDelay(int delay); // Minutes.
extern void ClearP2PConnection();
extern int Get_network_status();
extern void Load_timezone_des(char *des);
extern u8 Check_fw_ver_net(u8 connected); /*Check the latest firmware version on the server. Return 1 means new F/W has released, else return 0*/
extern void Upgrade_fw_net();/*Upgrade firmware via internet.*/
extern void NTP_Switch(int OnOff);/*Enable or disable get NTP time,ENABLE=1,DISABLE=0).*/
extern void ntpdate();/*Get current time from NTP server.*/
extern u8 GetFWDownProgress(s32 *fw_length);/*Return the progress of firmware downloading */
extern int DN2IP(char * URL,char * realIP); /* Resolve the domain name to IP address */

#endif

