#include "sysopt.h"

#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)

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
//#include "spiapi.h"
#include "uiapi.h"
#include <lwip/sockets.h>
#include "rtcapi.h"
#include "p2pserver_api.h"
#include "sysapi.h" //Sean

 //*------------------------------------?????-------------------------------------------------------------------------
  // void InitLwIP(void);
  
  // void SetLwIP(void);
s32_t spiRead(u8_t* , u32_t , u32_t);
void telnet_init(void); 
void GetNetworkInfo(struct NetworkInfo *);
void SetNetworkInfo(struct NetworkInfo *);

extern u8 SetLwIP(u8 mode);

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
u8_t  my_hwaddr[6]= { 0x6d,0xf0,0x49,0xa2,0x66,0xF7};
//const u16_t MAC_ADDR[6]= { 0x6c,0xf0,0x49,0xa2,0x66,0x68};
const char ssst[]="welcome to ucos world!\n";
u8_t my_ipaddr[5]={IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3};
u8 qetIP=0;
int NTP_ON;
RTC_DATE_TIME gDHCPTime;
u32 DHCP_leasetime;

OS_EVENT* Wait_Link_Status_Evt;   /* 等待網路線連線狀態 */
extern u8_t uiMACAddr[6];
extern int net_link_status;
extern u8 renewIP;

#if CLOUD_SUPPORT
RTC_DATE_TIME gCloud_Token_Time;
extern int gFlagCloudLoginOK;
#endif
#if((HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M721) && (PROJ_OPT == 5)) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 8))) //Sean: 20170621 add. //20171208 add M732HT
extern u8	Reset_P2P_Connection_flag;
extern u8	Reset_P2P_Connection_Stop_flag;
extern void Reset_P2P_Connection(void);
extern u8 FW_UPGRAD_flag;
#endif
//extern sysSPI_Enable(void);
//extern sysSPI_Disable(void);

extern void SendPushMessage(int);
extern void EMACInit(void);  


//??????????
 struct netif  DM9000_netif; //?????????,????????ne2k_isr????????
 struct ip_addr remote_ip;
 
/* Resolve the domain name to IP address */
int DN2IP(char * URL, char * realIP)
{
    char buf[50]="";
    INT32U pu32IP;
    int err=0;
    //int count=0;
    char * tmp_IP;  
    convert(URL,buf);

OSTaskChangePrio(T_ETHERNETIF_INPUT_PRIO,RFIU_TASK_PRIORITY_HIGH);
    err=u32DNToIP(buf,strlen(buf)+1,&pu32IP);
	if(err!=0)
		printf("DNS err:%d\n",err);
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
void ntpdate()
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
	//RTC_TIME_ZONE zone;
	s8 TimeZone;
	u8	retry_cnt=0;
	
if((net_link_status==NET_LINK_ON)&&(NTP_ON==NTP_SWITCH_ON))
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
		printf("Create socket fail : %d\n",s);
	}
	else
	{
          err=DN2IP(hostname1,hostname_IP); 
          if(err==0)
              printf("Get NTP time: pool.ntp.org,IP = %s\n",hostname_IP);
          else
              printf("DN2IP fail...\n");
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
       printf("Get NTP time: ca.pool.ntp.org,IP = %s\n",hostname_IP);
    else
        printf("DN2IP fail...\n");
    
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
        printf("Get NTP time: time.stdtime.gov.tw,IP = %s\n",hostname_IP);

    else
        printf("DN2IP fail...\n");
	server_addr.sin_addr.s_addr = inet_addr(hostname_IP);
    OSTimeDly(10);
	i=sendto(s,msg,sizeof(msg),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
	// get the data back
	OSTimeDly(200);
	i=recv(s,buf,sizeof(buf),0);
}
	printf("Retrieve NTP time : %d\n",i);	
#if((HW_BOARD_OPTION == MR8200_RX_RDI_M721)&&(PROJ_OPT == 5))
	if((retry_cnt == 0) && (i != 48))
	{
		printf("Retrieve NTP time Wrong, retry...\n");
		retry_cnt++;
		close(s);
		goto Retry;
	}
#endif	
	tmit=ntohl(buf[10]);	//# get transmit time
	//printf("tmit=%d\n",tmit);
	
	tmit-= 2208988800U;	
	tmit-=946684800;
	
	if(i>0)
	{
	#if AutoNTPupdate
	TimeZone = SendGetTimeZoneMessage();	
	if(TimeZone == 100)
		printf("\nNTP Time Get Fail.\n");
	else
	{
	tmit = tmit + TimeZone*3600;
	RTC_Second_To_Time(tmit,&NTP_time);
	printf("\nNTP Time: %s\n",ctime(&tmit));
	RTCTime_Gmt_To_Local(&NTP_time,&Local_time);
	//RTC_Set_Time(&Local_time);
	RTC_Set_GMT_Time(&Local_time);
	}

	#else
	RTC_Second_To_Time(tmit,&Local_time);
	printf("\nNTP Time: %s\n",ctime(&tmit));
	//RTCTime_Gmt_To_Local(&NTP_time,&Local_time);
	//RTC_Set_Time(&Local_time);
	RTC_Set_GMT_Time(&Local_time);
	#endif
		/*Sync with TX 20140731*/
		#if RFIU_SUPPORT
			for(i=0;i<MAX_RFIU_UNIT;i++)
				uiSetRfTimeRxToTx(i);
		#endif
		#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
			uiFlowCheckDST(&Local_time, 2);
    		#endif	
    	}	
	else
    {
	    printf("Could'n get the NTP Time.\n");
        //sysForceWDTtoReboot(); //20150402_Sean,      NTP issue was fixed by aher 20160322.
    }
	close(s);	
}	
}
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

	Wait_Link_Status_Evt = OSSemCreate(0);
  
  }
    	
 //*-----------------------------------------------------------------------------------------------------------------------
 //*????:SetLwIP
 //*????:??LWIP,?????????????
 //*????:none
 //*????:none
 //*------------------------------------------------------------------------------------------------------------------------
 #define DHCP_OPTIMIZE	0
 
 u8 SetLwIP(u8 mode)
{
    extern err_t ethernetif_init(struct netif * netif);
 
    struct ip_addr IpAddr, NetMask, GateWay;
 
#if LWIP_DHCP
//    INT32U j;
//    INT16U i;
//    INT8S result;
    INT16U cnt;
//    int mscnt =0;
    INT8U err;
	 u8_t *p;
    u8 count=0, ret;
    #if DHCP_OPTIMIZE
    u8 DHCP_interval = 2;
    #endif
#endif
    //extern struct netif  rtl8019if_netif;
if(!qetIP)
{
    LWIP_PLATFORM_DIAG(("SetLwIP!!\r\n"));
        my_hwaddr[0]=uiMACAddr[0];
        my_hwaddr[1]=uiMACAddr[1];
        my_hwaddr[2]=uiMACAddr[2];
        my_hwaddr[3]=uiMACAddr[3];
        my_hwaddr[4]=uiMACAddr[4];
        my_hwaddr[5]=uiMACAddr[5];
       printf("SetLwIP_my_hwaddr: %2x:%2x:%2x:%2x:%2x:%2x\n",my_hwaddr[0],my_hwaddr[1],my_hwaddr[2],my_hwaddr[3],my_hwaddr[4],my_hwaddr[5]);
    netif_init();
    LWIP_PLATFORM_DIAG(("S0!!\r\n"));
    EMACInit_reboot();
    OSTimeDly(20);
    EMACInit();
    LWIP_PLATFORM_DIAG(("S1!!\r\n"));

//Static IP or Dynamic IP by aher 20121129	

    IP4_ADDR(&IpAddr , IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
    IP4_ADDR(&NetMask , NET_MASK0,NET_MASK1,NET_MASK2,NET_MASK3);
    IP4_ADDR(&GateWay,  GATEWAY_ADDR0, GATEWAY_ADDR1,GATEWAY_ADDR2,GATEWAY_ADDR3);

    netif_add(&DM9000_netif,&IpAddr , &NetMask ,&GateWay, NULL ,ethernetif_init, tcpip_input);
    netif_set_default(&DM9000_netif);
    netif_set_up(&DM9000_netif);
	qetIP=1;
}
if(!LwipISStatic)
{
	printf("DYNAMIC IP\n");
#if LWIP_DHCP

    //for(i=0; i<5; i++) //modified by AHER , the dhcp request will nonstop until get IP addr. 20120924
    #if DHCP_OPTIMIZE
    DHCP_interval = 2; //100ms
    #endif
    do
    {
    	//if(net_link_status==NET_LINK_ON)
    	{
    		#if DHCP_OPTIMIZE
    		if(DHCP_interval >= 20)
    			DHCP_interval = 20;
    		#endif
	        LWIP_DEBUGF(DHCP_DEBUG | DBG_TRACE | DBG_STATE, ("Start DHCP Request!!\r\n"));
	        LWIP_PLATFORM_DIAG(("Start DHCP Request!\r\n"));
	        count++;
	        if (mode)   /*Re*/
	        {   
	            if (count>8)
	            {
	                ret=-1;
	                break;
	            }    
	        }   
	        if(count==4)
	            EMACInit();

	        dhcp_start(&DM9000_netif);
	        IP4_ADDR(&IpAddr,0,0,0,0);
	        for(cnt=0; (cnt<10) && (IpAddr.addr == 0); cnt++)
	        {
	            IpAddr.addr = DM9000_netif.ip_addr.addr;
	            //for(j=0;j<100000;j++);
	   	     // LWIP_PLATFORM_DIAG(("\n DHCP dly_S\r\n"));
	            //OSTimeDlyHMSM(0,0,0,100);
	            #if DHCP_OPTIMIZE
	            OSTimeDly(DHCP_interval);
	            #else
				OSTimeDlyHMSM(0,0,1,0);
	            #endif
	            // LWIP_PLATFORM_DIAG(("\n DHCP dly _E\r\n"));
	        }
			if(IpAddr.addr != 0)
			{
				printf("DHCP:t0=%lu,t1=%lu,t2=%lu\n",DM9000_netif.dhcp->offered_t0_lease,DM9000_netif.dhcp->offered_t1_renew,DM9000_netif.dhcp->offered_t2_rebind);
				DHCP_leasetime=DM9000_netif.dhcp->offered_t1_renew;
			}	
	        dhcp_stop(&DM9000_netif);
	        if(IpAddr.addr != 0)
	        {
	            LWIP_DEBUGF(DHCP_DEBUG | DBG_TRACE | DBG_STATE, ("Start DHCP Request *** OK *** \r\n"));
				net_link_status=0;
	            ret=0;
	            break;
	        }
        }
        #if DHCP_OPTIMIZE
        if(DHCP_interval < 20)
        	DHCP_interval += 2;
        #endif
        OSTimeDly(1);
    }while (IpAddr.addr == 0);
    LWIP_PLATFORM_DIAG(("\n=== DHCP====\r\n"));
    IpAddr.addr = DM9000_netif.ip_addr.addr;
    GateWay.addr = DM9000_netif.gw.addr;
    NetMask.addr = DM9000_netif.netmask.addr;

    if(1)
    {
    
//        u8_t *p;
        p =(u8_t *) &IpAddr.addr;
        my_ipaddr[0]=p[0]; my_ipaddr[1]=p[1]; my_ipaddr[2]=p[2]; my_ipaddr[3]=p[3];   
		LwipIPAddr[0]=p[0];LwipIPAddr[1]=p[1];LwipIPAddr[2]=p[2];LwipIPAddr[3]=p[3];
        OSFlagPost(gpiNetStatusFlagGrp, FLAGGPI_LWIP_IP_READY, OS_FLAG_SET, &err);
        printf("\nMY IP %u.%u.%u.%u \n",p[0],p[1],p[2],p[3]);
		p =(u8_t *) &IpAddr.addr;
		LwipIpAddrInfo[0]=p[0];LwipIpAddrInfo[1]=p[1];LwipIpAddrInfo[2]=p[2];LwipIpAddrInfo[3]=p[3];
		p =(u8_t *) &GateWay.addr;
		LwipDefaultGatewayInfo[0]=p[0];LwipDefaultGatewayInfo[1]=p[1];LwipDefaultGatewayInfo[2]=p[2];LwipDefaultGatewayInfo[3]=p[3];
		p =(u8_t *) &NetMask.addr;
		LwipSubMaskInfo[0]=p[0];LwipSubMaskInfo[1]=p[1];LwipSubMaskInfo[2]=p[2];LwipSubMaskInfo[3]=p[3];
	}   
 
#endif 
}
if(LwipISStatic)
{
	u8_t *p;

	printf("STATIC IP\n");	
#if	(TUTK_SUPPORT==1)
        net_link_status=Get_network_status();
#endif	
	p =(u8_t *) &IpAddr.addr;
	p[0]=LwipIPAddr[0];p[1]=LwipIPAddr[1];p[2]=LwipIPAddr[2];p[3]=LwipIPAddr[3];
	my_ipaddr[0]=p[0]; my_ipaddr[1]=p[1]; my_ipaddr[2]=p[2]; my_ipaddr[3]=p[3];
	printf("\nMY IP %u.%u.%u.%u \n",p[0],p[1],p[2],p[3]);
	p =(u8_t *) &GateWay.addr;
	p[0]=LwipDefaultGateway[0];p[1]=LwipDefaultGateway[1];p[2]=LwipDefaultGateway[2];p[3]=LwipDefaultGateway[3];
	p =(u8_t *) &NetMask.addr;
	p[0]=LwipSubnetMask[0];p[1]=LwipSubnetMask[1];p[2]=LwipSubnetMask[2];p[3]=LwipSubnetMask[3];
	DM9000_netif.ip_addr.addr=IpAddr.addr;
	DM9000_netif.gw.addr=GateWay.addr;
	DM9000_netif.netmask.addr=NetMask.addr;       
        OSFlagPost(gpiNetStatusFlagGrp, FLAGGPI_LWIP_IP_READY, OS_FLAG_SET, &err);
    ret=0;    
        
        //OSTimeDly(60); //20160120 Sean 
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
			DM9000_netif.ip_addr.addr=0;
			DM9000_netif.gw.addr=0;
    		DM9000_netif.netmask.addr=0;
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

#define RTSP_PLAY 0 //2160929 Sean

void T_LwIPEntry(void *pevent)
{
#if RTSP_PLAY
    struct netconn  *__pstConn, *__pstNewConn;
    struct netbuf	*__pstNetbuf;
    void *data;
    u8_t *datab,*datac;
    u16_t len,i,totallen;
    int run_rtsp;
#endif 	
    RTC_DATE_TIME current_time;
    INT8U   err;
    u8	p2p_fail_cnt=0;

    // StartTimer();  //??????

    printf("T_LwIPEntry\n");

    //???Lwip
    InitLwIP();
    //??Lwip,????????  
    SetLwIP(0);
    OSSemPend(Wait_Link_Status_Evt,OS_IPC_WAIT_FOREVER, &err);  //20160201 Sean Wait Untill Network Link Status  

    printf("T_LwIPEntry\n");
    
#if CLOUD_SUPPORT		
		gFlagCloudLoginOK = 1;
		RTC_Get_Time(&gCloud_Token_Time);
#endif

#if SET_NTPTIME_TO_RTC
#if((HW_BOARD_OPTION == MR8211_ZINWELL)||(HW_BOARD_OPTION == MR8200_RX_DB2))
    NTP_Switch(NTP_SWITCH_ON);
#elif((HW_BOARD_OPTION == MR8200_RX_MAYON_MWM719)||(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014) || (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902))
    if(iconflag[UI_MENU_SETIDX_NTP] == 1)
        NTP_Switch(NTP_SWITCH_ON);
#endif
    ntpdate();
#endif

#if (CHECK_FW_VER_BOOTING && NIC_SUPPORT)
    printf("Checking the latest firmware version.\n");
    Check_fw_ver_net(Check_by_net);
#endif



    RTC_Get_Time(&gDHCPTime);

    while(1)	
    {
        RTC_Get_Time(&current_time);		
        if(((net_link_status==NET_LINK_ON)&&(renewIP==1))||((DM9000_netif.ip_addr.addr==0)&&(net_link_status==NET_LINK_ON)
        &&(((RTC_Time_To_Second(&current_time))-(RTC_Time_To_Second(&gDHCPTime)))>DHCP_leasetime)))
        {
            printf("Renew IP address...\n");
            SetLwIP(0);
            renewIP=0;
            RTC_Get_Time(&gDHCPTime);
#if (CHECK_FW_VER_BOOTING && NIC_SUPPORT)
            printf("Checking the latest firmware version.\n");
            Check_fw_ver_net(Check_by_net);
#endif

#if(((HW_BOARD_OPTION == MR8200_RX_RDI_M721) && (PROJ_OPT == 5))  || ((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 8))) //Sean: 20170621 add. 20171201 add M706
			ntpdate();
#endif

        }
        if(current_time.day!=gDHCPTime.day)
        {
#if (CHECK_FW_VER_BOOTING && NIC_SUPPORT)
            printf("Checking the latest firmware version.\n");
            Check_fw_ver_net(Check_by_net);
#endif
            RTC_Get_Time(&gDHCPTime);
        }
        
#if CLOUD_SUPPORT   //Token存活一天
        if(((RTC_Time_To_Second(&current_time))-(RTC_Time_To_Second(&gCloud_Token_Time)))>86400)
        {
            gFlagCloudLoginOK = 1;
            RTC_Get_Time(&gCloud_Token_Time);
        }
#endif

#if((HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M721) && (PROJ_OPT == 5)) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 8))) //Sean: 20170621 add. //20171208 add M732HT
		if((!FW_UPGRAD_flag) && (!Reset_P2P_Connection_Stop_flag))
		{
	        if(Reset_P2P_Connection_flag)
	        {
	        	if(p2p_fail_cnt >= 6)
	        	{
	        		Reset_P2P_Connection();
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
        /*
        if((net_link_status==NET_LINK_OFF)&&(!LwipISStatic))
        {
        ClearNetworkInfo();

        }	
        */
    }

#if RTSP_PLAY

#if 1

	__pstConn = netconn_new(NETCONN_TCP);
	netconn_bind(__pstConn, NULL, RTSP_PORT);
	netconn_listen(__pstConn);
	
	//aher
	run_rtsp=0;
    //remote_ip = __pstConn->pcb.tcp->remote_ip;
    //printf("11. 0x%08x\n",__pstConn->pcb.tcp->remote_ip.addr);
    //printf("33. 0x%08x\n",remote_ip.addr);  

    //DEBUG_RTSP("=====>entering %s\n", __func__); 

	while(TRUE)
	{
		__pstNewConn = netconn_accept(__pstConn);
        remote_ip = __pstNewConn->pcb.tcp->remote_ip;
        //printf("1. 0x%08x\n",__pstNewConn->pcb.tcp->remote_ip.addr);
        //printf("3. 0x%08x\n",remote_ip.addr);                
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
 
#endif
}

#else
#include "general.h"
#include "task.h"
#include "uiapi.h"
#include "lwip/lwipsys.h"

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

u8 my_ipaddr[5]={IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3};
u8 LwipIPAddr[4]        ={192,168,1,118};
u8 LwipSubnetMask[4]    ={255,255,255,0};
u8 LwipDefaultGateway[4]={192,168,1,1};
u8 LwipISStatic=0;     /* 0: Dynamic IP, 1: Static IP */
u8 LwipIpAddrInfo[4]={0};
u8 LwipSubMaskInfo[4]={0};
u8 LwipDefaultGatewayInfo[4]={0};

/* Resolve the domain name to IP address */
int DN2IP(char * URL, char * realIP)
{
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
u8 SetLwIP(u8 mode){}
void ClearNetworkInfo(){}

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
void ntpdate(){}

#endif //#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)

