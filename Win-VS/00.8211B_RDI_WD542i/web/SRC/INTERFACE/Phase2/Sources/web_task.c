/*
 *  File:       web_task.c
 *
 *  Contains:   RomPager "Main" for Phase2
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *
 *  Copyright:  ?1995-1997 by Allegro Software Development Corporation
 *  All rights reserved.
 *
 *  This module contains confidential, unpublished, proprietary
 *  source code of Allegro Software Development Corporation.
 *
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 *
 *  License is granted for specific uses only under separate
 *  written license by Allegro Software Development Corporation.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Change History (most recent first):
 *
 *      11/25/97    rhb     integration with RomPager Version 2.0
 *      04/13/97    nam     integration with RomPager Version 1.6
 *      02/09/97    nam     created
 *
 *  To Do:
 *
 */
#if 1   /*Elsa modify for web*/
#include    "general.h"
#include    "wb_menu.h"
#else
#include    <nucleus.h>
#include    <inc/ctypes.h>

#include    <l3/port/inc/la3_os.h>
#include    <l3/port/inc/la3_port.h>
#include    <l3/ip/inc/la3_ip.h>
#include    <l3/ip/inc/epm_ip.h>
#include    <l3/inc/phase2.h>
#include    <socket.h>
#endif
#if (WEB_SERVER_SUPPORT == 1)

#include    "AsEngine.h"
//#include    "Rpprotos.h"
#include    "Asprotos.h"
#include    "lwipapi.h"
#include    "task.h"
#include    "gpiapi.h"

#if (WEB_SERVER_SUPPORT == 1)
#define WEB_MUT_TSK     0
#else
#define WEB_MUT_TSK     1
#endif
#if (WEB_MUT_TSK == 1)
#define L3_WEB_TABLE_SIZE L3_WEB_MAX_CONNECT_NUMBER
#else
#define L3_WEB_TABLE_SIZE   1
#endif

#define WEB_BAD_PORT_EVENT 1
#define WEB_UPDATE_PORT_EVENT 2
#define WEB_SOCK_FORK_READY      1
#define WEB_SOCK_DUPICATE_READY  2
#define WEB_SESSION_READY        4

#define WEB_REBOOT_DELAY 300  /* tick */


#if 1   /*Elsa modify for web*/
#define LA3_WEB_TASK_STACK_SIZE     (4*1024)
#endif

enum WEB_SOCK_FORK_STATE {
    WEB_SOCK_FORK_NONE=0,
    WEB_SOCK_FORK_BUSY,
    WEB_SOCK_FORK_WAIT
};


typedef struct web_sock_forkblock
{   
    s8 name[8];
    UI32_T qID;
    enum WEB_SOCK_FORK_STATE state; 
    OS_FLAG_GRP *eventGroup;
    UI32_T ipAddress;
}WEB_SOCK_FORKBLOCK;

#if (WEB_MUT_TSK == 1)
int WebTaskID[L3_WEB_TABLE_SIZE] =
{
    1,
    2,
    3,
    4
};
#else
int WebTaskID[L3_WEB_TABLE_SIZE] =
{
    0
};
#endif

/*Should be remove to task.h*/
#define WEB_TASK1_PRIORITY  20
#define WEB_TASK2_PRIORITY  20
#define WEB_TASK3_PRIORITY  20
#define WEB_TASK4_PRIORITY  20
#define WEB_TASK_LISTEN_PRIORITY  20
#define WEB_TASK_LISTEN_SSL_PRIORITY  20
/*end of Should be remove to task.h*/

#if (WEB_MUT_TSK == 1)
INT8U WebTaskPriority[L3_WEB_TABLE_SIZE] =
{
    WEB_TASK1_PRIORITY,
    WEB_TASK2_PRIORITY,
    WEB_TASK3_PRIORITY,
    WEB_TASK4_PRIORITY,
};

static OS_STK webStack[L3_WEB_TABLE_SIZE][LA3_WEB_TASK_STACK_SIZE];
#else
INT8U WebTaskPriority[L3_WEB_TABLE_SIZE] =
{
    WEB_TASK_PRIORITY
};
OS_STK webStack[WEB_TASK_STACK_SIZE];
OS_STK webListenStack[WEB_LISTEN_TASK_STACK_SIZE];
#endif

#if 0   /*Elsa modify for web*/
static LA3_OS_TASK_T  webForkTask[L3_WEB_TABLE_SIZE] ;
#endif
static int sessionNumber = 0;
static int webMaxConnectNumber=L3_WEB_TABLE_SIZE;

static WEB_SOCK_FORKBLOCK webStaticSessionTable[L3_WEB_TABLE_SIZE];

static unsigned short   webPortNumber = 80;
static unsigned short   webPortNumberSSL = 443;//Scott Tsai 2004/01/15
static Boolean          serverIsUp ;
#if 0   /*Elsa modify for web*/
static LA3_OS_TASK_T    taskWEB[2];//Scott Tsai 2004/01/15
#endif
//static LA3_OS_TASK_T    taskWEB ;
static BOOLEAN_T        bWebReset ;
static BOOLEAN_T        bListenEnable = TRUE;
static BOOLEAN_T        bListenEnableSSL = TRUE;
static BOOLEAN_T        bChangePort=TRUE;
static BOOLEAN_T        bChangeSSLPort=TRUE;
static OS_EVENT*        webSemaphore;
static OS_FLAG_GRP*     webEvent;

static OS_STK    WEB_Stack[2][LA3_WEB_TASK_STACK_SIZE];//Scott Tsai 2004/01/15
//static UI8_T    WEB_Stack[LA3_WEB_TASK_STACK_SIZE];

int debug_web = 0 ;
static int web_task_debug_ssl = 0 ;
static int stcpPort[L3_WEB_MAX_CONNECT_NUMBER]; /*Anson for debug 2001/10/23 11:35¤W¤È*/

/*
 * Time ticks when the web task be created *
 * used for calculating the time(in sec) since the web task be created
 *
 * added by chchien 8/21/2000
 */
static BOOLEAN_T  bReboot = FALSE;
static BOOLEAN_T  bResetConfig = FALSE;
static BOOLEAN_T  bResetSystem = FALSE;
static UI32_T     rebootTime;
char	USERNAME[32]= "";

int LissenFd;
/*------------------------------------------------------------------------------
 *  Function:
 *      static int  _WEB_Get_Ticket(int sockfd,UI32_T clientAddress)
 *  Summary:
 *      Allocate telnet control block and set the parameter.
 *  Parameters:
 *      flags : the telnet connection flag.
 *      sockfd: the new connection socket.
 *      clientAddress: the new connection client address.
 *      sessionNumber: the number of old connections
 *  Return:
 *      > 0: the point of the telnet session control block.
 *      = 0: some error happen, we can not receive enougth resource to open
 *  Description:
 *      None.
 *------------------------------------------------------------------------------
 */
static int  _WEB_Get_Ticket(int sockfd, UI32_T clientAddress)
{
    int sessionID=-1,i;
    WEB_SOCK_FORKBLOCK    *ptrSession;

    if(sockfd){}

    /* Find an empty entry of TelnetInfo Table */
    if(sessionNumber >= webMaxConnectNumber)
    {
        printf("_WEB_Get_Ticket %d > %d\n", sessionNumber, webMaxConnectNumber);
        return -1;
    }

    for(i=0;i<L3_WEB_TABLE_SIZE;i++)
    {
        ptrSession=&webStaticSessionTable[i];
        printf("ptrSession->state %d\n",ptrSession->state);

        if(ptrSession->state==WEB_SOCK_FORK_WAIT)
        {
            ptrSession->ipAddress=clientAddress;
            sessionID=i;
            break;
        }
    }

    if (sessionID == -1)
        return -1;
        
    sessionNumber++;
    return sessionID;   
}

static u8 _WEB_SOCK_Fork(WEB_SOCK_FORKBLOCK    *ptrSession)
{
    u8 err;
    OS_FLAGS status;

    if (ptrSession->state != WEB_SOCK_FORK_WAIT)
        return 0;

    printf("OSFlagPost eventGroup %x\n",ptrSession->eventGroup);
    status = OSFlagPost(ptrSession->eventGroup, WEB_SOCK_DUPICATE_READY, OS_FLAG_SET, &err);

    if (err != OS_NO_ERR)
    {
        DEBUG_WEB("WEB_SOCK_Fork: OSFlagPost Error !!\n");
    }
    return 1;
}

static int _WEB_Create_TcpListenSock(UI32_T host,unsigned short port)
{
    struct sockaddr_in addr_in;
	struct sockaddr_in sin;	
    int s;

#if 0
	if((s =(int) socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		printf("  [SetupSocket] open UDP socket error!\n");
		return;
	}
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(9527);
	if(bind(s, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0)
	{
		printf("  [SetupSocket] bind UDP socket error!\n");
		return;
	}
	sin.sin_port = htons(8888);
	sin.sin_addr.s_addr = inet_addr("10.55.10.100");
#else
    addr_in.sin_family     = AF_INET ;
    addr_in.sin_port       = htons(port);
    addr_in.sin_addr.s_addr= htonl(host);

    DEBUG_WEB("_WEB_Create_TcpListenSock \n");
    if((s = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        
        DEBUG_WEB("_WEB_Create_TcpListenSock: socket Error !!\n");
        return -1;
    }
    if(bind(s, (struct sockaddr *)&addr_in, sizeof(addr_in)) < 0)
    {
        DEBUG_WEB("_WEB_Create_TcpListenSock: bind Error !!\n");
        close(s);
        return -1;
    }
    if(listen(s, 5) != 0 )
    {
        DEBUG_WEB("_WEB_Create_TcpListenSock: listen Error !!\n");
        close(s);
        return -1;
    }
#endif
    return s;
}

/*------------------------------------------------------------------------------
 *  Function:
 *      static void _WEB_Task_Session(void* argc)
 *  Summary:
 *      The main function of web session.
 *  Parameters:
 *      argc:
 *      argv: *
 *  Return:
 *      None.
 *  Description:
 *      None.
 *------------------------------------------------------------------------------
 */
 

static void _WEB_Task_Session(void* argc)
{
    int                 acceptfd;
    int                 *sessionID;
    WEB_SOCK_FORKBLOCK  *ptrSession;
    void                *ptrTheData;
    u8                  err;

    int theHttpFlag=0;
    int theTcpFlag=0;
    int bReturn = 0;

    sessionID=(int*)argc;

    ptrSession=&webStaticSessionTable[*sessionID];

    OSFlagPend(gpiNetStatusFlagGrp, FLAGGPI_LWIP_IP_READY, OS_FLAG_WAIT_SET_ANY, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_WEB("_WEB_Task_Session [%d] Start\n", *sessionID);
    ptrTheData=AllegroTaskInit(*sessionID);


    while (bWebReset == FALSE)
    {
        OSFlagPend(ptrSession->eventGroup, WEB_SOCK_DUPICATE_READY, 
            (OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME), OS_IPC_WAIT_FOREVER, &err);
        DEBUG_WEB("Session  Pend %d, err %d\n", *sessionID, err);
        if (err == OS_NO_ERR)
        {
            acceptfd = LissenFd;
            ptrSession->state=WEB_SOCK_FORK_BUSY;
            //RomPagerMainTask(ptrTheData,acceptfd,ptrSession->ipAddress);
            do{
                printf("_WEB_Task_Session acceptfd %d!!!!! \n",acceptfd);
                bReturn =  AllegroConnectionTask(ptrTheData,*sessionID, &theHttpFlag, &theTcpFlag, acceptfd);

        	}while(bReturn == 1);

            ptrSession->state=WEB_SOCK_FORK_WAIT;
            printf("@@@@Task_Session End\n");
        }
        //~jesson add Rp4.0
        if(*sessionID==0)
	    	AllegroTimerTask(ptrTheData, acceptfd);
        close(acceptfd);
        sessionNumber--;
        OSFlagPost(ptrSession->eventGroup, WEB_SESSION_READY, OS_FLAG_SET, &err);
        stcpPort[*sessionID]=0;
    }
    //RomPagerDeInit(ptrTheData);
    AllegroTaskDeInit(ptrTheData); //~jesson sd1 Rp4.0
    return;
}

/*------------------------------------------------------------------------------
 *  Function:
 *      void _WEB_Task_listen(void* argc)
 *  Summary:
 *      The main function of web listen.
 *  Parameters:
 *      argc:
 *      argv: *
 *  Return:
 *      None.
 *  Description:
 *      None.
 *------------------------------------------------------------------------------
 */

static void _WEB_Task_listen(void* argc)
{
#if 1   /*Elsa modify for web*/
    struct timeval          tout;
    struct sockaddr_in      clientaddr ;
    int                     len,n;
    fd_set                  rs, ws;
#endif
    int                     sockfd, listenfd;
    int sessionID;
    WEB_SOCK_FORKBLOCK      *ptrSession;
    BOOLEAN_T               bCreateFd = FALSE;
    unsigned short          oldPort = 80;
    u8                      err;

    /*avoid warning message*/
    if(argc)
    {};

    listenfd=0;
    tout.tv_sec=1;
    tout.tv_usec=0;

    OSFlagPend(gpiNetStatusFlagGrp, FLAGGPI_LWIP_IP_READY, OS_FLAG_WAIT_SET_ANY, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_WEB("_WEB_Task_listen Start\n");

    while(bWebReset == FALSE)
    {
        //OSTimeDly(50);
#if 1   /*Elsa : do not need to change port*/
        /* implement change port */
        printf("bChangePort = %d, bCreateFd = %d \n",bChangePort, bCreateFd);
        if (bChangePort || bCreateFd)
        {
            if (listenfd>=0)
            {
                printf("if (listenfd>=0)\n");
                close(listenfd);
            }

            if ((listenfd = _WEB_Create_TcpListenSock(INADDR_ANY,webPortNumber))<0)
            {
                DEBUG_WEB("Web change port Fail!\n");
                webPortNumber = oldPort;

                /* find avaliable port */
                while ((listenfd = _WEB_Create_TcpListenSock(INADDR_ANY,webPortNumber))<0)
                {
                    webPortNumber ++ ;

                    if (webPortNumber > oldPort + 100)
                    {
                        DEBUG_WEB("Web Fail!\n");
                    }
                }

                /* send event */
                if (bChangePort)
                {
                    bChangePort = FALSE;
                    OSFlagPost(webEvent, WEB_BAD_PORT_EVENT, OS_FLAG_SET, &err);
                    if (err != OS_NO_ERR)
                        DEBUG_WEB("Web OSFlagPost Bad Port Fail!\n");
                }
            }
            else
            {
                oldPort = webPortNumber;

                /* send event */
                if (bChangePort)
                {
                    bChangePort = FALSE;

                    OSFlagPost(webEvent, WEB_UPDATE_PORT_EVENT, OS_FLAG_SET, &err);
                    if (err != OS_NO_ERR)
                        DEBUG_WEB("Web OSFlagPost Change Port Fail!\n");
                }
            }

            bCreateFd = FALSE;
        }
#endif
#if 1   /*Elsa : do not need to enable/disable web*/
        /* implement disable */
        if (!bListenEnable)
        {
            if (listenfd>0)
            {
                DEBUG_WEB("Web disable!\n");
                oldPort = webPortNumber;
                webPortNumber = 0;
                close(listenfd);

                listenfd = 0;
            }
            OSTimeDly(40);
            continue;
        }

    #if 0
        /* implement enable */
        if (listenfd == 0)
        {
            bCreateFd = TRUE;

            if (webPortNumber == 0)
                webPortNumber = oldPort;

            continue;
        }
    #endif
#endif
        FD_ZERO(&rs);
        FD_SET(listenfd,&rs);
        n = select(listenfd+1, &rs, NULL, NULL, &tout);
        printf("select %d\n",n);
 #if 0   /*Elsa modify for web*/
       /* for web reboot page show in browser */

        if (bReboot)
        {
            /* delay some time */
            if (OSTimeGet() - rebootTime > WEB_REBOOT_DELAY)
            {
                ST_SYS_Reboot();
        		bReboot = FALSE;
        	}
        }

        if (bResetConfig)
        {
        	/* delay some time */
           	if (OSTimeGet() - rebootTime > WEB_REBOOT_DELAY)
           	{
                web_Reset_Config();
                bResetConfig = FALSE;
            }
    	}

        if (bResetSystem)
        {
           	/* delay some time */
           	if (OSTimeGet() - rebootTime > WEB_REBOOT_DELAY)
           	{
                web_Reset_Config();
                dlk_SaveChangesNow(USERNAME);
         	    ST_SYS_Reboot();
                bResetSystem = FALSE;
            }
        }
#endif
        if(n<=0)
            continue;

        len = sizeof(clientaddr);

        sockfd = accept(listenfd , (struct sockaddr *) &clientaddr, &len) ;
      	DEBUG_WEB("sockfd=%d\n", sockfd);

        if(sockfd < 0)
        {
            for(;;);
        }
        else
        {
            if ((sessionID = _WEB_Get_Ticket(sockfd,clientaddr.sin_addr.s_addr))>=0)
            {
                printf("Web Listen sessionID %d\n",sessionID);
                LissenFd = sockfd;
                stcpPort[sessionID] = clientaddr.sin_port;
                ptrSession = &webStaticSessionTable[sessionID];
                if (_WEB_SOCK_Fork(ptrSession) != 1)
                    sessionNumber--;
                OSFlagPend(ptrSession->eventGroup, WEB_SESSION_READY, 
                    (OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME), OS_IPC_WAIT_FOREVER, &err);

            }
            //OSTimeDly(400);
            //close(sockfd);
        }
    }
    return;
}

#if (RomPagerSecure)
static void _WEB_Task_listenSSL(void* argc)
{
#if 0   /*Elsa modify for web*/
    struct timeval          tout;
    struct sockaddr_in      clientaddr ;
    int                     len,n;
    fd_set                  rs;
    int                     sockfd, listenfd;
    int sessionID;
    WEB_SOCK_FORKBLOCK      *ptrSession;
    BOOLEAN_T               bCreateFd = FALSE;
    unsigned short          oldPort = 443;


    /*avoid warning message*/
    if(argc )
    {}

#ifdef WIN32
    LA3_MEM_Register("web_S", (UI32_T) LA3_OS_Current_Task_Pointer());
#endif

    listenfd=0;
    tout.tv_sec=1;
    tout.tv_usec=0;

    while (!LA3_IP_Get_TaskStatus())
        OSTimeDly(40); /* sleep 1 sec */

    while(bWebReset == FALSE)
    {
        /* implement change port */
        if (bChangeSSLPort || bCreateFd)
        {
            if (listenfd>0)
            {
                s_close(listenfd);
            }

            if((listenfd=LA3_SOCK_Create_TcpListenSock(INADDR_ANY,webPortNumberSSL))<1)
            {
                webPortNumberSSL = oldPort;

                /* find avaliable port */
                while((listenfd=LA3_SOCK_Create_TcpListenSock(INADDR_ANY,webPortNumberSSL))<1)
                {
                    webPortNumberSSL ++ ;

                    if (webPortNumberSSL > oldPort + 100)
                    {
                        DEBUG_WEB("Web Fail!");
                    }
                }

                /* send event */
                if (bChangeSSLPort)
                {
                    bChangeSSLPort = FALSE;

                    if (LA3_OS_Set_Events(&webEvent, WEB_BAD_PORT_EVENT, LA3_OS_OR)!= LA3_OS_SUCCESS)
                        DEBUG_WEB("Web Fail!");
                }
            }
            else
            {
                oldPort = webPortNumberSSL;
                /* send event */
                if (bChangeSSLPort)
                {
                    bChangeSSLPort = FALSE;

                    if (LA3_OS_Set_Events(&webEvent, WEB_UPDATE_PORT_EVENT, LA3_OS_OR)!= LA3_OS_SUCCESS)
                        DEBUG_WEB("Web Fail!");
                }
            }

            bCreateFd = FALSE;
            FD_ZERO(&rs);
            FD_SET(listenfd,&rs);
        }

        /* implement disable */
        if (!bListenEnableSSL)
        {
            if (listenfd>0)
            {
                oldPort = webPortNumberSSL;
                webPortNumberSSL = 0;
                s_close(listenfd);

                listenfd = 0;
            }

            OSTimeDly(40);
            continue;
        }

        /* implement enable */
        if (listenfd == 0)
        {
            bCreateFd = TRUE;

            if (webPortNumberSSL == 0)
                webPortNumberSSL = oldPort;

            continue;
        }

        n = select(L3_WEB_TABLE_SIZE, &rs, NULL, NULL, &tout);
        if(n<=0)
            continue;

        len = sizeof(clientaddr);

        sockfd = accept(listenfd , (struct sockaddr *) &clientaddr, &len) ;
        if (WEB_Get_SSL_Debug() > 0)
            xprintf("SSL sockfd=%d\n", sockfd);
        if(sockfd < 0)
        {
            for(;;);
        }
        else
        {
            if((sessionID=_WEB_Get_Ticket(sockfd,clientaddr.sin_addr))>=0)
            {
                stcpPort[sessionID]=clientaddr.sin_port;
                ptrSession=&webStaticSessionTable[sessionID];
                if(_WEB_SOCK_Fork(ptrSession)!=L3_SUCCESS)
                    sessionNumber--;

            }
            s_close(sockfd);
        }
    }
#endif
    return;
}
#endif

void WEB_Init(void)
{
    INT8U   status;
    int i;
    WEB_SOCK_FORKBLOCK  *ptrSession;
    UI32_T              port;
    u8                  err;

    serverIsUp  = FALSE;
    bWebReset   = FALSE;

    memset(webStaticSessionTable,0,sizeof(webStaticSessionTable));

    for(i=0;i<L3_WEB_TABLE_SIZE;i++)
    {
        ptrSession=&webStaticSessionTable[i];

        sprintf((char *)webStaticSessionTable[i].name,"web_%i",i);

#if (WEB_MUT_TSK == 1)
        status  = OSTaskCreate(_WEB_Task_Session, (void *)&WebTaskID[i], webStack[i], WebTaskPriority[i]);
#else
        status  = OSTaskCreate(_WEB_Task_Session, (void *)&WebTaskID[i], WEB_TASK_STACK, WebTaskPriority[i]);
#endif
        if (status != OS_NO_ERR)
        {
            DEBUG_WEB("Create Web task %d fail !!\n", i);
            return;
        }
        ptrSession->eventGroup = OSFlagCreate(0x00000000, &err);

        if (err != OS_NO_ERR)
        {
            DEBUG_WEB("Create web eventGroup %d fail !!\n", i);
            return;
        }

        webStaticSessionTable[i].state=WEB_SOCK_FORK_WAIT;
        stcpPort[i]=0;
    }

    webSemaphore = OSSemCreate(0); 

    if (webSemaphore == NULL)
    {
        /* must success */
        DEBUG_WEB("Create webSemaphore fail !!\n");
        return;
    }

    webEvent = OSFlagCreate(0x00000000, &err);
    if(err != OS_NO_ERR)
    {
        /* must success */
        DEBUG_WEB("Create webEvent Flag fail !!\n");
        return;
    }

#if 0   /*Elsa modify for web*/
    /*Set web Port and state*/
    if (EPM_IP_Get_WEBPort(&port))
    {
        webPortNumber = port;
    }
    else
        webPortNumber = 80;

    if (!EPM_IP_Get_WEBState(&bListenEnable))
        bListenEnable = TRUE;
#endif

    status  = OSTaskCreate(_WEB_Task_listen, (void *)0, WEB_LISTEN_TASK_STACK, WEB_LISTEN_TASK_PRIORITY);
    if (status != OS_NO_ERR)
    {
        DEBUG_WEB("Create listen fail !!\n");
        return;
    }

#if (RomPagerSecure)
    status  = OSTaskCreate(_WEB_Task_listenSSL, (void *)0, WEB_Stack[1], WEB_TASK_LISTEN_SSL_PRIORITY);
    if (status != OS_NO_ERR)
    {
        DEBUG_WEB("Create listenSSL fail !!\n");
        return;
    }
#endif

    /* for first port create */
    //OSFlagPend(webEvent, (WEB_BAD_PORT_EVENT | WEB_UPDATE_PORT_EVENT), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
}

void WEB_Terminate(void)
{
    bWebReset = TRUE;

    while (serverIsUp)
    {
        OSTimeDly(8);
    }
    return;
}

int WEB_Get_SessionInfo(int sessionID)
{
    if(sessionID>webMaxConnectNumber)
        return -1;
    return stcpPort[sessionID];
}

/*------------------------------------------------------------------------
 * Function:    WEB_Set_State
 * Purpose:     set web state
 * Parameters:
 *    Input:    state =  TRUE : enable web, FALSE : disable web
 *    Output:   None
 * returns:     TRUE : success FALSE : failure
 *------------------------------------------------------------------------
 */
BOOLEAN_T WEB_Set_State(BOOLEAN_T state)
{
    bListenEnable =  state;

#if 0   /*Elsa modify for web*/
    EPM_IP_Update_WEBState(state);

#endif
    return TRUE;
}

/*------------------------------------------------------------------------
 * Function:    WEB_Get_State
 * Purpose:     get web state
 * Parameters:
 *    Input:    None
 *    Output:   *pState = TRUE : enable web, FALSE : disable web
 * returns:     TRUE : success FALSE : failure
 *------------------------------------------------------------------------
 */
BOOLEAN_T WEB_Get_State(BOOLEAN_T *pState)
{
    *pState = bListenEnable;

    return TRUE;
}

/*------------------------------------------------------------------------
 * Function:    WEB_Set_Port
 * Purpose:     set web port
 * Parameters:
 *    Input:    port = the port number
 *    Output:   None
 * returns:     TRUE : success FALSE : failure
 *------------------------------------------------------------------------
 */
BOOLEAN_T WEB_Set_Port(UI32_T port)
{
    UI32_T      event = 0;
    BOOLEAN_T   returnVal = FALSE;
    u8          err;

    /* check if in change process */
    if (bChangePort == TRUE)
        return FALSE;

    if (port == 0 || port == 443)
        return FALSE;

    if (port == webPortNumber)
        return TRUE;

    if (port > 0xFFFF)
        return FALSE;

    OSSemPend(webSemaphore, OS_IPC_WAIT_FOREVER, &err); 
    if (err != OS_NO_ERR)
    {
        /* must success */
        DEBUG_WEB("Web Set Port Fail!\n");
    }

    webPortNumberSSL=443;
    event = OSFlagPend(webEvent, (WEB_BAD_PORT_EVENT | WEB_UPDATE_PORT_EVENT), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPost(webSemaphore);

    switch (event)
    {
        case WEB_UPDATE_PORT_EVENT:
            returnVal = TRUE;
            bChangePort=TRUE;
        	webPortNumber=port;
#if 0   /*Elsa modify for web*/
            EPM_IP_Update_WEBPort(port);
#endif
            break;
        case WEB_BAD_PORT_EVENT:
        default:
            returnVal = FALSE;
            break;
    }
    return returnVal;
}

/*------------------------------------------------------------------------
 * Function:    WEB_Get_Port
 * Purpose:     get web port
 * Parameters:
 *    Input:    None
 *    Output:   *pPort = the port number
 * returns:     TRUE : success FALSE : failure
 *------------------------------------------------------------------------
 */
BOOLEAN_T WEB_Get_Port(UI32_T *pPort)
{
    *pPort=webPortNumber;
    return TRUE;
}

/*------------------------------------------------------------------------
 * Function:    WEB_Set_SSL_Debug
 * Purpose:     set web ssl debug
 * Parameters:
 *    Input:    value of debug, now is 1/0
 *    Output:   None
 * returns:     TRUE : success FALSE : failure
 *------------------------------------------------------------------------
 */
BOOLEAN_T WEB_Set_SSL_Debug(int value)
{
	web_task_debug_ssl = value;
    return TRUE;
}

/*------------------------------------------------------------------------
 * Function:    WEB_Get_SSL_Debug
 * Purpose:     get web ssl debug
 * Parameters:
 *    Input:    None
 *    Output:   None
 * returns:     value of debug, now is 1/0
 *------------------------------------------------------------------------
 */
int WEB_Get_SSL_Debug(void)
{
    return web_task_debug_ssl;
}

/*------------------------------------------------------------------------
 * Function:    WEB_Set_Reboot
 * Purpose:     set system reboot
 * Parameters:
 *    Input:    None
 *    Output:   None
 * returns:     None
 *------------------------------------------------------------------------
 */
void WEB_Set_Reboot(void)
{
    rebootTime = OSTimeGet();
    bReboot = TRUE;
    return ;
}


void WEB_Set_Reset_Config(void)
{
    rebootTime = OSTimeGet();
    bResetConfig = TRUE;
    return ;
}
void WEB_Set_Reset_System(char *username)
{
    rebootTime = OSTimeGet();
    bResetSystem = TRUE;
    strcpy(USERNAME, username);
    return ;
}
/*-------------------------------------- end of program ------------------------------------------*/
#endif /*end of #if (WEB_SERVER_SUPPORT == 1)*/
