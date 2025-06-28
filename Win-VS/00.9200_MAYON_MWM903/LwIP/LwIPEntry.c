
//*-------------------------------------??????--------------------------------------------------------------------
#include "general.h"
#include "lwip/lwipsys.h"
#include "lwip/mem.h"
#include "lwip/memp.h" 
#include "lwip/pbuf.h"
#include "lwip/tcpip.h"
#include "lwip/opt.h"
#include "gpiapi.h"
#include "sysopt.h"

#if (UDP_ECHO_SERVER_SUPPORT)
#include "lwip/udpecho.h"
#elif (TCP_ECHO_SERVER_SUPPORT)
#include "lwip/tcpecho.h"
#endif
#include "uiapi.h"
#include <lwip/sockets.h>
#include "rtcapi.h"
#include "p2pserver_api.h"
#include "FTMAC110.h"
#if(NIC_SUPPORT == 1)
 //*------------------------------------?????-------------------------------------------------------------------------

s32_t spiRead(u8_t* , u32_t , u32_t);
void telnet_init(void); 
void GetNetworkInfo(struct NetworkInfo *);
void SetNetworkInfo(struct NetworkInfo *);

extern u8 SetLwIP(u8 mode);
//extern void udpecho_init(void);
//*------------------------------------ ?????????? --------------------------------------
//* IP??
#define 	IP_ADDR0			0
#define 	IP_ADDR1			0
#define 	IP_ADDR2			0
#define 	IP_ADDR3			0

//* ????
#define		GATEWAY_ADDR0 		0
#define		GATEWAY_ADDR1 		0
#define		GATEWAY_ADDR2 		0
#define		GATEWAY_ADDR3 		0

//* ????
#define		NET_MASK0			0
#define		NET_MASK1			0 
#define		NET_MASK2			0
#define		NET_MASK3			0

#define HTTP_PORT  					80
#define RTSP_PORT  					554

OS_FLAG_GRP  *gpiNetStatusFlagGrp;    /* Flag for network Status. */

/*Ethernet interface*/
//struct netif  DM9000_netif; //?????????,????????ne2k_isr????????
struct netif  FTMAC110_netif;
struct ip_addr remote_ip;

//UI Setting
u8 LwipIPAddr[4]        ={192,168,1,118};
u8 LwipSubnetMask[4]    ={255,255,255,0};
u8 LwipDefaultGateway[4]={192,168,1,1};
u8 LwipISStatic=0;     /* 0: Dynamic IP, 1: Static IP */
u8 LwipIpAddrInfo[4]={0};
u8 LwipSubMaskInfo[4]={0};
u8 LwipDefaultGatewayInfo[4]={0};
u8 Lwipredhcp=0;     /* 0:  IP, 1:  IP */
__align(4)  u8_t  __RxBufs[1024] ;			// ?????
__align(4)  u8_t  __TxBufs[1024] ;			// ?????static volatile  
//u8_t  my_hwaddr[6]= { 0x6d,0xf0,0x49,0xa2,0x66,0xF7};
//const u16_t MAC_ADDR[6]= { 0x6c,0xf0,0x49,0xa2,0x66,0x68};
const char ssst[]="welcome to ucos world!\n";
u8_t my_ipaddr[5]={IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3};
u8 qetIP=0;
int NTP_ON;
RTC_DATE_TIME gDHCPTime;
u32 DHCP_leasetime;

//========================================
#if(HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) //20171208 add.
extern u8	Reset_P2P_Connection_flag;
extern u8	Reset_P2P_Connection_Stop_flag;
extern void Reset_P2P_Connection(void);
extern u8 FW_UPGRAD_flag;
#endif

extern u8_t uiMACAddr[6];
extern int net_link_status;
extern u8 renewIP;
extern u8  uiSetTxTime[MULTI_CHANNEL_MAX];
extern RTC_TIME_ZONE ntpTimeZone;
#if APP_KEEP_ALIVE
#define KEEP_ALIVE_TIMEOUT	60
#define MAX_CLIENT			4

extern u8 APPConnectIcon;
extern int gOnlineNum;
extern int gFlagKeepAlive[];
extern RTC_DATE_TIME gKeepAliveTime0;
extern RTC_DATE_TIME gKeepAliveTime1;
extern RTC_DATE_TIME gKeepAliveTime2;
extern RTC_DATE_TIME gKeepAliveTime3;
extern RTC_DATE_TIME gKeepAliveTime4;

#endif


//========================================
extern void EMACInit(void);  
//========================================
 
/* Resolve the domain name to IP address */
int DN2IP(char * URL, char * realIP)
{
    char buf[50]="";
    INT32U pu32IP;
    int err=0;
    int count=0;
    char * tmp_IP;  
    convert(URL,buf);

	OSTaskChangePrio(T_ETHERNETIF_INPUT_PRIO,RFIU_TASK_PRIORITY_HIGH);
    err=u32DNToIP(buf,strlen(buf)+1,&pu32IP);
	if(err!=0)
		DEBUG_P2P("DNS err:%d\n",err);
	else
	{
    	tmp_IP=(char *)&pu32IP;
    	sprintf(realIP,"%d.%d.%d.%d",*tmp_IP,*(tmp_IP+1),*(tmp_IP+2),*(tmp_IP+3));
	}
	OSTaskChangePrio(RFIU_TASK_PRIORITY_HIGH,T_ETHERNETIF_INPUT_PRIO);	
    return err;
}

static int convert(char *url, char *tmp)
{
	char * delim = ".";
	char * pch;
    short tmp_1;
    
	pch = strtok(url,delim);
	
	while (pch != NULL)
 	{
		tmp_1=strlen(pch);
		strcat(tmp,(char *)&tmp_1);
		strcat(tmp,pch);
		pch = strtok (NULL, delim);
	}
	return 0;
}
 
void NTP_Switch(int OnOff)
{
    NTP_ON=OnOff;
}

/*Get time from NTP server */
s32 ntpdate(u32 dummy, u32 dummy2)
{
    char	hostname1[30]="0.pool.ntp.org";
	char    hostname2[30]="0.ca.pool.ntp.org";
	char	hostname3[30]="time.stdtime.gov.tw";
	char    hostname_IP[50]="";
    int     err;
	int	portno=123;		//NTP is port 123
	int	i;			// misc var i
	unsigned char msg[48]={010,0,0,0,0,0,0,0,0};	// the packet we send
	unsigned long  buf[1024];	// the buffer we get back
	//struct in_addr ipaddr;		
	//struct protoent *proto;		
	struct sockaddr_in server_addr;
	int	s;	// socket
	u8	noblock;
	int	tmit;	// the time -- This is a time_t sort of
	RTC_DATE_TIME NTP_time;
	RTC_DATE_TIME Local_time;
	RTC_TIME_ZONE zone;
	s8 TimeZone;
	u8	retry_cnt=0;
	
	//if((net_link_status==NET_LINK_ON)&&(NTP_ON==NTP_SWITCH_ON))
	//if(NTP_ON==NTP_SWITCH_ON)
	if(net_link_status==NET_LINK_ON)
	{

	/*4?Channel ????*/
	//MultiChannelGetCaptureVideoStatus(Ch_ID+MULTI_CHANNEL_LOCAL_MAX);/*??????*/
	//MultiChannelSysCaptureVideoStopOneCh(Ch_ID+MULTI_CHANNEL_LOCAL_MAX)/*[?? 08:38:20] elsa: ????*/
	//MultiChannelSysCaptureVideoOneCh(Ch_ID+MULTI_CHANNEL_LOCAL_MAX/*????*/
		
		//use Socket;
		//
		//#we use the system call to open a UDP socket
		//socket(SOCKET, PF_INET, SOCK_DGRAM, getprotobyname("udp")) or die "socket: $!";
		//proto=getprotobyname("udp");
Retry:		
		s=socket(PF_INET, SOCK_DGRAM, 0);
		if(s<0) 
		{
			DEBUG_P2P("Create socket fail : %d\n",s);
		}
		else
		{
			err=DN2IP(hostname1,hostname_IP); 
			if(err==0)
				DEBUG_P2P("Get NTP time: pool.ntp.org,IP = %s\n",hostname_IP);
			else
				DEBUG_P2P("DN2IP fail...\n");
			if(renewIP == 1)
			{
				DEBUG_P2P("Renew IP, leave ntpdate.\n");
				close(s);
				return -1;
			}
			noblock=1;	
			ioctlsocket(s,FIONBIO,&noblock);

			//#convert hostname to ipaddress if needed
			//$ipaddr   = inet_aton($HOSTNAME);
			memset( &server_addr, 0, sizeof( server_addr ));
			server_addr.sin_family=AF_INET;
	 
			server_addr.sin_addr.s_addr = inet_addr(hostname_IP);
			server_addr.sin_port=htons(portno);
		
			i=sendto(s,msg,sizeof(msg),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
			// get the data back
			OSTimeDly(200);
			i=recv(s,buf,sizeof(buf),0);	

	    	if(i<0)
		    {
		
			    err=DN2IP(hostname2,hostname_IP); 
		    	if(err==0)
					DEBUG_P2P("Get NTP time: ca.pool.ntp.org,IP = %s\n",hostname_IP);
			    else
	    	    	DEBUG_P2P("DN2IP fail...\n");
	    
				server_addr.sin_addr.s_addr = inet_addr(hostname_IP);
		   		OSTimeDly(10);
				i=sendto(s,msg,sizeof(msg),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
				// get the data back
				OSTimeDly(200);
				i=recv(s,buf,sizeof(buf),0);	
	    	}
			if(i<0)
			{
	    		err=DN2IP(hostname3,hostname_IP); 
		    	if(err==0)
	    	    	DEBUG_P2P("Get NTP time: time.stdtime.gov.tw,IP = %s\n",hostname_IP);
			    else
			        DEBUG_P2P("DN2IP fail...\n");
				server_addr.sin_addr.s_addr = inet_addr(hostname_IP);
			    OSTimeDly(10);
				i=sendto(s,msg,sizeof(msg),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
				// get the data back
				OSTimeDly(200);
				i=recv(s,buf,sizeof(buf),0);
			}
			DEBUG_P2P("Retrieve NTP time : %d\n",i);	
#if(((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777)&&(PROJ_OPT == 3)) || ((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777)&&(PROJ_OPT == 9))) //20170914 Sean: M936 use.
			if((retry_cnt == 0) && (i != 48))	//Retry once
			{
				printf("Retrieve NTP time Wrong, retry...\n");
				retry_cnt++;
				close(s);
				goto Retry;
			}
#endif				
			tmit=ntohl(buf[10]);	//# get transmit time
			//DEBUG_P2P("tmit=%d\n",tmit);
		
			tmit-= 2208988800U;	
			tmit-=946684800;
		
			if(i>0)
			{
#if ((AutoNTPupdate ==1) && (UI_SUPPORT_TIMEZONE_MENU == 0))
								
				TimeZone = SendGetTimeZoneMessage();	
				if(TimeZone == 100)
				{
					printf("\nNTP TimeZone Get Fail. Set to default:%d\n",Default_TimeZone);
					TimeZone = Default_TimeZone;
				}
				tmit = tmit + TimeZone*3600;
#endif
				RTC_Second_To_Time(tmit,&Local_time);
				printf("\nNTP Time: %s\n",ctime(&tmit));
				RTC_Set_GMT_Time(&Local_time);


			#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
				uiFlowCheckDST(&Local_time, 2);
				#if RFIU_SUPPORT
					for(i=0;i<MAX_RFIU_UNIT;i++)
						uiSetTxTime[i] = uiSetRfTimeRxToTx(i);
				#endif    	

			#else
				#if RFIU_SUPPORT
					for(i=0;i<MAX_RFIU_UNIT;i++)
						uiSetRfTimeRxToTx(i);
				#endif

			#endif		
#if RTC_INCREASE_CHECK
			close(s);
			return 1;
#endif
			}	
			else
			{
				DEBUG_P2P("Could'n get the NTP Time.\n");
				//sysForceWDTtoReboot(); //20150402_Sean
			}
			close(s);
		}
	}
	return -1;
}
	
  //*---------------------------------------------------------------------------------------------------------------------
 //*????:InitLwIP
 //*????:??lwip????????
 //*          ???LwIP???????PBUF?PCB??OS????????
 //*????:none
 //*????:none
 //*----------------------------------------------------------------------------------------------------------------------
  __inline void InitLwIP(void)
  {
	extern void memp_init(void);
	
	sys_init();
	
	mem_init();
	
	memp_init();
	
	pbuf_init();
	
	raw_init();
	
//	netif_init();
//	ip_init();	
	tcpip_init(NULL, NULL);
  
  }
    	
 //*-----------------------------------------------------------------------------------------------------------------------
 //*????:SetLwIP
 //*????:??LWIP,?????????????
 //*????:none
 //*????:none
 //*------------------------------------------------------------------------------------------------------------------------
 
 
 u8 SetLwIP(u8 mode)
{
	
    extern err_t ethernetif_init(struct netif * netif);
    struct ip_addr IpAddr, NetMask, GateWay;
 
	#if LWIP_DHCP
	    INT32U j;
	    INT16U i;
	    INT8S result;
	    INT16U cnt;
	    int mscnt =0;
	    INT8U err;
	 	u8_t *p;
	    u8 count=0;
	    u8 ret;
	#endif

    //extern struct netif  rtl8019if_netif;
	if(!qetIP)
	{
	    LWIP_PLATFORM_DIAG(("SetLwIP!!\r\n"));
	
	    netif_init();
	    LWIP_PLATFORM_DIAG(("S0!!\r\n"));
	    //EMACInit_reboot();
    	/* processing PHY, set full/half duplex, 10/100 Mbps, using auto-negotiation */
		change_duplex_speed(FULL, _100); 
    	LWIP_PLATFORM_DIAG(("S1!!\r\n"));

		//Static IP or Dynamic IP by aher 20121129	

	    IP4_ADDR(&IpAddr , IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
	    IP4_ADDR(&NetMask , NET_MASK0,NET_MASK1,NET_MASK2,NET_MASK3);
    	IP4_ADDR(&GateWay,  GATEWAY_ADDR0, GATEWAY_ADDR1,GATEWAY_ADDR2,GATEWAY_ADDR3);

	    netif_add(&FTMAC110_netif,&IpAddr , &NetMask ,&GateWay, NULL ,ethernetif_init, tcpip_input);
    	netif_set_default(&FTMAC110_netif);
	    netif_set_up(&FTMAC110_netif);
		qetIP=1;
//        udpecho_init();
	}
	if(!LwipISStatic)
	{
		DEBUG_P2P("DYNAMIC IP\n");
		#if LWIP_DHCP

	    //for(i=0; i<5; i++) //modified by AHER , the dhcp request will nonstop until get IP addr. 20120924
    	do
	    {
	    	//if(net_link_status==NET_LINK_ON)
	    	{
	    	    LWIP_DEBUGF(DHCP_DEBUG | DBG_TRACE | DBG_STATE, ("Start DHCP Request!!1\r\n"));
	        	LWIP_PLATFORM_DIAG(("Start DHCP Request!!\r\n"));
		        count++;
	    	    if (mode)   /*Re*/
	        	{   
	            	if (count>8)
		            {
	    	            ret=-1;
		                break;
	    	        }    
		        }   
	    	    if(count==50)
		        {    
	    	    	//EMACInit(); marked by aher
		        }	

	    	    result = dhcp_start(&FTMAC110_netif);
	        	IP4_ADDR(&IpAddr,0,0,0,0);
		        for(cnt=0; (cnt<10) && (IpAddr.addr == 0); cnt++)
	    	    {
	        	    IpAddr.addr = FTMAC110_netif.ip_addr.addr;
	    	        OSTimeDlyHMSM(0,0,1,0);
	        	}
				if(IpAddr.addr != 0)
				{
					DEBUG_P2P("DHCP:t0=%lu,t1=%lu,t2=%lu\n",FTMAC110_netif.dhcp->offered_t0_lease,FTMAC110_netif.dhcp->offered_t1_renew,FTMAC110_netif.dhcp->offered_t2_rebind);
					DHCP_leasetime=FTMAC110_netif.dhcp->offered_t1_renew;
				}	
	    	    dhcp_stop(&FTMAC110_netif);
		        if(IpAddr.addr != 0)
	    	    {
	        	    LWIP_DEBUGF(DHCP_DEBUG | DBG_TRACE | DBG_STATE, ("Start DHCP Request *** OK *** \r\n"));
					//net_link_status=0;
		            ret=0;
	    	        break;
	        	}
        	}
	        OSTimeDly(1);
    	}while (IpAddr.addr == 0);
	    LWIP_PLATFORM_DIAG(("\n=== DHCP====\r\n"));
    	IpAddr.addr = FTMAC110_netif.ip_addr.addr;
	    GateWay.addr = FTMAC110_netif.gw.addr;
    	NetMask.addr = FTMAC110_netif.netmask.addr;

	    if(1)
	    {
       		p =(u8_t *) &IpAddr.addr;
	        my_ipaddr[0]=p[0]; my_ipaddr[1]=p[1]; my_ipaddr[2]=p[2]; my_ipaddr[3]=p[3];   
			LwipIPAddr[0]=p[0];LwipIPAddr[1]=p[1];LwipIPAddr[2]=p[2];LwipIPAddr[3]=p[3];
	        OSFlagPost(gpiNetStatusFlagGrp, FLAGGPI_LWIP_IP_READY, OS_FLAG_SET, &err);
	        DEBUG_P2P("\nMY IP %u.%u.%u.%u \n",p[0],p[1],p[2],p[3]);
			p =(u8_t *) &IpAddr.addr;
			LwipIpAddrInfo[0]=p[0];LwipIpAddrInfo[1]=p[1];LwipIpAddrInfo[2]=p[2];LwipIpAddrInfo[3]=p[3];
			p =(u8_t *) &GateWay.addr;
			LwipDefaultGatewayInfo[0]=p[0];LwipDefaultGatewayInfo[1]=p[1];LwipDefaultGatewayInfo[2]=p[2];LwipDefaultGatewayInfo[3]=p[3];
			p =(u8_t *) &NetMask.addr;
			LwipSubMaskInfo[0]=p[0];LwipSubMaskInfo[1]=p[1];LwipSubMaskInfo[2]=p[2];LwipSubMaskInfo[3]=p[3];
		}   
#if (TCP_ECHO_SERVER_SUPPORT)
         LWIP_PLATFORM_DIAG(("S2-TCP!!\r\n"));
         tcpecho_init();
#elif (UDP_ECHO_SERVER_SUPPORT)
         LWIP_PLATFORM_DIAG(("S2-UDP!!\r\n"));
         udpecho_init();
#endif
	#endif 
	}
	if(LwipISStatic)
	{
		u8_t *p;

		DEBUG_P2P("STATIC IP\n");	
		#if	(TUTK_SUPPORT==1)
        	net_link_status=Get_network_status();
		#endif	
		p =(u8_t *) &IpAddr.addr;
		p[0]=LwipIPAddr[0];p[1]=LwipIPAddr[1];p[2]=LwipIPAddr[2];p[3]=LwipIPAddr[3];
		my_ipaddr[0]=p[0]; my_ipaddr[1]=p[1]; my_ipaddr[2]=p[2]; my_ipaddr[3]=p[3];
		DEBUG_P2P("\nMY IP %u.%u.%u.%u \n",p[0],p[1],p[2],p[3]);
		p =(u8_t *) &GateWay.addr;
		p[0]=LwipDefaultGateway[0];p[1]=LwipDefaultGateway[1];p[2]=LwipDefaultGateway[2];p[3]=LwipDefaultGateway[3];
		p =(u8_t *) &NetMask.addr;
		p[0]=LwipSubnetMask[0];p[1]=LwipSubnetMask[1];p[2]=LwipSubnetMask[2];p[3]=LwipSubnetMask[3];
		FTMAC110_netif.ip_addr.addr=IpAddr.addr;
		FTMAC110_netif.gw.addr=GateWay.addr;
		FTMAC110_netif.netmask.addr=NetMask.addr;       
		OSFlagPost(gpiNetStatusFlagGrp, FLAGGPI_LWIP_IP_READY, OS_FLAG_SET, &err);
	    ret=0;    
	        
	}
	return ret;
}

/*
Get Network information. by aher 2013/03/22
*/
void GetNetworkInfo(struct NetworkInfo *info)
{
	int i;

	if(LwipISStatic)
	{
		for(i=0;i<4;i++)
		{
			info->IPaddr[i]=LwipIPAddr[i];
			info->Netmask[i]=LwipSubnetMask[i];
			info->Gateway[i]=LwipDefaultGateway[i];
			info->IsStaticIP = LwipISStatic;
		}
	}	
	else
	{
		for(i=0;i<4;i++)
		{
			info->IPaddr[i]=LwipIpAddrInfo[i];
			info->Netmask[i]=LwipSubMaskInfo[i];
			info->Gateway[i]=LwipDefaultGatewayInfo[i];
			info->IsStaticIP = LwipISStatic;
		}
	}
}
void SetNetworkInfo(struct NetworkInfo *info)
{
	int i;
	for(i=0;i<4;i++)
	{
		LwipIPAddr[i]=info->IPaddr[i];
		LwipSubnetMask[i]=info->Netmask[i];
		LwipDefaultGateway[i]=info->Gateway[i];
	}
	LwipISStatic=info->IsStaticIP;
}
  
/*Clear Ip address.*/
void ClearNetworkInfo()
{
    if(LwipIpAddrInfo[0]!=0)
		{
			/*Clear IP information.*/			
			FTMAC110_netif.ip_addr.addr=0;
			FTMAC110_netif.gw.addr=0;
    		FTMAC110_netif.netmask.addr=0;
			LwipIpAddrInfo[0]=0;LwipIpAddrInfo[1]=0;LwipIpAddrInfo[2]=0;LwipIpAddrInfo[3]=0;
			LwipDefaultGatewayInfo[0]=0;LwipDefaultGatewayInfo[1]=0;LwipDefaultGatewayInfo[2]=0;LwipDefaultGatewayInfo[3]=0;
			LwipSubMaskInfo[0]=0;LwipSubMaskInfo[1]=0;LwipSubMaskInfo[2]=0;LwipSubMaskInfo[3]=0;
		}
}
  
//*-----------------------------------------------------------------------------------------------------------------------
//*????:LwIPEntry
//*????:LwIP????
//*????:none
//*????:none
//*-----------------------------------------------------------------------------------------------------------------------
// extern void vHandler_RTSP(ST_NETCONN);
    
void T_LwIPEntry(void *pevent)
{
	struct netconn  *__pstConn, *__pstNewConn;
	struct netbuf	*__pstNetbuf;
	void *data;
	u8_t *datab,*datac;
	u16_t len,i,totallen;
	int run_rtsp;
	RTC_DATE_TIME current_time;
	INT8U err;
	INT8U x;
    u8	p2p_fail_cnt=0;
 // StartTimer();  //??????

  	DEBUG_P2P("T_LwIPEntry\n");
	gpiNetStatusFlagGrp = OSFlagCreate(0x00000000, &err);
	InitLwIP();
	/*ENABLE_FTMAC110_INT_PHYSTS_CHG*/
	phywrite16(0x8000,FTPHY_REG_CONTROL_TEST2);  
  	//??Lwip,???????? 
  	//SetLwIP(0);

	for(x=0;x<50;x++)
	{
		if ((phyread16(FTPHY_REG_STATUS)&FTPHY_REG_STATUS_LINK)==1)
			break;
		OSTimeDly(1);
	}
	OSTimeDly(4); // Add for phy init to get correct status, Paul add, 180529
	if ((phyread16(FTPHY_REG_STATUS)&FTPHY_REG_STATUS_LINK)==0)
	{
		DEBUG_P2P(" Link => FAIL\n");
		//SetLwIP(0);
		//renewIP=0;
	}
	else
	{
    	DEBUG_P2P(" Link status => Ok\n");
		DEBUG_P2P("Get IP address...\n");
		SetLwIP(0);
		renewIP=0;
		#if SET_NTPTIME_TO_RTC
/*		Mark for project deine issue, need to discuss.Paul, 180524
		if (iconflag[UI_MENU_SETIDX_NTP_TIME] == 1)	//UI_MENU_NTPTIME_ON
			NTP_Switch(1);	//NTP_SWITCH_ON
		else
			NTP_Switch(0);	//NTP_SWITCH_OFF
*/
		ntpdate(0, 0);
		#endif
		#if CHECK_FW_VER_BOOTING
			DEBUG_P2P("Checking the latest firmware version.\n");
			Check_fw_ver_net(Check_by_net);
		#endif
			RTC_Get_Time(&gDHCPTime);
	}
	
	while(1)	
	{	
		RTC_Get_Time(&current_time);

		if(((net_link_status==NET_LINK_ON)&&(renewIP==1))||((FTMAC110_netif.ip_addr.addr==0)&&(net_link_status==NET_LINK_ON)
		&&(((RTC_Time_To_Second(&current_time))-(RTC_Time_To_Second(&gDHCPTime)))>DHCP_leasetime)))
		{
			DEBUG_P2P("Renew IP address...\n");
			SetLwIP(0);
			renewIP=0;
			RTC_Get_Time(&gDHCPTime);
			#if SET_NTPTIME_TO_RTC
/*		Mark for project deine issue, need to discuss.Paul, 180524
			if (iconflag[UI_MENU_SETIDX_NTP_TIME] == 1)	//UI_MENU_NTPTIME_ON
				NTP_Switch(1);	//NTP_SWITCH_ON
			else
				NTP_Switch(0);	//NTP_SWITCH_OFF
*/
			ntpdate(0, 0);
			#endif			
			#if CHECK_FW_VER_BOOTING	
				DEBUG_P2P("Checking the latest firmware version.\n");
				Check_fw_ver_net(Check_by_net);
			#endif
		}
			if(current_time.day!=gDHCPTime.day)
			{
			#if CHECK_FW_VER_BOOTING
				DEBUG_P2P("Checking the latest firmware version.\n");
				Check_fw_ver_net(Check_by_net);
			#endif
			RTC_Get_Time(&gDHCPTime);
		}

#if(HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) //20171208 add.
		if((!FW_UPGRAD_flag) && (!Reset_P2P_Connection_Stop_flag))
		{
			if(Reset_P2P_Connection_flag)
			{
				if(p2p_fail_cnt >= 6)
				{
					ResetALL();
					p2p_fail_cnt=0;
				}
				else
					p2p_fail_cnt++;
			}
				
			else
				Reset_P2P_Connection_flag = 1;
		}
#endif            
		
		OSTimeDly(200);

#if APP_KEEP_ALIVE
	/*===============================KEEP ALIVE==================================*/
			if((APPConnectIcon != 0) || (gOnlineNum > 0))
			{
				for(i=0;i<MAX_CLIENT;i++)
				{
					if(!gFlagKeepAlive[i])
					{
						Clear_Session_Status(i);
					}	
					else
					{
						RTC_Get_Time(&current_time);
						switch(i)
						{
							case 0:
							if(((RTC_Time_To_Second(&current_time))-(RTC_Time_To_Second(&gKeepAliveTime0))) >= KEEP_ALIVE_TIMEOUT)
							{
								gFlagKeepAlive[0] = 0;
								RTC_Get_Time(&gKeepAliveTime0);
							}
							break;
							case 1:
							if(((RTC_Time_To_Second(&current_time))-(RTC_Time_To_Second(&gKeepAliveTime1))) >= KEEP_ALIVE_TIMEOUT)
							{
								gFlagKeepAlive[1] = 0;
								RTC_Get_Time(&gKeepAliveTime1);
							}
							break;
							case 2:
							if(((RTC_Time_To_Second(&current_time))-(RTC_Time_To_Second(&gKeepAliveTime2))) >= KEEP_ALIVE_TIMEOUT)
							{
								gFlagKeepAlive[2] = 0;
								RTC_Get_Time(&gKeepAliveTime2);
							}
							break;
							case 3:
							if(((RTC_Time_To_Second(&current_time))-(RTC_Time_To_Second(&gKeepAliveTime3))) >= KEEP_ALIVE_TIMEOUT)
							{
								gFlagKeepAlive[3] = 0;
								RTC_Get_Time(&gKeepAliveTime3);
							}
							break;
							case 4:
							if(((RTC_Time_To_Second(&current_time))-(RTC_Time_To_Second(&gKeepAliveTime4))) >= KEEP_ALIVE_TIMEOUT)
							{
								gFlagKeepAlive[4] = 0;
								RTC_Get_Time(&gKeepAliveTime4);
							}
							break;
							default:
								break;
						}
						//printf("-%d-\n",gFlagKeepAlive);		
					}
				}
			}
	/*===============================KEEP ALIVE==================================*/
#endif

	/*
	if((net_link_status==NET_LINK_OFF)&&(!LwipISStatic))
	{
	    ClearNetworkInfo();
		
		}	
		*/
	}
/*
#if 1

	__pstConn = netconn_new(NETCONN_TCP);
	netconn_bind(__pstConn, NULL, RTSP_PORT);
	netconn_listen(__pstConn);
	
	//aher
	run_rtsp=0;
    //remote_ip = __pstConn->pcb.tcp->remote_ip;
    //DEBUG_P2P("11. 0x%08x\n",__pstConn->pcb.tcp->remote_ip.addr);
    //DEBUG_P2P("33. 0x%08x\n",remote_ip.addr);  

    //DEBUG_RTSP("=====>entering %s\n", __func__); 

	while(TRUE)
	{
		__pstNewConn = netconn_accept(__pstConn);
        remote_ip = __pstNewConn->pcb.tcp->remote_ip;
        //DEBUG_P2P("1. 0x%08x\n",__pstNewConn->pcb.tcp->remote_ip.addr);
        //DEBUG_P2P("3. 0x%08x\n",remote_ip.addr);                
        //aher
        //while(1)
        while(run_rtsp==0)
            {
		if(__pstNewConn != NULL)
		{			
			run_rtsp=vHandler_RTSP(__pstNewConn);
			//while(netconn_delete(__pstNewConn) != ERR_OK)
				//OSTimeDlyHMSM(0, 0, 5, 0);
		}
	    }
	    run_rtsp=0;
		while(netconn_delete(__pstNewConn) != ERR_OK)
				OSTimeDlyHMSM(0, 0, 5, 0);
    }

#else
	__pstConn = netconn_new(NETCONN_TCP);
	netconn_bind(__pstConn, NULL, 2200);
	netconn_listen(__pstConn);
	while(1)
	{

//		OSTimeDly(1000);
		__pstNewConn = netconn_accept(__pstConn);	
		while(__pstNewConn != NULL)
		{
		//	netconn_write(__pstNewConn, (u8_t*)ssst, 23, NETCONN_COPY);			
		                                        //	while((__pstNetbuf = netconn_recv(__pstNewConn)) != NULL)
		__pstNetbuf = netconn_recv(__pstNewConn);
			if(__pstNetbuf != NULL)
			{
				datac=__TxBufs; 
				totallen=0;
			do
				{
					netbuf_data(__pstNetbuf, &data, &len);
					datab = (unsigned char *) data;
					for(i=0;i<len;i++)
					{
						if(totallen<1024) 
							{
					    *datac++=*datab++; 
					    totallen++;
					    } 
				  }
				}while(netbuf_next(__pstNetbuf) >= 0);
				netconn_write(__pstNewConn, __TxBufs, totallen, NETCONN_COPY);
//		for(i=0;i<totallen;i++)
//		  {
//		  while((U0LSR&0x20)==0); 
//		  U0THR	=__TxBufs[i];
//		  }
				netbuf_delete(__pstNetbuf);	
			}
			//__pstNetbuf = netconn_recv(__pstNewConn);
			//if(__pstNetbuf != NULL)
			//{
			//	netconn_write(__pstNewConn, ssst, 20, NETCONN_COPY);
			//	netconn_write(__pstNewConn, "<body><h1>??LWIP TCP??!</h1></body>", 40, NETCONN_COPY);
			//	netbuf_delete(__pstNetbuf);	
			//}

		//	netconn_close(__pstNewConn);
		//	netconn_delete(__pstNewConn);
		// UART0WrStr("next delete conn.\n");
		//while(netconn_delete(__pstNewConn) != ERR_OK)
		// {
		//	 UART0WrStr("delete conn.\n");
		//	 OSTimeDly(100);
			 //OSTimeDlyHMSM(0, 0, 1, 0);
		//  }
	   }
	}
#endif

 }
 
*/
}
#else
u8_t my_ipaddr[5]={0};

#endif
