#include "ucos_ii.h"
#include "general.h" 
#include "../LwIP/include/lwip/tcpip.h"
#include "../LwIP/include/lwip/api.h"
#include "../LwIP/include/lwip/net.h"
#include "../LwIP/include/lwip/rtp.h"
#include "../LwIP/include/lwip/rtsp.h"
#include "../LwIP/include/lwip/dns_clinet.h"

#include "../LwIP/tools/stringExtAPI.h"
#include "../LwIP/tools/search.h"
#include "../LwIP/tools/converter.h"
//*------------------------------------ 變量、數據類型宏定義 ---------------------------------------
RTSP_buffer RtspBuffer;
RTSP_session session_list; 
//#define ST_NETCONN struct netconn
#define HTTP_HTML_HEADER		"HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n"

void send_reply(u16 err, char *addon, RTSP_buffer * rtsp )
{
    u16 len;
    char *b;
    char p[1024];
 //   int res;
//    u8 method[32];
//    u8 object[256];
//    u8 ver[32];
     DEBUG_RTSP("=====>entering %s err %d\n", __func__,err); 

    if (addon != NULL) 
    {
        len = 256 + strlen(addon);
    } 
    else 
    {
        len = 256;
    }

    /*分配空間*/
//    p = (char *) malloc(len);
//    if (p == NULL) 
 //   {
//         DEBUG_RTSP("send_reply(): memory allocation error.\n");
//       return ERR_ALLOC;
//    }
    memset(p, 0, sizeof(p));
    b= p + 54;
    /*按照協議格式填充數據*/
    sprintf(b, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL, RTSP_VER, err, get_stat(err), rtsp->rtsp_cseq);
    strcat(b, RTSP_EL);

       netconn_write(rtsp->pstConn, (u8 *)p, (u16) strlen(b),NETCONN_COPY);
//	free(p);

}
int RTSP_state_machine(RTSP_buffer * rtsp, int method)
{

    char *s;
    RTSP_session *p;
    long int session_id;
    char trash[255];

    char  szDebug[255];
	
    DEBUG_RTSP("entering rtsp_state_machine!\n");

    /*找到會話位置*/
    if ((s = strstr(rtsp->in_buffer, HDR_SESSION)) != NULL) 
    {
        if (sscanf(s, "%254s %ld", trash, &session_id) != 2) 
        {
            DEBUG_RTSP("Invalid Session number in Session header\n");
            send_reply(454, 0, rtsp);              /* 沒有此會話*/
            return 0;
        }
    }

    /*打開會話列表*/
    p = rtsp->session_list;
    if (p == NULL) {
        DEBUG_RTSP("ray flag");
        return 0;
    }
    DEBUG_RTSP("current state is:  %d  \n",p->cur_state);
    DEBUG_RTSP("state_machine:current state is  ");
     
    /*根據狀態遷移規則，從當前狀態開始遷移*/
    switch (p->cur_state) 
    {
        case INIT_STATE:                    /*初始態*/
        {
            switch (method)
            {
                //DEBUG_RTSP("current method code is:  %d  \n",method);
                
                case RTSP_ID_DESCRIBE:  /*狀態不變*/
                    RTSP_describe(rtsp);
                    Init_RTP_Session();
                    break;
                    
                case RTSP_ID_SETUP:                /*狀態遷移為就緒態*/
                    if (RTSP_setup(rtsp, &p) == ERR_NOERROR) 
                    {
                        p->cur_state = READY_STATE;
                        DEBUG_RTSP("TRANSFER TO READY STATE!");
                    }
                    break;
                    
                case RTSP_ID_TEARDOWN:       /*狀態不變*/
                    Stop_RTP_Session();
                    RTSP_teardown(rtsp);
                    break;
                    
                case RTSP_ID_OPTIONS:
                  if (RTSP_options(rtsp) == ERR_NOERROR)
                    {
                        p->cur_state = INIT_STATE;             /*狀態不變*/                  
                    }
                    break;
               
                case RTSP_ID_PLAY:          /* method not valid this state. */
                
                case RTSP_ID_PAUSE:
                    send_reply(455, "Accept: OPTIONS, DESCRIBE, SETUP, TEARDOWN\n", rtsp);
                    break;
                    
                default:
                    send_reply(501, "Accept: OPTIONS, DESCRIBE, SETUP, TEARDOWN\n", rtsp);
                    break;
            }
        break;
        }                                                               
            
        case READY_STATE:
        {
            DEBUG_RTSP("current method code is:%d\n",method);
            
            switch (method) 
            {
                case RTSP_ID_PLAY:                                      /*狀態遷移為播放態*/
                    //UIReadFirstFile();
                    if (RTSP_play(rtsp) == ERR_NOERROR) 
                    {
                        DEBUG_RTSP("playing no error!\n");
                        Start_RTP_Session();
                        p->cur_state = PLAY_STATE;
                    }
                    break;
                    
                case RTSP_ID_SETUP:                                         
                   if (RTSP_setup(rtsp, &p) == ERR_NOERROR)       /*狀態不變*/   
                    {
                        p->cur_state = READY_STATE;
                    }
                    break;
                
                case RTSP_ID_TEARDOWN:					
                    Stop_RTP_Session();
                    RTSP_teardown(rtsp);                                         /*狀態變為初始態 ?*/
                    break;
                
                case RTSP_ID_OPTIONS:
                    if (RTSP_options(rtsp) == ERR_NOERROR)
                    {
                        p->cur_state = INIT_STATE;             /*狀態不變*/                  
                    }
                    break;
                    
                case RTSP_ID_PAUSE:                                             /* method not valid this state. */
                    send_reply(455, "Accept: OPTIONS, SETUP, PLAY, TEARDOWN\n", rtsp);
                    break;

                case RTSP_ID_DESCRIBE:
                    RTSP_describe(rtsp);
                    break;

                default:
                    send_reply(501, "Accept: OPTIONS, SETUP, PLAY, TEARDOWN\n", rtsp);
                    break;
            }
            
            break;
        }


        case PLAY_STATE:
        {
            switch (method)
            {
                case RTSP_ID_PLAY:
                    // Feature not supported
                    DEBUG_RTSP("UNSUPPORTED: Play while playing.\n");
                    send_reply(551, 0, rtsp);        /* Option not supported */
                    break;

                case RTSP_ID_PAUSE:                                  /*狀態遷移為就緒態*/
                   if (RTSP_pause(rtsp) == ERR_NOERROR)
                   {
                        DEBUG_RTSP("pause  no error!\n");                 
                        Pause_RTP_Session();
                        p->cur_state = READY_STATE;    
                    }
                    break;

                case RTSP_ID_TEARDOWN:
                    Stop_RTP_Session();
                   RTSP_teardown(rtsp);                             /*狀態遷移為初始態*/
					
                    break;
                
                case RTSP_ID_OPTIONS:
                    break;
                
                case RTSP_ID_DESCRIBE:
                    RTSP_describe(rtsp);
                    break;
                
                case RTSP_ID_SETUP:
                    break;
            }
            
            break;
        }/* PLAY state */

        default:
            {        
                /* invalid/unexpected current state. */
                DEBUG_RTSP("State error: unknown state=%d, method code=%d\n", p->cur_state, method);
            }
            break;
    }/* end of current state switch */

    DEBUG_RTSP("leaving rtsp_state_machine!\n");
}

char *get_stat(int err)
{
    struct {
        u8 *token;
        int code;
    } status[] = {
        {
        "Continue", 100}, {
        "OK", 200}, {
        "Created", 201}, {
        "Accepted", 202}, {
        "Non-Authoritative Information", 203}, {
        "No Content", 204}, {
        "Reset Content", 205}, {
        "Partial Content", 206}, {
        "Multiple Choices", 300}, {
        "Moved Permanently", 301}, {
        "Moved Temporarily", 302}, {
        "Bad Request", 400}, {
        "Unauthorized", 401}, {
        "Payment Required", 402}, {
        "Forbidden", 403}, {
        "Not Found", 404}, {
        "Method Not Allowed", 405}, {
        "Not Acceptable", 406}, {
        "Proxy Authentication Required", 407}, {
        "Request Time-out", 408}, {
        "Conflict", 409}, {
        "Gone", 410}, {
        "Length Required", 411}, {
        "Precondition Failed", 412}, {
        "Request Entity Too Large", 413}, {
        "Request-URI Too Large", 414}, {
        "Unsupported Media Type", 415}, {
        "Bad Extension", 420}, {
        "Invalid Parameter", 450}, {
        "Parameter Not Understood", 451}, {
        "Conference Not Found", 452}, {
        "Not Enough Bandwidth", 453}, {
        "Session Not Found", 454}, {
        "Method Not Valid In This State", 455}, {
        "Header Field Not Valid for Resource", 456}, {
        "Invalid Range", 457}, {
        "Parameter Is Read-Only", 458}, {
        "Unsupported transport", 461}, {
        "Internal Server Error", 500}, {
        "Not Implemented", 501}, {
        "Bad Gateway", 502}, {
        "Service Unavailable", 503}, {
        "Gateway Time-out", 504}, {
        "RTSP Version Not Supported", 505}, {
        "Option not supported", 551}, {
        "Extended Error:", 911}, {
        NULL, -1}
    };

    int i;
    for (i = 0; status[i].code != err && status[i].code != -1; ++i);
    
    return status[i].token;
}

int RTSP_validate_method(RTSP_buffer * rtsp)
{
    char method[32], hdr[16];
    char object[256];
    char ver[32];
    unsigned int seq;
   // int pcnt;   /* parameter count */
    int mid = ERR_GENERIC;
   

    *method = *object = '\0';
    seq = 0;
    DEBUG_RTSP("=====>entering %s\n", __func__); 
    DEBUG_RTSP("rtsp->in_buffer=> %s\n", rtsp->in_buffer); 

    /*按照請求消息的格式解析消息的第一行*/

    DEBUG_RTSP("RTSP_buffer size = %d ", sizeof(RTSP_buffer));
    if ( ( sscanf(rtsp->in_buffer, " %31s %255s %31s\n%15s %u ", method, object, ver, hdr, &seq)) != 5)
        {
        DEBUG_RTSP(" method => %s\n", method);
        DEBUG_RTSP(" object => %s\n", object);
        DEBUG_RTSP(" ver    => %s\n", ver);
        DEBUG_RTSP(" hdr in => %s\n", hdr);
        DEBUG_RTSP(" seq    => %u\n", seq);
            return ERR_GENERIC;
        }
    DEBUG_RTSP(" method=> %s\n", method); 
    /*如果沒有頭標記，則錯誤*/
    if ( !strstr(hdr, HDR_CSEQ) )
        return ERR_GENERIC;

    /*根據不同的方法，返迴響應的方法ID*/
    if (strcmp(method, RTSP_METHOD_DESCRIBE) == 0) {
        rtsp->session_list->session_id = 0;
        rtsp->session_list->cur_state =INIT_STATE;
        mid = RTSP_ID_DESCRIBE;
    }
    if (strcmp(method, RTSP_METHOD_ANNOUNCE) == 0) {
        mid = RTSP_ID_ANNOUNCE;
    }
    if (strcmp(method, RTSP_METHOD_GET_PARAMETERS) == 0) {
        mid = RTSP_ID_GET_PARAMETERS;
    }
    if (strcmp(method, RTSP_METHOD_OPTIONS) == 0) {
        mid = RTSP_ID_OPTIONS;
    }
    if (strcmp(method, RTSP_METHOD_PAUSE) == 0) {
        mid = RTSP_ID_PAUSE;
    }
    if (strcmp(method, RTSP_METHOD_PLAY) == 0) {
        mid = RTSP_ID_PLAY;
    }
    if (strcmp(method, RTSP_METHOD_RECORD) == 0) {
        mid = RTSP_ID_RECORD;
    }
    if (strcmp(method, RTSP_METHOD_REDIRECT) == 0) {
        mid = RTSP_ID_REDIRECT;
    }
    if (strcmp(method, RTSP_METHOD_SETUP) == 0) {
        mid = RTSP_ID_SETUP;
    }
    if (strcmp(method, RTSP_METHOD_SET_PARAMETER) == 0) {
        mid = RTSP_ID_SET_PARAMETER;
    }
    if (strcmp(method, RTSP_METHOD_TEARDOWN) == 0) {
        mid = RTSP_ID_TEARDOWN;
    }

    /*設置當前方法的請求序列號*/
    rtsp->rtsp_cseq = seq;
    return mid;
}

//*------------------------------------------------------------------------------------------------
//* 函數名稱 : Handler_HTTP
//* 功能描述 : HTTP處理
//* 入口參數 : <pstConn>[in] 指向ST_NETCONN結構的指針
//* 出口參數 : 無
//*------------------------------------------------------------------------------------------------
int vHandler_RTSP(ST_NETCONN *pstConn)
{
	ST_NETBUF 		*__pstNetbuf;
	INT32S			__s32Len;
       INT16S			__s16Len;

       char  *rtsp_data;
	s16  request;
	RTSP_buffer *rtsp;
	int errno;

    //DEBUG_RTSP("=====>entering %s\n", __func__); 

    rtsp = &RtspBuffer;
    rtsp->session_list = & session_list;

    __pstNetbuf = netconn_recv(pstConn);

    if(__pstNetbuf != NULL)
    {
        netbuf_data(__pstNetbuf, &rtsp_data, &__s16Len);
        __s32Len=__s16Len;
        request = 0;
        rtsp->pstConn =pstConn;
        rtsp->in_buffer = rtsp_data;
        request = RTSP_validate_method(rtsp);       
        DEBUG_RTSP("request Method = %d\n",request);
        if (request < 0) 
        {
            DEBUG_RTSP("Bad Request\n ");
            send_reply(400, NULL, rtsp);
	     //aher 
	     return 1;
        } 
       else
        {
            RTSP_state_machine(rtsp, request);           /*進入到狀態機*/	 
        }  
		netbuf_delete(__pstNetbuf);	        
		    //如果收到TEARDOWN request,則close connection by aher.
	    if (request == 10)
	    {
	        //printf("aher : receive TEARDOWN\n");
		 errno=netconn_close(pstConn);  
	        return 1;
	    	}
	    else	
	        return 0;
	}

	else
    {
        printf("%");
	//aher
	 return 1;
    }	
//	netconn_close(pstConn);   
}
