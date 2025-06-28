/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    CloudService.c

Abstract:

    Cloud Service of Interface Unit.


Environment:

        ARM RealView Developer Suite

Revision History:

    2016/12/07    Sean Create
*/



#include "general.h"
#include "board.h"
#include "lwipapi.h"
#include "p2pserver_api.h"
#include "rfiuapi.h"
#include "rtcapi.h"
#include "task.h"
#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn.h"
#if HOME_RF_SUPPORT
#include "MR8200def_homeautomation.h"
#include "../rfiu/inc/HomeRF.h"

#endif
#if CLOUD_SUPPORT

#define GETOWNER_HTTP_HEADER		"POST /api/getowner HTTP/1.1\r\n""Host: www.mars-cloud.com.tw:8001\r\n""Connection: keep-alive\r\n""User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36\r\n""Content-Type: text/plain;charset=UTF-8\r\n""Accept: */*\r\n""Accept-Encoding: gzip, deflate\r\n""Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n""Content-Length: %d\r\n\r\n%s"
#define GETONCEAUTH_HTTP_HEADER		"POST /auth/get_once_auth?alg=AES&encode=true HTTP/1.1\r\n""Connection: keep-alive\r\n""User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36\r\n""Content-Type: text/plain;charset=UTF-8\r\n""Accept: */*\r\n""Accept-Encoding: gzip, deflate\r\n""Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n""Content-Length: %d\r\n\r\n%s"
#define CLOUD_LOGIN_HEADER			"POST /auth/login? HTTP/1.1\r\n""Host: www.mars-cloud.com.tw:8001\r\n""Connection: keep-alive\r\n""User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36\r\n""Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n""Upgrade-Insecure-Requests: 1\r\n""Accept-Encoding: gzip,deflate,sdch\r\n""Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n""Accept-Charset: Big5,utf-8;q=0.7,*;q=0.3\r\n""Cache-Control: max-age=0\r\n""Content-Length: %d\r\n\r\n%s"
#define GETPAIRGATEWAY_HTTP_HEADER	"POST /api/usrinfo?method=adddatasrc HTTP/1.1\r\n""Host: www.mars-cloud.com.tw:8001\r\n""Connection: keep-alive\r\n""Authentication: Bearer %s\r\n""User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36\r\n""Content-Type: text/plain;charset=UTF-8\r\n""Accept: */*\r\n""Accept-Encoding: gzip, deflate\r\n""Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n""Content-Length: %d\r\n\r\n%s"
#define JSON_HTTP_HEADER_DATA 		"POST /api/put?data HTTP/1.1\r\n""Host: www.mars-cloud.com.tw:8001\r\n""Connection: keep-alive\r\n""Authentication: Bearer %s\r\n""User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36\r\n""Content-Type: text/plain;charset=UTF-8\r\n""Accept: */*\r\n""Accept-Encoding: gzip, deflate\r\n""Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n""Content-Length: %d\r\n\r\n%s"
#define JSON_HTTP_HEADER_EVENT 		"POST /api/put?event HTTP/1.1\r\n""Host: www.mars-cloud.com.tw:8001\r\n""Connection: keep-alive\r\n""Authentication: Bearer %s\r\n""User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36\r\n""Content-Type: text/plain;charset=UTF-8\r\n""Accept: */*\r\n""Accept-Encoding: gzip, deflate\r\n""Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n""Content-Length: %d\r\n\r\n%s"
#define CLOUD_ADD_DEVICE 			"POST /api/usrinfo?method=adddatasrc HTTP/1.1\r\n""Host: www.mars-cloud.com.tw:8001\r\n""Connection: keep-alive\r\n""Authentication: Bearer %s\r\n""User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36\r\n""Content-Type: text/plain;charset=UTF-8\r\n""Accept: */*\r\n""Accept-Encoding: gzip, deflate\r\n""Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n""Content-Length: %d\r\n\r\n%s"
#define CLOUD_DEL_DEVICE 			"POST /api/usrinfo?method=deldatasrc HTTP/1.1\r\n""Host: www.mars-cloud.com.tw:8001\r\n""Connection: keep-alive\r\n""Authentication: Bearer %s\r\n""User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36\r\n""Content-Type: text/plain;charset=UTF-8\r\n""Accept: */*\r\n""Accept-Encoding: gzip, deflate\r\n""Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n""Content-Length: %d\r\n\r\n%s"
#define CLOUD_VIDEO_HEADER_DATA 	"POST /stream/putstream?src=XP81X3JNTM1BVH79111A.1&type=http&codec=mp4&ivs=0&face=0 HTTP/1.1\r\n""Host: 192.168.1.55:20003\r\n""Connection: keep-alive\r\n""User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36\r\n""Content-Type: application/octet-stream\r\n""Accept: */*\r\n""Accept-Encoding: gzip, deflate\r\n""Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n\r\n"

#define sean_test 1
/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */

OS_STK cloudServiceTaskStack[CLOUD_TASK_STACK_SIZE];
OS_STK cloudVideoTaskStack[CLOUD_VIDEO_TASK_STACK_SIZE];


int  gFlagCloudLoginOK = 0;
char CLOUD_TOKEN[1024];
char CLOUD_OWNER[128];
char CLOUD_ACK[128];
char CLOUD_CMD[512];
u32  gCloudMsgTime = 0;
u32  gCloudsID = 0;
u8	 url[64] = "";
u8	 gVideo_task=0;
int  skt2;
u8	 aa2 = 0;

struct	sockaddr_in cloudVideoSrvAddr;
//struct sockaddr_in cloudServiceSrvAddr;

#if sean_test
static const char *JSON_STRING =
    "{\"user\": \"johndoe\", \"admin\": false, \"uid\": 1000,\n  "
    "\"groups\": [\"users\", \"wheel\", \"audio\", \"video\"]}";
#endif
/*
*********************************************************************************************************
* Extern Variables
*********************************************************************************************************
*/
extern OS_EVENT* Wait_Link_Status_Evt;    /* 等待網路線連線狀態 */
extern u8 * gUID;
extern int net_link_status;
extern u8  homeSwitchControl;

#if sean_test

extern s8  P2PEnableStreaming[1];
extern u32 P2PVideoBufReadIdx[1];
extern u32 P2PVideoPresentTime[1];
extern u32 P2PAudioBufReadIdx[1];
extern u32 P2PAudioPresentTime[1];
extern u32 P2PChannelStart[1];
extern VIDEO_BUF_MNG *P2PVideoBuf[1];
extern IIS_BUF_MNG   *P2PAudioBuf[1];
extern int P2P_AV_Source[1];
extern int gFirstConnect = 0;
extern int gPlaybackWidth;
extern int gPlaybackHeight;
extern unsigned char MPEG4_config[0x1d];
#endif
//extern void TTTT(void);

extern OS_EVENT    *dcfReadySemEvt;

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

//void SendCloudMessage(u32 sID, u32 target);
//static char *Get_JSON_MessageString(u32 sID, u8 *target);



/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */
#if 0//sean_test
int Sean_Test()
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[512]; /* We expect no more than 512 tokens */

    jsmn_init(&p);
    r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
    if (r < 0)
    {
        printf("Failed to parse JSON: %d\n", r);
        return 1;
    }

    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT)
    {
        printf("Object expected\n");
        return 1;
    }

    /* Loop over all keys of the root object */
    for (i = 1; i < r; i++)
    {
        if (jsoneq(JSON_STRING, &t[i], "user") == 0)
        {
            /* We may use strndup() to fetch string value */
            printf("- User: %.*s\n", t[i+1].end-t[i+1].start,
                   JSON_STRING + t[i+1].start);
            i++;
        }
        else if (jsoneq(JSON_STRING, &t[i], "admin") == 0)
        {
            /* We may additionally check if the value is either "true" or "false" */
            printf("- Admin: %.*s\n", t[i+1].end-t[i+1].start,
                   JSON_STRING + t[i+1].start);
            i++;
        }
        else if (jsoneq(JSON_STRING, &t[i], "uid") == 0)
        {
            /* We may want to do strtol() here to get numeric value */
            printf("- UID: %.*s\n", t[i+1].end-t[i+1].start,
                   JSON_STRING + t[i+1].start);
            i++;
        }
        else if (jsoneq(JSON_STRING, &t[i], "groups") == 0)
        {
            int j;
            printf("- Groups:\n");
            if (t[i+1].type != JSMN_ARRAY)
            {
                continue; /* We expect groups to be an array of strings */
            }
            for (j = 0; j < t[i+1].size; j++)
            {
                jsmntok_t *g = &t[i+j+2];
                printf("  * %.*s\n", g->end - g->start, JSON_STRING + g->start);
            }
            i += t[i+1].size + 1;
        }
        else
        {
            printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
                   JSON_STRING + t[i].start);
        }
    }
    return 0;
}
#endif

u8 CloudServiceInit(void)
{
    if(OSTaskCreate(CLOUD_TASK, CLOUD_TASK_PARAMETER, CLOUD_TASK_STACK, CLOUD_TASK_PRIORITY) != OS_NO_ERR)
    {
        DEBUG_P2P("OSTaskCreate Task_Cloud failed!!!!!!!!!!!!!!!!!!!\n");
        return;
    }
}


static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0)
    {
        return 0;
    }
    return -1;
}

u8 CloudGetSensorListIdx(u32 sUID)
{
    u8 idx;

    for(idx=0; idx<HOMERF_SENSOR_MAX; idx++)
    {
        if(gHomeRFSensorList->sSensor[idx].sUID.hi == sUID)
            break;
    }

    return idx;
}

void CloudCmdHandleEvent(u8 *cmdStr)
{
    u8 UUID[21] = "";
    u8 cmd[8] = "";
    //u8 url[64] = "";
    u32  sUID;
    char num_temp[10];
    u8 isPowerOn, slot;
    u8 idx;

    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[512]; /* expect no more than 512 tokens */


    /*********************************************************************/
    /**********************Start Parsing JSON Content*********************/
    /*********************************************************************/
    jsmn_init(&p);
    r = jsmn_parse(&p, cmdStr, strlen(cmdStr), t, sizeof(t)/sizeof(t[0]));
    if (r < 0)
    {
        printf("Failed to parse JSON: %d\n", r);
    }

    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT)
    {
        printf("Object expected\n");
    }

    /* Loop over all keys of the root object */
    for (i = 1; i < r; i++)
    {
        if (jsoneq(cmdStr, &t[i], "suid") == 0)
        {
            //printf("- suid: %.*s\n", t[i+1].end-t[i+1].start,
            //       cmdStr + t[i+1].start);
            sprintf(num_temp, "%.*s", t[i+1].end-t[i+1].start, cmdStr + t[i+1].start);
            sUID = strtol(num_temp, NULL, 10);
            //printf("!!!!sUID=%d=!!!\n",sUID);

            i++;
        }
        else if (jsoneq(cmdStr, &t[i], "uuid") == 0)
        {
            //printf("- uuid: %.*s\n", t[i+1].end-t[i+1].start,
            //       cmdStr + t[i+1].start);

            sprintf(UUID, "%.*s", t[i+1].end-t[i+1].start, cmdStr + t[i+1].start);
            //printf("!!!!UUID=%s=!!!\n",UUID);
            i++;
        }
        else if (jsoneq(cmdStr, &t[i], "isPowerOn") == 0)
        {
            //printf("- isPowerOn: %.*s\n", t[i+1].end-t[i+1].start,
            //       cmdStr + t[i+1].start);
            memset(num_temp, 0, sizeof(num_temp));
            sprintf(num_temp, "%.*s", t[i+1].end-t[i+1].start, cmdStr + t[i+1].start);
            isPowerOn = strtol(num_temp, NULL, 10);

            //printf("!!!!isPowerOn=%d=!!!\n",isPowerOn);

            i++;
        }
        else if (jsoneq(cmdStr, &t[i], "slot") == 0)  //use global value to setup in future.
        {
            memset(num_temp, 0, sizeof(num_temp));
            sprintf(num_temp, "%.*s", t[i+1].end-t[i+1].start, cmdStr + t[i+1].start);
            slot = strtol(num_temp, NULL, 10);

            //printf("!!!!slot=%d=!!!\n",slot);

            i++;
        }
        else if (jsoneq(cmdStr, &t[i], "status") == 0)
        {
            //printf("- status: %.*s\n", t[i+1].end-t[i+1].start,
            //       cmdStr + t[i+1].start);
            i++;
        }
        else if (jsoneq(cmdStr, &t[i], "cmd") == 0)
        {
            sprintf(cmd, "%.*s", t[i+1].end-t[i+1].start, cmdStr + t[i+1].start);
            //printf("!!!!cmd=%s=!!!\n",cmd);
            i++;
        }
        else if (jsoneq(cmdStr, &t[i], "url") == 0)
        {
            sprintf(url, "%.*s", t[i+1].end-t[i+1].start, cmdStr + t[i+1].start);
            //printf("!!!!url=%s=!!!\n",url);
            i++;
        }
        else if (jsoneq(cmdStr, &t[i], "owner") == 0)
        {
            sprintf(CLOUD_OWNER, "%.*s", t[i+1].end-t[i+1].start, cmdStr + t[i+1].start);
            //printf("!!!!owner=%s=!!!\n",CLOUD_OWNER);
            i++;
        }

    }

    /*********************************************************************/
    /*************************Start Handle Event**************************/
    /*********************************************************************/

    if(strcmp(cmd, "set") == 0)
    {
        idx = CloudGetSensorListIdx(sUID);

        if(idx == HOMERF_SENSOR_MAX)
        {
            DEBUG_P2P("SENSOR_MAX, Do nothing.\n");
            goto End;
        }
        if(gHomeRFSensorList->sSensor[idx].type == HOMERF_DEVICE_SWITCH)
        {
#if 0 //old setting.
            if(isPowerOn == TRUE)
            {
                printf("\x1B[96m Ready to switch %d\x1B[0m\n",isPowerOn);
                homeRFSendToSensor(HOMERF_SEND_SWITCH_ON,idx);
            }
            else
                homeRFSendToSensor(HOMERF_SEND_SWITCH_OFF,idx);
#else
            printf("\x1B[96m Ready to switch %d:%d\x1B[0m\n",slot,isPowerOn);
            if(isPowerOn == TRUE)
                homeSwitchControl = (gHomeRFSensorList->sSensor[idx].data.SWITCH.isPowerOn)|(0x01 << slot);
            else
                homeSwitchControl = (gHomeRFSensorList->sSensor[idx].data.SWITCH.isPowerOn)&~(0x01 << slot);
            printf("\x1B[96m homeSwitchControl=%d \x1B[0m\n",homeSwitchControl);
            homeRFSendToSensor(HOMERF_SEND_SWITCH_ON,idx);
#endif

        }
    }
    else if(strcmp(cmd, "request") == 0)
    {
        //printf("\x1B[91m%s, %s\x1B[0m\n",cmd, url);
        if(url == NULL)
        {
            DEBUG_P2P("InVaild URL, Do nothing.\n");
        }
        else
        {
#if 0

            if(gVideo_task == 0)
            {
                if(OSTaskCreate(CLOUD_VIDEO_TASK, CLOUD_VIDEO_TASK_PARAMETER, CLOUD_VIDEO_TASK_STACK, CLOUD_VIDEO_TASK_PRIORITY) != OS_NO_ERR)
                {
                    DEBUG_P2P("OSTaskCreate Task_Cloud_Video failed!\n");
                }
                else
                    gVideo_task = 1;
            }
            else
            {
                //OSTaskDel(CLOUD_VIDEO_TASK_PRIORITY);
                //gVideo_task = 0;
                //Stop_P2P_Session(0);
            }
#endif
        }
    }
End:
}

u8 SendCloudGetOwner()
{
    int skt;
    int on=1, bytesRecv;
    char msg[1024];
    char GETOWNER_CONTENT_DATA[64];
    int Content_Len;
    char hostname[64]="";
    char hostname_IP[32]="";
    struct sockaddr_in cloudGetOwnerSrvAddr;
    u8 ack_ret;

    strcpy(hostname,CLOUD_SERVER);
    DN2IP(hostname,hostname_IP);

    cloudGetOwnerSrvAddr.sin_addr.s_addr=inet_addr(hostname_IP);
    cloudGetOwnerSrvAddr.sin_port = htons(PORT_CLOUD_PAIR);
    cloudGetOwnerSrvAddr.sin_family = AF_INET;

    if ((skt = (int)socket(AF_INET, SOCK_STREAM, 0)) >= 0)
    {
        setsockopt(skt,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(on));/*Check the TCP connection whether alive.*/
        if (connect(skt, (struct sockaddr *)&cloudGetOwnerSrvAddr, sizeof(struct sockaddr_in)) == 0)
        {
            DEBUG_P2P("Cloud Get Owner...\n");

            sprintf(GETOWNER_CONTENT_DATA,	"{"
                    "\"uuid\":\"%s\""
                    "}",
                    gUID);

            Content_Len = strlen(GETOWNER_CONTENT_DATA);

            memset(msg, 0, sizeof(msg));
            sprintf(msg, GETOWNER_HTTP_HEADER, Content_Len,GETOWNER_CONTENT_DATA);

            if(send(skt, msg, strlen(msg), 0)<0)
            {
                DEBUG_P2P("Send Cloud MSG(Get Owner) Fail.\n");
            }
            else
            {
                DEBUG_P2P("Send Cloud MSG(Get Owner) Success.\n");

                memset(CLOUD_ACK, 0, sizeof(CLOUD_ACK));
                bytesRecv = recv(skt, CLOUD_ACK, sizeof(CLOUD_ACK), 0);  //200 OK

                ack_ret = ParsingHTTPHeader(CLOUD_ACK, strlen(CLOUD_ACK));
                if(ack_ret != 2)
                {
                    DEBUG_P2P("%s\n",CLOUD_ACK);
                    close(skt);
                    return 0;
                }

                memset(CLOUD_OWNER,0,sizeof(CLOUD_OWNER));
                bytesRecv = recv(skt, CLOUD_OWNER, 128, 0);  //OWNER
                CloudCmdHandleEvent(CLOUD_OWNER);
                //DEBUG_P2P("TOKEN lenth2 : %d\n%s\n", strlen(CLOUD_OWNER), CLOUD_OWNER);

                /*===========================*/
                /*===========================*/
                /*===Still Need Owner Save===*/
                /*===========================*/
                /*===========================*/
            }
        }
        else
        {
            DEBUG_P2P("Couldn't LogIn Cloud Server(Get Owner)\n");
            return 0;
        }
        close(skt);
        return 1;
    }
    else
    {
        DEBUG_P2P("Create Get Owner socket fail.\n");
        return 0;
    }
}

u8 SendCloudGetOnceAuth()
{
    int skt;
    int on=1, bytesRecv;
    char msg[512];
    char GETONCEAUTH_CONTENT_DATA[64];
    int Content_Len;
    int token_lenth;

    struct sockaddr_in cloudGetOnceAuthSrvAddr;
    char hostname[64]="";
    char hostname_IP[32]="";
    u8 ack_ret;

    strcpy(hostname,CLOUD_SERVER);
    DN2IP(hostname,hostname_IP);

    cloudGetOnceAuthSrvAddr.sin_addr.s_addr=inet_addr(hostname_IP);
    cloudGetOnceAuthSrvAddr.sin_port = htons(PORT_CLOUD_PAIR);
    cloudGetOnceAuthSrvAddr.sin_family = AF_INET;

    if ((skt = (int)socket(AF_INET, SOCK_STREAM, 0)) >= 0)
    {
        setsockopt(skt,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(on));/*Check the TCP connection whether alive.*/
        if (connect(skt, (struct sockaddr *)&cloudGetOnceAuthSrvAddr, sizeof(struct sockaddr_in)) == 0)
        {
            DEBUG_P2P("Cloud Get Once Auth...\n");

            sprintf(GETONCEAUTH_CONTENT_DATA,	"{"
                    "\"usr\":\"%s\","
                    "\"ekey\":\"test\""
                    "}",CLOUD_OWNER);

            Content_Len = strlen(GETONCEAUTH_CONTENT_DATA);

            memset(msg, 0, sizeof(msg));
            sprintf(msg, GETONCEAUTH_HTTP_HEADER, Content_Len,GETONCEAUTH_CONTENT_DATA);

            if(send(skt, msg, strlen(msg), 0)<0)
            {
                DEBUG_P2P("Send Cloud MSG(Once Auth) Fail.\n");
            }
            else
            {
                DEBUG_P2P("Send Cloud MSG(Once Auth) Success.\n");

                memset(CLOUD_ACK,0,sizeof(CLOUD_ACK));
                bytesRecv = recv(skt, CLOUD_ACK, sizeof(CLOUD_ACK), 0);  //200 OK

                ack_ret = ParsingHTTPHeader(CLOUD_ACK, strlen(CLOUD_ACK));
                if(ack_ret != 2)
                {
                    DEBUG_P2P("%s\n",CLOUD_ACK);
                    close(skt);
                    return 0;
                }

                memset(CLOUD_TOKEN,0,sizeof(CLOUD_TOKEN));
                bytesRecv = recv(skt, CLOUD_TOKEN, 1024, 0);  //CLOUD_ONCE_TOKEN
                //printf("@@@%s@@@\n",CLOUD_TOKEN);
                //DEBUG_P2P("TOKEN lenth3 : %d\n%s\n", strlen(CLOUD_TOKEN), CLOUD_TOKEN);

            }
        }
        else
        {
            DEBUG_P2P("Couldn't LogIn Cloud Server(Once Auth)\n");
            return 0;
        }
        close(skt);
        return 1;
    }
    else
    {
        DEBUG_P2P("Create Get Owner socket fail.\n");
        return 0;
    }
}

u8 SendCloudPairGateway()
{
    int skt;
    int on=1;
    char msg[1024];
    char PAIRGATEWAY_CONTENT_DATA[512];
    int Content_Len;
    int token_lenth;

    struct sockaddr_in cloudPairGatewaySrvAddr;
    char hostname[64]="";
    char hostname_IP[32]="";
    u8 ack_ret;

    strcpy(hostname,CLOUD_SERVER);
    DN2IP(hostname,hostname_IP);

    cloudPairGatewaySrvAddr.sin_addr.s_addr=inet_addr(hostname_IP);
    cloudPairGatewaySrvAddr.sin_port = htons(PORT_CLOUD_PAIR);
    cloudPairGatewaySrvAddr.sin_family = AF_INET;

    if ((skt = (int)socket(AF_INET, SOCK_STREAM, 0)) >= 0)
    {
        setsockopt(skt,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(on));/*Check the TCP connection whether alive.*/
        if (connect(skt, (struct sockaddr *)&cloudPairGatewaySrvAddr, sizeof(struct sockaddr_in)) == 0)
        {
            DEBUG_P2P("Cloud Get PairGateway...\n");

            sprintf(PAIRGATEWAY_CONTENT_DATA,	"{"
                    "\"name\":\"%s\","
                    "\"uuid\":\"%s\","
                    "\"vender_id\":\"mars-semi\""
                    "}",gUID,gUID);

            Content_Len = strlen(PAIRGATEWAY_CONTENT_DATA);

            memset(msg, 0, sizeof(msg));
            sprintf(msg, GETPAIRGATEWAY_HTTP_HEADER, CLOUD_TOKEN, Content_Len,PAIRGATEWAY_CONTENT_DATA);

            if(send(skt, msg, strlen(msg), 0)<0)
            {
                DEBUG_P2P("Send Cloud MSG(Pair Gateway) Fail.\n");
                return 0;
            }
            else
            {
                memset(CLOUD_ACK, 0, sizeof(CLOUD_ACK));
                recv(skt, CLOUD_ACK, sizeof(CLOUD_ACK), 0);  //200 OK

                ack_ret = ParsingHTTPHeader(CLOUD_ACK, strlen(CLOUD_ACK));
                if(ack_ret != 2)
                {
                    DEBUG_P2P("%s\n",CLOUD_ACK);
                    close(skt);
                    return 0;
                }

                DEBUG_P2P("Send Cloud MSG(Pair Gateway) Success.\n");
            }
        }
        else
        {
            DEBUG_P2P("Couldn't LogIn Cloud Server(Pair Gateway)\n");
            return 0;
        }
        close(skt);
        return 1;
    }
    else
    {
        DEBUG_P2P("Create Get Owner socket fail.\n");
        return 0;
    }
}

u8 SendCloudLogInReg()
{
    int skt;
    int on=1, bytesRecv, i;
    char msg[1024];
    char LOGIN_CONTENT_DATA[512];
    int Content_Len;
    struct sockaddr_in cloudLogInSrvAddr;
    char hostname[64]="";
    char hostname_IP[32]="";
    u8 ack_ret;

    strcpy(hostname,CLOUD_SERVER);
    DN2IP(hostname,hostname_IP);

    cloudLogInSrvAddr.sin_addr.s_addr=inet_addr(hostname_IP);
    cloudLogInSrvAddr.sin_port = htons(PORT_CLOUD_PAIR);
    cloudLogInSrvAddr.sin_family = AF_INET;

    if ((skt = (int)socket(AF_INET, SOCK_STREAM, 0)) >= 0)
    {
        setsockopt(skt,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(on));/*Check the TCP connection whether alive.*/
        if (connect(skt, (struct sockaddr *)&cloudLogInSrvAddr, sizeof(struct sockaddr_in)) == 0)
        {
            DEBUG_P2P("Cloud LogIn...\n");

            sprintf(LOGIN_CONTENT_DATA,	"{"
                    "\"usr\":\"%s\","
                    "\"pwd\":\"%s\""
                    "}",CLOUD_OWNER,CLOUD_TOKEN);
            Content_Len = strlen(LOGIN_CONTENT_DATA);

            memset(msg, 0, sizeof(msg));
            sprintf(msg, CLOUD_LOGIN_HEADER, Content_Len,LOGIN_CONTENT_DATA);
//printf("%s\n",msg);
            send(skt, msg,strlen(msg), 0);

            memset(CLOUD_ACK, 0, sizeof(CLOUD_ACK));
            bytesRecv = recv(skt, CLOUD_ACK, sizeof(CLOUD_ACK), 0);  //200 OK

            ack_ret = ParsingHTTPHeader(CLOUD_ACK, strlen(CLOUD_ACK));
            if(ack_ret != 2)
            {
                DEBUG_P2P("ret = %d\n%s\n",ack_ret, CLOUD_ACK);
                close(skt);
                return 0;
            }

            bytesRecv = recv(skt, CLOUD_TOKEN, 1024, 0);  //Token


            if(bytesRecv < 0 )
            {
                DEBUG_P2P("Cloud LogIn Fail.\n");
                return 0;
            }
            else
            {
                DEBUG_P2P("Cloud LogIn Success.\n");
            }
        }
        else
        {
            DEBUG_P2P("Couldn't LogIn Cloud Server\n");
            return 0;
        }
        close(skt);
        return 1;
    }
    else
    {
        DEBUG_P2P("Create socket fail.\n");
        return 0;
    }
}

static char *Get_JSON_MessageString(u32 sID, u8 *target)
{
    HOMERF_SENSOR_DATA SENSOR_DATA;
    RTC_DATE_TIME   localTime;
    RTC_TIME_ZONE   localzone;
    u32 LocalTimeInSec;
    char msg[2048];
    char JSON_CONTENT_DATA[768];
    char JSON_CONTENT_DATA_TYPE[256];
    char JSON_Sensor_Name[17];
    char dataset[20] = "";
    char id[21] = "";
    char local_time[20];
    char local_zone_time[4];
    int Content_Len;

    sysGetSensorData(sID, &SENSOR_DATA);
    RTC_Get_Time(&localTime);
    RTC_Get_TimeZone(&localzone);
    LocalTimeInSec=RTC_Time_To_Second(&localTime)+946684800;
    strcpy(JSON_Sensor_Name,SENSOR_DATA.name);
    strcpy(id, gUID);
    sprintf(local_time,"%lu", LocalTimeInSec);
    strcat(local_time, "000/");
    strcat(local_time, (localzone.operator)?"-":"+");
    sprintf(local_zone_time, "%d", localzone.hour);
    strcat(local_time, local_zone_time);

    if(sID == 0) /* Camera Motion Trigger 未完成*/
    {
        sprintf(JSON_CONTENT_DATA,    "{"
                "\"cmd\":\"put\","
                "\"target\":\"data\","
                "\"id\":\"%s\","
                "\"values\":"
                "["
                "{"
                "\"local_time\":\"%s\","
                "\"name\":\"camera\","
                "\"status\":\"1\","
                "\"type\":\"13\""
                "}]}",
                id,
                local_time);

        Content_Len = strlen(JSON_CONTENT_DATA);
        sprintf(msg, JSON_HTTP_HEADER_DATA,Content_Len,JSON_CONTENT_DATA);

        return msg;
    }
    printf("\x1B[96m suid:%d \x1B[0m\n", SENSOR_DATA.sUID.hi);
    //"\"target\":\"event\"," 觸發型sensor需要再多傳一筆event來作推送,data僅存資料
    //"\"msg\":\"%s\"," (option)
    sprintf(JSON_CONTENT_DATA,  "{"
            "\"cmd\":\"put\","
            "\"target\":\"%s\","
            "\"uuid\":\"%s\","
            "\"suid\":\"%d\","
            "\"values\":"
            "["
            "{"
            "\"local_time\":\"%s\","
            "\"name\":\"%s\","
            "\"battery\":\"%d\","
            "\"pushOnOff\":\"%d\", "
            "\"sirenOnOff\":\"%d\","
            "\"status\":\"%d\","
            "\"maxSubSensor\":\"%d\","
            "\"alarmTimer\":\"%d\","
            "\"majorVer\":\"%d\","
            "\"minorVer\":\"%d\","
            "\"lifeCount\":\"%d\","
            "\"type\":\"%d\","
            "\"sID\":\"%d\"",
            target,
            id,
            SENSOR_DATA.sUID.hi,
            local_time,
            JSON_Sensor_Name,
            SENSOR_DATA.battery,
            SENSOR_DATA.pushOnOff,
            SENSOR_DATA.sirenOnOff,
            SENSOR_DATA.status,
            SENSOR_DATA.maxSubSensor,
            SENSOR_DATA.alarmTimer,
            SENSOR_DATA.majorVer,
            SENSOR_DATA.minorVer,
            SENSOR_DATA.lifeCount,
            SENSOR_DATA.type,
            SENSOR_DATA.sID);
    switch(SENSOR_DATA.type)
    {
    case(HOMERF_DEVICE_GAS):
    case(HOMERF_DEVICE_LEAK):
    case(HOMERF_DEVICE_PLUG):
    case(HOMERF_DEVICE_SMOKE):
    case(HOMERF_DEVICE_VIBRATE):
    case(HOMERF_DEVICE_PIR):
        sprintf(JSON_CONTENT_DATA_TYPE, "}]}");
        break;

    case(HOMERF_DEVICE_DOOR):
        sprintf(JSON_CONTENT_DATA_TYPE, ",\"data\":{\"isOpen\":%d}}]}",SENSOR_DATA.data.door.isOpen);
        break;

    case(HOMERF_DEVICE_IAQ):
        sprintf(JSON_CONTENT_DATA_TYPE, ",\"data\":{\"value\":%d}}]}",SENSOR_DATA.data.IAQ.value);
        break;

    case(HOMERF_DEVICE_ADE): //
        //sprintf(JSON_CONTENT_DATA_TYPE, ",\"data\":{\"isPowerOn\":\"%d\"}}]}",SENSOR_DATA.data.ADE.isPowerOn);
        break;

    case(HOMERF_DEVICE_FDS):
        sprintf(JSON_CONTENT_DATA_TYPE, ",\"data\":{\"Accel_X_H\":%d,"
                "\"Accel_X_L\":%d,"
                "\"Accel_Y_H\":%d,"
                "\"Accel_Y_L\":%d,"
                "\"Accel_Z_H\":%d,"
                "\"Accel_Z_L\":%d,"
                "\"GYRO_X_H\":%d,"
                "\"GYRO_X_L\":%d,"
                "\"GYRO_Y_H\":%d,"
                "\"GYRO_Y_L\":%d,"
                "\"GYRO_Z_H\":%d,"
                "\"GYRO_Z_L\":%d,"
                "\"RSSI\":%d}}]}"
                ,SENSOR_DATA.data.FDS.Accel_X_H
                ,SENSOR_DATA.data.FDS.Accel_X_L
                ,SENSOR_DATA.data.FDS.Accel_Y_H
                ,SENSOR_DATA.data.FDS.Accel_Y_L
                ,SENSOR_DATA.data.FDS.Accel_Z_H
                ,SENSOR_DATA.data.FDS.Accel_Z_L
                ,SENSOR_DATA.data.FDS.GYRO_X_H
                ,SENSOR_DATA.data.FDS.GYRO_X_L
                ,SENSOR_DATA.data.FDS.GYRO_Y_H
                ,SENSOR_DATA.data.FDS.GYRO_Y_L
                ,SENSOR_DATA.data.FDS.GYRO_Z_H
                ,SENSOR_DATA.data.FDS.GYRO_Z_L
                ,SENSOR_DATA.data.FDS.RSSI
               );
        break;

    case(HOMERF_DEVICE_SWITCH):
        sprintf(JSON_CONTENT_DATA_TYPE, ",\"data\":{\"isPowerOn\":%d}}]}",SENSOR_DATA.data.SWITCH.isPowerOn);
        break;

    case(HOMERF_DEVICE_TEMP_HYG):
        sprintf(JSON_CONTENT_DATA_TYPE, ",\"data\":{\"Thermometer_H\":%d,"
                "\"Thermometer_L\":%d,"
                "\"Hygrometer_H\":%d,"
                "\"Hygrometer_L\":%d}}]}"
                ,SENSOR_DATA.data.Temp_HYG.Thermometer_H
                ,SENSOR_DATA.data.Temp_HYG.Thermometer_L
                ,SENSOR_DATA.data.Temp_HYG.Hygrometer_H
                ,SENSOR_DATA.data.Temp_HYG.Hygrometer_L
               );
        break;

    default:
        sprintf(JSON_CONTENT_DATA_TYPE, "}]}");
        break;
    }
    strcat(JSON_CONTENT_DATA, JSON_CONTENT_DATA_TYPE);

    Content_Len = strlen(JSON_CONTENT_DATA);
    memset(msg, 0, sizeof(msg));

    if(strcmp(target, "data") == 0)
        sprintf(msg, JSON_HTTP_HEADER_DATA,CLOUD_TOKEN, Content_Len,JSON_CONTENT_DATA);
    else
        sprintf(msg, JSON_HTTP_HEADER_EVENT,CLOUD_TOKEN, Content_Len,JSON_CONTENT_DATA);

    return msg;
}


void SendCloudMessage(u32 sID, u32 target) /* target 0:DATA, 1:EVENT*/
{
    int skt;
    char *msg;
    int on=1;

    struct sockaddr_in cloudSrvAddr;
    char hostname[64]="";
    char hostname_IP[32]="";
    u8 ack_ret;
    u32 current_time;

    if(gCloudsID != sID)
    {
        gCloudsID = sID;
        gCloudMsgTime = OSTimeGet();
    }
    else
    {
        current_time=OSTimeGet();
        if((current_time - gCloudMsgTime) < 60)
            goto End;
        else
            gCloudMsgTime = OSTimeGet();
    }


    strcpy(hostname,CLOUD_SERVER);
    DN2IP(hostname,hostname_IP);

    if(strlen(CLOUD_TOKEN) != 0)
    {
        cloudSrvAddr.sin_addr.s_addr=inet_addr(hostname_IP);
        cloudSrvAddr.sin_port = htons(PORT_CLOUD);
        cloudSrvAddr.sin_family = AF_INET;

        if ((skt = (int)socket(AF_INET, SOCK_STREAM, 0)) >= 0)
        {
            setsockopt(skt,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(on));/*Check the TCP connection whether alive.*/
            if (connect(skt, (struct sockaddr *)&cloudSrvAddr, sizeof(struct sockaddr_in)) == 0)
            {
                DEBUG_P2P("Sending cloud MSG\n");
                if(target == TARGET_DATA)
                    msg = Get_JSON_MessageString(sID, "data");
                else if(target == TARGET_EVENT)
                    msg = Get_JSON_MessageString(sID, "event");

                if(send(skt, msg, strlen(msg), 0)<0)
                {
                    DEBUG_P2P("Send Cloud MSG Fail.\n");
                }
                else
                {
                    DEBUG_P2P("Send Cloud MSG Event Success.\n");
                    memset(CLOUD_ACK, 0, sizeof(CLOUD_ACK));
                    recv(skt, CLOUD_ACK, sizeof(CLOUD_ACK), 0);  //200 OK

                    ack_ret = ParsingHTTPHeader(CLOUD_ACK, strlen(CLOUD_ACK));
                    if(ack_ret != 2)
                    {
                        DEBUG_P2P("%s\n",CLOUD_ACK);
                    }

                }
            }
            else
            {
                DEBUG_P2P("Couldn't send Cloud MSG\n");
            }
            close(skt);
        }
        else
        {
            DEBUG_P2P("Create socket fail.\n");
        }
    }
    else
    {
        DEBUG_P2P("No Cloud Server Connect.\n");
    }
End:
}

static char *Get_JSON_ADD_DEL(u32 sID, u8 sType, u8 isCamera) /* sType:0 DEL ; sType:1 ADD*/
{
    HOMERF_SENSOR_DATA SENSOR_DATA;
    char msg[2048];
    char JSON_CONTENT_DATA[128];
    char JSON_CONTENT_DATA_TYPE[50];
    char JSON_Sensor_Name[17];
    char id[21] = "";
    int Content_Len;

    sysGetSensorData(sID, &SENSOR_DATA);
    strcpy(JSON_Sensor_Name,SENSOR_DATA.name);
    strcpy(id, gUID);

    if(isCamera != 0)
    {
        sprintf(JSON_CONTENT_DATA,  "{"
                "\"name\":\"%s\","
                "\"uuid\":\"%s\","
                "\"suid\":\"%d\","
                "\"data_profile\":\"both.IPcamera\"",
                isCamera,
                id,
                isCamera);
    }
    else
    {
        sprintf(JSON_CONTENT_DATA,  "{"
                "\"name\":\"%s\","
                "\"uuid\":\"%s\","
                "\"suid\":\"%d\","
                "\"data_profile\":\"in.%d\"",
                JSON_Sensor_Name,
                id,
                SENSOR_DATA.sUID.hi,
                SENSOR_DATA.type);
    }
    switch(sType)
    {
    case 0:
        sprintf(JSON_CONTENT_DATA_TYPE, "}");
        break;
    case 1:
        sprintf(JSON_CONTENT_DATA_TYPE, ",\"desc\":\"\"}");
        break;

    default:
        sprintf(JSON_CONTENT_DATA_TYPE, "}");
        break;
    }
    strcat(JSON_CONTENT_DATA, JSON_CONTENT_DATA_TYPE);

    Content_Len = strlen(JSON_CONTENT_DATA);

    memset(msg, 0, sizeof(msg));
    if (sType)
        sprintf(msg, CLOUD_ADD_DEVICE,CLOUD_TOKEN, Content_Len,JSON_CONTENT_DATA);
    else
        sprintf(msg, CLOUD_DEL_DEVICE,CLOUD_TOKEN, Content_Len,JSON_CONTENT_DATA);
    return msg;
}

void SendCloud_ADD_DEL_Message(u32 sID, u8 sType, u8 isCamera) /* sType:0 DEL ; sType:1 ADD*/
{
    int skt;
    char *msg;
    int ret;
    int on=1,temp;

    struct sockaddr_in cloudSrvAddr;
    char hostname[64]="";
    char hostname_IP[32]="";
    u8 ack_ret;

    if(net_link_status!=NET_LINK_ON)
        goto End;

    strcpy(hostname,CLOUD_SERVER);
    DN2IP(hostname,hostname_IP);

    memset(&cloudSrvAddr, 0, sizeof(struct sockaddr_in));
    cloudSrvAddr.sin_family = AF_INET;
    cloudSrvAddr.sin_addr.s_addr=inet_addr(hostname_IP);
    cloudSrvAddr.sin_port = htons(PORT_CLOUD_PAIR);

    if(strlen(CLOUD_TOKEN) < 10)
    {
        OSTimeDly(100);
        SendCloudLogInReg();
    }
    if ((skt = (int)socket(AF_INET, SOCK_STREAM, 0)) >= 0)
    {
        ret=setsockopt(skt,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(on));/*Check the TCP connection whether alive.*/
        if (connect(skt, (struct sockaddr *)&cloudSrvAddr, sizeof(struct sockaddr_in)) == 0)
        {
            DEBUG_P2P("Sending cloud MSG\n");
            msg = Get_JSON_ADD_DEL(sID, sType, isCamera);
            if(send(skt, msg, strlen(msg), 0)<0)
            {
                DEBUG_P2P("Send Cloud ADD_DEL MSG Fail.\n");
                //SendCloud_ADD_DEL_Message(sID, sType);
                ret=-1;
            }
            else
            {
                memset(CLOUD_ACK, 0, sizeof(CLOUD_ACK));
                recv(skt, CLOUD_ACK, sizeof(CLOUD_ACK), 0);  //200 OK
                ack_ret = ParsingHTTPHeader(CLOUD_ACK, strlen(CLOUD_ACK));
                if(ack_ret != 2)
                {
                    DEBUG_P2P("%s\n",CLOUD_ACK);
                }

                DEBUG_P2P("Send Cloud ADD_DEL MSG Success.\n");
                ret=0;
            }
        }
        else
        {
            DEBUG_P2P("Couldn't send Cloud ADD_DEL MSG\n");
            ret=-1;
        }
        close(skt);
    }
    else
    {
        DEBUG_P2P("Create socket fail.\n");
    }
End:
}

void SendCloud_SYNC_Message(void)
{
    int skt;
    char *msg;
    int ret;
    int on=1,temp;

    u8 ack_ret, i;

    if(net_link_status!=NET_LINK_ON)
        goto End; // Do nothing.

    if(strlen(CLOUD_TOKEN) < 10)
    {
        OSTimeDly(100);
        SendCloudLogInReg();
    }

    DEBUG_P2P("Sending cloud Sync MSG\n");
    for(i=0; i<HOMERF_SENSOR_MAX; i++)
    {
        if((gHomeRFSensorList->sSensor[i].sID !=0) && (gHomeRFSensorList->sSensor[i].sID !=0xffffffff))
        {
            SendCloud_ADD_DEL_Message(gHomeRFSensorList->sSensor[i].sID, 1, 0);
        }
    }

End:
}

void CloudServiceTask(void* pData)
{
    INT8U	err;
    char hostname[64]="";
    char hostname_IP[32]="";
    int skt;
    int on=1, bytesRecv;
    char CLOUDSERVICE_CONTENT_DATA[64];
    char msg[1024];
    int Content_Len;
    int keepidle 	= 600;
    u8 *body;
    struct sockaddr_in cloudServiceSrvAddr;


    //OSSemPend(Wait_Link_Status_Evt,OS_IPC_WAIT_FOREVER, &err);  //20160201 Sean Wait Untill Network Link Status

Start:

    /* LogIn Cloud Server. */
    while(1)
    {
        //printf("\x1B[96m %d, %d \x1B[0m\n",gFlagCloudLoginOK,net_link_status);
        if(gFlagCloudLoginOK && (net_link_status==NET_LINK_ON))
        {
            DEBUG_P2P("\x1B[96mLogin Cloud Server.\x1B[0m\n");
            if(SendCloudGetOwner())
                if(SendCloudGetOnceAuth())
                    if(SendCloudPairGateway())
                        if(SendCloudLogInReg()) //Get Cloud Token
                        {
                            SendCloud_SYNC_Message();
                            gFlagCloudLoginOK = 0;
                            break;
                        }
        }
        //OSTimeDly(600); //30sec
        OSTimeDly(100);
    }

    SendCloud_ADD_DEL_Message(0, 1, 1); //Test Add 1 Camera
#if 1

    strcpy(hostname,CLOUD_SERVER);
    DN2IP(hostname,hostname_IP);

    cloudServiceSrvAddr.sin_addr.s_addr=inet_addr(hostname_IP);
    cloudServiceSrvAddr.sin_port = htons(PORT_CLOUD_SERVICE);
    cloudServiceSrvAddr.sin_family = AF_INET;


    DEBUG_P2P("Connecting to Cloud Server...\n");
Reconnect:
    if ((skt = (int)socket(AF_INET, SOCK_STREAM, 0)) >= 0)
    {
        setsockopt(skt,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(int));/*Check the TCP connection whether alive.*/
        setsockopt(skt,IPPROTO_TCP,TCP_NODELAY, &on, sizeof(int));
        setsockopt(skt,IPPROTO_TCP,TCP_KEEPALIVE, &on, sizeof(int));

        if (connect(skt, (struct sockaddr *)&cloudServiceSrvAddr, sizeof(struct sockaddr_in)) == 0)
        {

            sprintf(CLOUDSERVICE_CONTENT_DATA,	"{"
                    "\"cmd\":\"subscribe\","
                    "\"target\":\"command\","
                    "\"src\":\"%s.*\""
                    "}",
                    gUID);
            Content_Len = strlen(CLOUDSERVICE_CONTENT_DATA);

            memset(msg, 0, sizeof(msg));
            sprintf(msg, JSON_HTTP_HEADER_DATA, CLOUD_TOKEN, Content_Len, CLOUDSERVICE_CONTENT_DATA);

            //printf("\x1B[92m%s\x1B[0m\n",msg);
            if(send(skt, msg, strlen(msg), 0)<0)
            {
                DEBUG_P2P("Send Cloud MSG Fail.\n");
            }

            printf("\x1B[96mReady to recv! \x1B[0m\n");
            while(1)
            {
                memset(CLOUD_CMD, 0, sizeof(CLOUD_CMD));
                bytesRecv = recv(skt, CLOUD_CMD, sizeof(CLOUD_CMD), 0);
                //printf("\x1B[96m%s\x1B[0m\n",CLOUD_CMD);

                if(ParsingHTTPHeader(CLOUD_CMD, bytesRecv) == 4)
                {
                    printf("\x1B[96m%s\x1B[0m\n",CLOUD_CMD);
                    continue;
                }

                if(bytesRecv <= 0)
                {
                    printf("reconnect!\n");
                    close(skt);
                    OSTimeDly(20);
                    goto Reconnect;
                }
                body = strstr(CLOUD_CMD, "{") ; //locate to JSON start.
                if(body != NULL)
                {
                    //printf("\x1B[96m %d: %s \x1B[0m\n",bytesRecv, body);
                    CloudCmdHandleEvent(body);
                }
            }
        }
        else
        {
            DEBUG_P2P("Couldn't LogIn Cloud Service\n");
        }
    }
    else
    {
        DEBUG_P2P("Create Cloud Service socket fail.\n");
    }
#endif

}

#define split_size 1460

void CloudVideoTask(void* pData)
{
    //struct	sockaddr_in cloudVideoSrvAddr;
    char	hostname_IP[32]="";
    int 	keepidle=600;
    u8		on=1;
////////////////////////////////////
    u16 video_value[4]= {0};
    u16 video_value_max[4]= {0};
    u16 audio_value[4]= {0};
    u16 audio_value_max[4]= {0};
    u8	ch = 0;
    int i;
    u8	VOLSend;
    u8  temp[512]="POST /stream/putstream?src=XP81X3JNTM1BVH79111A.1&type=http&codec=mp4&ivs=0&face=0 HTTP/1.1\r\n""Host: 192.168.1.55:20003\r\n""Content-Length: 999999999\r\n""Connection: Keep-Alive\r\n""Keep-Alive: 30000\r\n""User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36\r\n""Content-Type: application/octet-stream\r\n""Accept: */*\r\n""Accept-Encoding: gzip, deflate\r\n""Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n\r\n";
    u8	bb;
    u32 video_size,video_temp;
    int DlyFlag,RunCount;
    int cnt,ret;
    int SyncTime;

    unsigned int t1[1];
    unsigned int t2[1];

    unsigned int dt;
    u32 TimeShift[1]= {0};
////////////////////////////////////
    u8 SID = 0,avIndex=0;



    DEBUG_P2P("IOTYPE_IPCAM_START[%d:%d]\n", SID, avIndex);
    regedit_client_to_avsession(SID, avIndex);
    regedit_client_to_video(SID);

    if(gFirstConnect==0)
    {
        //gFirstConnect=1;
        DEBUG_P2P("Set VGA to 10 FPS.\n");
        uiSetP2PImageLevel(0,2);// 480P 10FPS
        //uiSetP2PImageLevel(0,3);// 240P 10FPS
        //uiSetP2PImageLevel(0,4);
    }
    else
    {
        uiSetP2PImageLevel(0,6);
    }

    Start_P2P_Session(0, 2);



#if 1


Reconnect:
#if 1
    if ((skt2 = (int)socket(AF_INET, SOCK_STREAM, 0)) >= 0)
    {
        //printf("\x1B[96m=%d,%d,%d=\x1B[0m\n", setsockopt(skt2,SOL_SOCKET,TCP_KEEPALIVE,&on,sizeof(on)), setsockopt(skt2,SOL_SOCKET,TCP_NODELAY, &on, sizeof(on)), setsockopt(skt2,IPPROTO_TCP,TCP_NODELAY, &on, sizeof(on)));
        setsockopt(skt2,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(int));/*Check the TCP connection whether alive.*/
        setsockopt(skt2,IPPROTO_TCP,TCP_NODELAY, &on, sizeof(int));
        setsockopt(skt2,IPPROTO_TCP,TCP_KEEPALIVE, &on, sizeof(int));

        cloudVideoSrvAddr.sin_addr.s_addr=inet_addr("192.168.1.55");
        cloudVideoSrvAddr.sin_port = htons(20003);		//20161220 PORT TBD
        cloudVideoSrvAddr.sin_family = AF_INET;


        if (connect(skt2, (struct sockaddr *)&cloudVideoSrvAddr, sizeof(struct sockaddr_in)) == 0)
        {
            printf("\x1B[91mSTART!\x1B[0m\n");
        }
        else
        {
            DEBUG_P2P("Couldn't Push Cloud Video\n");
            goto Reconnect;
        }
    }
    else
    {
        DEBUG_P2P("Create Cloud Video socket fail.\n");
        goto Reconnect;
    }
#endif
    while(1)
    {
        DlyFlag=1;
		audio_value[ch] = OSSemAccept(P2PAudioCmpSemEvt[ch]);

#if 0
        // ------Streaming audio payload------//
        if((video_value[ch] == 0) || (P2PAudioPresentTime[ch] <= P2PVideoPresentTime[ch]))
        {
            audio_value[ch] = OSSemAccept(P2PAudioCmpSemEvt[ch]);
            if (audio_value[ch] > 0)
            {
                if(audio_value_max[ch] < audio_value[ch])
                    audio_value_max[ch] = audio_value[ch];
                //DEBUG_P2P("PAIdx = %d iisIdx = %d\n", P2PAudioBufReadIdx[ch], VideoClipOption[ch + 1].iisSounBufMngWriteIdx);
            }

            if (audio_value[ch] > 0)
            {
                timerCountRead(guiRFTimerID, &t2[ch]);
                if(P2PChannelStart[ch])
                {
                    t1[ch]=t2[ch];
                    P2PChannelStart[ch]=0;
                    TimeShift[ch]=0;
                }
                if(t1[ch] >= t2[ch])
                    dt=t1[ch]-t2[ch];
                else
                    dt=(t1[ch]+TimerGetTimerCounter(7))-t2[ch];

                if(dt > 20000)
                {
                    TimeShift[ch] = dt/10;
                    if(P2PChannelStart[ch])
                        TimeShift[ch]=0;
                    else
                    {
                        P2PAudioPresentTime[ch] += (TimeShift[ch]+2000);
                        P2PVideoPresentTime[ch] += (TimeShift[ch]+2000);
                        DEBUG_P2P("--->P2P Ch-%d Shift=%d ms\n",ch,TimeShift[ch]);
                    }
                }
                else
                    TimeShift[ch]=0;
                t1[ch]=t2[ch];

                P2PAudioPresentTime[ch] += (P2PAudioBuf[ch][P2PAudioBufReadIdx[ch]].time);    //if use chunk time
                P2PAudioBufReadIdx[ch]   = (P2PAudioBufReadIdx[ch] + 1) % IIS_BUF_NUM;
                //DEBUG_P2P("A%d ",audio_value[ch]);
            }
        }
#endif
        //------ Streaming video payload------//
        //if((audio_value[ch] == 0) || (P2PAudioPresentTime[ch] >= P2PVideoPresentTime[ch]))
        {
            video_value[ch] = OSSemAccept(P2PVideoCmpSemEvt[ch]);
            if (video_value[ch] > 0)
            {
                //DEBUG_P2P("P2PVideoCmpSemEvt[%d]=%d\n", ch, P2PVideoCmpSemEvt[ch]->OSEventCnt);
                if(video_value_max[ch] < video_value[ch])
                    video_value_max[ch] = video_value[ch];
                P2PVideoPresentTime[ch] += (P2PVideoBuf[ch][P2PVideoBufReadIdx[ch]].time); //if use chunk time
            }
                if(gFirstConnect==0)//當APP第一次連線,將VOL設為VGA
                {
                    gPlaybackWidth=640;
                    gPlaybackHeight=480;
                    gFirstConnect=1;
                    UpdateHeader(6,USE_MPEG_QUANTIZATION);
                }
                else
                    UpdateHeader(ch,gRfiuUnitCntl[ch].TX_Status & RFIU_TX_STA_MPEG_Q);

                memcpy(p2plocal_buffer, MPEG4_config, 0x1d);
                memcpy_hw(p2plocal_buffer+0x1d, P2PVideoBuf[ch][P2PVideoBufReadIdx[ch]].buffer, P2PVideoBuf[ch][P2PVideoBufReadIdx[ch]].size);
                //printf("\x1B[91m.%d.\x1B[0m\n",P2PVideoBuf[ch][P2PVideoBufReadIdx[ch]].size);
                video_size = (0x1d+P2PVideoBuf[ch][P2PVideoBufReadIdx[ch]].size);

#if 1
                if((video_size-0x1d > 0) && (video_temp != video_size))
                {
                    printf("\x1B[91m.%d. %d\x1B[0m\n",video_size, P2PVideoBuf[ch][P2PVideoBufReadIdx[ch]].flag);
                    video_temp = video_size;

                    if(aa2 == 0)
                    {
                        if(send(skt2, temp, strlen(temp), 0)<0)
                            //if(send(skt, p2plocal_buffer, 0x1d+P2PVideoBuf[ch][P2PVideoBufReadIdx[ch]].size, 0)<0)
                        {
                            printf("Video reconnect!\n");
                            close(skt2);
                            //goto Reconnect;
							Stop_P2P_Session(0);

                            gVideo_task = 0;
                            OSTaskDel(CLOUD_VIDEO_TASK_PRIORITY);

                        }
                        else
                        {
                            printf("\x1B[96mheader\x1B[0m\n");
                            aa2 = 1;
                        }
                    }
                    else
                    {
                        cnt = video_size/split_size;
                        for(bb=0; bb<cnt; bb++)
                        {
                            //printf("\x1B[91m%d\x1B[0m\n",bb);

                            if(send(skt2, p2plocal_buffer+split_size*bb, split_size, 0)<0)
                            {
                                aa2 = 0;
                                printf("Video reconnect2!\n");
                                close(skt2);
                                //goto Reconnect;
								Stop_P2P_Session(0);

                                gVideo_task = 0;
                                OSTaskDel(CLOUD_VIDEO_TASK_PRIORITY);
                            }
                            //OSTimeDly(1);
                        }
                        if(video_size%split_size != 0)
                        {
                            if(send(skt2, p2plocal_buffer+split_size*cnt, video_size-split_size*cnt, 0)<0)
                            {
                                aa2 = 0;
                                printf("Video reconnect3!\n");
                                close(skt2);
                                //goto Reconnect;
								Stop_P2P_Session(0);

                                gVideo_task = 0;
                                OSTaskDel(CLOUD_VIDEO_TASK_PRIORITY);
                            }
                        }
                    }

                }
#endif

            if (video_value[ch] > 0)
            {
                P2PVideoBufReadIdx[ch] = (P2PVideoBufReadIdx[ch] + 1) % VIDEO_BUF_NUM;
            }
        }

        ///------------------- Bitstream buffer control---------------------------------//
        /*
        Lsk: 以Video bitstream buffer 內的index剩餘個數為偵測點,若大於 ASF_DROP_FRAME_THRESHOLD
        則為網路速度過慢,需drop frame.

        */
        if( (P2PVideoCmpSemEvt[ch]->OSEventCnt > 60) )
        {
            DEBUG_P2P("\nP2P-%d DROP Video frame Start:(%d,%d) \n",ch,P2PVideoPresentTime[ch],P2PAudioPresentTime[ch]);
            SyncTime=0;
            //------Video-----//
            do
            {
                video_value[ch] = OSSemAccept(P2PVideoCmpSemEvt[ch]);
                //DEBUG_P2P("%d ",video_value[ch]);
                if (video_value[ch] > 0)
                {
                    if(video_value_max[ch] < video_value[ch])
                        video_value_max[ch] = video_value[ch];

                    P2PVideoPresentTime[ch] += P2PVideoBuf[ch][P2PVideoBufReadIdx[ch]].time;
                    SyncTime += P2PVideoBuf[ch][P2PVideoBufReadIdx[ch]].time;
                    P2PVideoBufReadIdx[ch] = (P2PVideoBufReadIdx[ch] + 1) % VIDEO_BUF_NUM;
                }
                else
                {
                    DEBUG_P2P("Video buffer empty!\n");
                    //break;
                    OSTimeDly(1);
                }
            }
            while(P2PVideoBuf[ch][P2PVideoBufReadIdx[ch]].flag != FLAG_I_VOP);
#if 0
            //------Audio-----//
            DEBUG_P2P("P2P-%d DROP Audio frame Start:%d,SyncTime=%d \n",ch,P2PAudioCmpSemEvt[ch]->OSEventCnt,SyncTime);

            do
            {
                audio_value[ch] = OSSemAccept(P2PAudioCmpSemEvt[ch]);
                //DEBUG_P2P("%d ",audio_value[ch]);

                if (audio_value[ch] > 0)
                {
                    if(audio_value_max[ch] < audio_value[ch])
                        audio_value_max[ch] = audio_value[ch];

                    P2PAudioPresentTime[ch] += P2PAudioBuf[ch][P2PAudioBufReadIdx[ch]].time;
                    SyncTime -= P2PAudioBuf[ch][P2PAudioBufReadIdx[ch]].time;
                    P2PAudioBufReadIdx[ch] = (P2PAudioBufReadIdx[ch] + 1) % IIS_BUF_NUM;
                }
                else
                {
                    DEBUG_P2P("Audio buffer empty!\n");
                    break;
                }

                if(SyncTime < 0)
                    break;
            }
            while( (P2PAudioPresentTime[ch] < P2PVideoPresentTime[ch]) );
#endif
            //P2PAudioPresentTime[ch] = P2PVideoPresentTime[ch];
            DEBUG_P2P("P2P-%d DROP frame End:(%d,%d)!!\n\n",ch,P2PVideoPresentTime[ch],P2PAudioPresentTime[ch]);
            sysDeadLockMonitor_Reset();
            DlyFlag=0;
            RunCount=0;
            //OSTimeDly(1);
        }

        if ( (video_value[ch] > 0) || (audio_value[ch] > 0) )
            DlyFlag=0;

        if(DlyFlag)
        {
            RunCount=0;
            //DEBUG_P2P("&");
            OSTimeDly(1);

        }
        else
        {
            if( (RunCount & 0x03) == 0) //Lucian: release CPU power
            {
                //DEBUG_P2P("A=%d ",P2PAudioCmpSemEvt[0]->OSEventCnt);
                //DEBUG_P2P("Rcnt=%d ",RunCount);
                if(RunCount>100)
                    sysDeadLockMonitor_Reset();
                OSTimeDly(1);
            }
        }

        RunCount ++;
    }


#endif
    //End:
}


#endif
