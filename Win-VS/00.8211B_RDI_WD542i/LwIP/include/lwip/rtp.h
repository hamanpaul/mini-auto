#ifndef __RTP_H__

#define __RTP_H__
#define MAX_RTP_PAYLOAD_SIZE 1400


#define PT_FOR_MPEG4          96
#define PT_FOR_PCM_16K        97

#define Local_record   0 
#define Local_playback 1
#define RX_receive     2
#define RX_transcoder  3

typedef struct _port_pair
{
    int RTP;
    int RTCP;
} port_pair;

typedef struct _RTP_transport
{
    port_pair client_ports;
    port_pair server_ports;    
} RTP_transport;

typedef struct _RTCP_stats 
{
    unsigned int RR_received;
    unsigned int SR_received;
    unsigned long dest_SSRC;
    unsigned int pkt_count;
    unsigned int octet_count;
    int pkt_lost;
    unsigned char fract_lost;
    unsigned int highest_seq_no;
    unsigned int jitter;
    unsigned int last_SR;
    unsigned int delay_since_last_SR;
} RTCP_stats;

typedef struct _RTP_session
{
    RTP_transport transport;    
    unsigned short seq;
    unsigned int rtptime; 
    unsigned int  ssrc; 
	unsigned char pause;
    unsigned char started;
    u8 dest_hwaddr[6];
	RTCP_stats rtcp_stats; 
} RTP_session;

typedef struct _RTP_header 
{
    /* byte 0 */
    unsigned char csrc_len:4;   /* expect 0 */
    unsigned char extension:1;  /* expect 1, see RTP_OP below */
    unsigned char padding:1;    /* expect 0 */
    unsigned char version:2;    /* expect 2 */
    /* byte 1 */
    unsigned char payload:7;/* RTP_PAYLOAD_RTSP */
    unsigned char marker:1;/* expect 1 */
    /* bytes 2, 3 */
    unsigned short seq_no;
    /* bytes 4-7 */
    unsigned int timestamp;
    /* bytes 8-11 */
    unsigned int ssrc;/* stream number is used here. */
} RTP_header;

extern RTP_session RTP_session_for_audio, RTP_session_for_video;

void Start_RTP_Session(void);
void Stop_RTP_Session(void);
void Pause_RTP_Session(void);
#endif
