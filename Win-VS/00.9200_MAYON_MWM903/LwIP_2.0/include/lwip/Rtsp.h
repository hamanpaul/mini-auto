#ifndef __RTSP_H__
#define __RTSP_H__


/*error codes define,yanf*/
#define ERR_NOERROR          0
#define ERR_GENERIC             -1
#define ERR_NOT_FOUND       -2
#define ERR_PARSE           -3
#define ERR_ALLOC               -4
#define ERR_INPUT_PARAM         -5
#define ERR_NOT_SD          -6
#define ERR_UNSUPPORTED_PT      -7
#define ERR_EOF             -8
#define ERR_FATAL                   -9
#define ERR_CONNECTION_CLOSE        -10

/* 消息頭關鍵字 */
#define HDR_CONTENTLENGTH "Content-Length"
#define HDR_ACCEPT "Accept"
#define HDR_ALLOW "Allow"
#define HDR_BLOCKSIZE "Blocksize"
#define HDR_CONTENTTYPE "Content-Type"
#define HDR_DATE "Date"
#define HDR_REQUIRE "Require"
#define HDR_TRANSPORTREQUIRE "Transport-Require"
#define HDR_SEQUENCENO "SequenceNo"
#define HDR_CSEQ "CSeq"
#define HDR_STREAM "Stream"
#define HDR_SESSION "Session"
#define HDR_TRANSPORT "Transport"
#define HDR_RANGE "Range"	
#define HDR_USER_AGENT "User-Agent"


/*rtsp方法*/
#define RTSP_METHOD_MAXLEN 15
#define RTSP_METHOD_DESCRIBE "DESCRIBE"
#define RTSP_METHOD_ANNOUNCE "ANNOUNCE"
#define RTSP_METHOD_GET_PARAMETERS "GET_PARAMETERS"
#define RTSP_METHOD_OPTIONS "OPTIONS"
#define RTSP_METHOD_PAUSE "PAUSE"
#define RTSP_METHOD_PLAY "PLAY"
#define RTSP_METHOD_RECORD "RECORD"
#define RTSP_METHOD_REDIRECT "REDIRECT"
#define RTSP_METHOD_SETUP "SETUP"
#define RTSP_METHOD_SET_PARAMETER "SET_PARAMETER"
#define RTSP_METHOD_TEARDOWN "TEARDOWN"


/*rtsp方法記號ID*/
#define RTSP_ID_DESCRIBE 0
#define RTSP_ID_ANNOUNCE 1
#define RTSP_ID_GET_PARAMETERS 2
#define RTSP_ID_OPTIONS 3
#define RTSP_ID_PAUSE 4
#define RTSP_ID_PLAY 5
#define RTSP_ID_RECORD 6
#define RTSP_ID_REDIRECT 7
#define RTSP_ID_SETUP 8
#define RTSP_ID_SET_PARAMETER 9
#define RTSP_ID_TEARDOWN 10

/* SD keywords ,these keywords may occur in .sd files,and the server will interpret it,yanf*/
#define SD_STREAM "STREAM"
#define SD_STREAM_END "STREAM_END"
#define SD_FILENAME "FILE_NAME"
#define SD_CLOCK_RATE "CLOCK_RATE"
#define SD_PAYLOAD_TYPE "PAYLOAD_TYPE"
#define SD_AUDIO_CHANNELS "AUDIO_CHANNELS"
#define SD_ENCODING_NAME "ENCODING_NAME"
#define SD_AGGREGATE "AGGREGATE"
#define SD_BIT_PER_SAMPLE "BIT_PER_SAMPLE"
#define SD_SAMPLE_RATE "SAMPLE_RATE"
#define SD_CODING_TYPE "CODING_TYPE"
#define SD_FRAME_LEN "FRAME_LEN"
#define SD_PKT_LEN "PKT_LEN"
#define SD_PRIORITY "PRIORITY"
#define SD_BITRATE "BITRATE"
#define SD_FRAME_RATE "FRAME_RATE"
#define SD_FORCE_FRAME_RATE "FORCE_FRAME_RATE"
#define SD_BYTE_PER_PCKT "BYTE_PER_PCKT"
#define SD_MEDIA_SOURCE "MEDIA_SOURCE"
#define SD_TWIN "TWIN"
#define SD_MULTICAST "MULTICAST"

/*start CC*/
#define SD_LICENSE "LICENSE"
#define SD_RDF "VERIFY"  
#define SD_TITLE "TITLE"
#define SD_CREATOR "CREATOR"
/*end CC*/

#define RTSP_BUFFERSIZE 4096

/* Stati della macchina a stati del server rtsp*/
#define INIT_STATE      0
#define READY_STATE     1
#define PLAY_STATE      2

#define RTSP_VER "RTSP/1.0"

#define RTSP_EL "\r\n"

#define R_VERSION                     "0.1"

#define RTSP_RTP_AVP "RTP/AVP"

#define RTP_DEFAULT_PORT 52052
#ifndef MAX_DESCR_LENGTH
#define MAX_DESCR_LENGTH 1024
#endif
#define DEFAULT_MULTICAST_ADDRESS "0.0.0.0"
#define DEFAULT_TTL 32

#define DEFAULT_hostname_Addr "192.168.1.97"



/* Name of package */
#define PACKAGE "fenice"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "fenice"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "fenice 1.11"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "fenice"




 typedef struct _RTSP_session {
    int cur_state;   /*狀態*/               
    int session_id; /*會話的ID*/

    RTP_session *rtp_session; /*RTP會話*/

    struct _RTSP_session *next; /*下一個會話的指針，構成鏈表結構*/
} RTSP_session;

typedef struct _RTSP_buffer {
     struct netconn *pstConn;    /*socket文件描述符*/
    unsigned int port;/*端口號*/
  
    char *in_buffer;
    u16 in_size;/*接收緩衝區的大小*/
    int out_size;/*發送緩衝區大小*/

    u16 rtsp_cseq;/*序列號*/
    char descr[MAX_DESCR_LENGTH];/*描述*/
    RTSP_session *session_list;/*會話鏈表*/
    struct _RTSP_buffer *next; /*指向下一個結構體，構成了鏈表結構*/
} RTSP_buffer;

typedef struct _play_args
{
    struct tm playback_time;                    /*回放時間*/
    short playback_time_valid;                 /*回放時間是否合法*/
    float start_time;                                   /*開始時間*/
    short start_time_valid;                        /*開始時間是否合法*/
    float end_time;                                     /*結束時間*/
} play_args;

    void rtsp_rcve(u8 * , u16 );
	void send_reply(u16 , char *, RTSP_buffer * );
//	void rtsp_send(u8  *, u16 , u8 );
	char *get_stat(int );
	u32 is_valid_multicast_address(char *);

#endif
