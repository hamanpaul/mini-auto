#include "web_util.h"
//---------------------------------------------

// ------------- for TCP/IP Config ---------------------
typedef struct _WEB_CONFIG_TCPIP {

	unsigned char   macAddr[6];
	unsigned long	 ipAddr;
	unsigned long   netmask;
	unsigned long	 gateway;
	int		vid;
	char 	description[WEB_NAME_LENGTH];	//Add by Scott
	int		getIpFrom;
	int        priority;             //Added by Vic
	int    vid_Style;         //Added by Vic
} WEB_CONFIG_TCPIP;

// ------------- for Advanced Setup ---------------------
// ---------------------------------------------------------------
// CfgSwAdvSetup.html
// Switch Information
// CfgSwitchAdv_DHS3226.html ,CfgSwitchAdv_DES3226.html
// ---------------------------------------------------------------

typedef struct _WEB_CONFIG_SWADV_INFO {
	int		timeout;
	int		agingTime;
	int		igmpSnoop;
	int		gvrpState;
	int     telnetState;
	int     webState;
	int		bandwithCtrl;
	int		groupAddrMode;
	int		trunkShare;
	int		schedule;
	int     backpressure;
	int     gmrpState;
	//--Added by Vic Yu for DES7000v ,2002-09/23
	int		RMONStatus;    // RMON Status   ,2=>Disabled ,3=> Enabled
	int		SerialBaud;     // Serial Port Baud Rate  1=>9600 ,2=>19200 ,3=>38400 ,4=> 115200
	//int		MgmtSpeed;   // Management Port Speed 1=> Auto ,2=>10_half,3=>10_full,4=>100_half ,5=>100_full
	//int		MgmtFlctrl;   // Management Port Flow Control  ,2=>Disabled ,3=> Enabled
	int		MultiRtOnly;   // Multicast router Only,2=>Disabled ,3=> Enabled
	int		Tra_Segmt;   // Multicast router Only
	int		Serv_Mac;   // Multicast router Only
	int		Switch8021xstate;	// 2->disable, 3->enable
	
	int		HOLState;  // HOL state ,2->disable, 3->enable
	int		JumboState;  // Jumbo Frame state ,2->disable, 3->enable
	int		AuthProtocol;  // Authentication Protocol ,1: Local ,4: Radius Eap
	int		syslog;  // Syslog state ,2->disable, 3->enable
} WEB_CONFIG_SWADV_INFO;

// ------------- for Management Port ------------------  Added by Vic Yu 2002-10/14
typedef struct _WEB_CONFIG_SWADV_MGTPORT {
	int		mgtSpeed;      // Management Port Speed 1=> Auto ,2=>10_half,3=>10_full,4=>100_half ,5=>100_full
	int		mgtFlctrl;    // Management Port Flow Control  ,2=>Disabled ,3=> Enabled
	char		mgtstatus_Speed[255];   // Management Port Link Status ,Speed/Duplex
	char		mgtstatus_FC[255];   // Management Port Link Status ,Flow control
}WEB_CONFIG_SWADV_MGTPORT;
// ------------- for PortInformation ---------------------
typedef struct _WEB_CONFIG_PORT_INFO {
	int start;
	int end;
	int index;
	int state;
	int flowCtrl;
	int nWay;		// retrive the Port Nway Speed/Duplex
	int	type;
	char portName[32];
	char vlanName[WEB_NAME_LENGTH];
	int	 speed;
	int DHS3226connection;//11-23-2001 add
	int trunkIndex;
} WEB_CONFIG_PORT_INFO;
extern char *html_SetBuffer(char *);
extern char *dlk_GetPortConnection(int);
extern int dlk_CfgPortGetModuleType(void);

// ------------- for PortMirroring ---------------------
typedef struct _WEB_CONFIG_PORT_MIRROR {
	int srcPort;
	int srcDirection;
	int dstPort;
	int state;
} WEB_CONFIG_PORT_MIRROR;
extern void dlkGetPortMirror(WEB_CONFIG_PORT_MIRROR *,int);
extern void dlkSetPortMirror(WEB_CONFIG_PORT_MIRROR,int);
//------------------------------------------------------
