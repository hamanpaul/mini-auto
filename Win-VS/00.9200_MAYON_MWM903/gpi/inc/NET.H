//-----------------------------------------------------------------------------
// NET.H
//
//-----------------------------------------------------------------------------
#ifndef _NET_H_

#define _NET_H_

#define NULL 						0
#define Max_Size 					10
#define TRUE      					1
#define ON								1
#define OK                    	1
#define FALSE     					0
#define OFF								0
#define MATCH							0
#define RET		   					0x0D
#define LF								0x0A
#define SPACE		 					0x20

// Port numbers
#define ECHO_PORT						3040
#define UDP_DATA_PORT					3080
#define DHCP_PORT 						68
#define DAYTIME_PORT					13
#define CHARGEN_PORT					19
#define TIME_PORT						37
#define HTTP_PORT  					80
#define RTSP_PORT  					554

// Event word bits
#define EVENT_ETH_ARRIVED			0x0001
#define EVENT_AGE_ARP_CACHE		    0x0002
#define EVENT_TCP_RETRANSMIT		0x0004
#define EVENT_TCP_INACTIVITY		0x0008
#define EVENT_ARP_RETRANSMIT		0x0010
#define EVENT_READ_ANALOG			0x0020
#define EVENT_RS232_ARRIVED		    0x0040


// Type number field in Ethernet frame
#define IP_PACKET             	0x0800
#define ARP_PACKET            	0x0806
#define RARP_PACKET				0x8035
// Protocol identifier field in IP datagram
#define ICMP_TYPE             	1
#define IGMP_TYPE						2
#define TCP_TYPE              	6
#define UDP_TYPE              	17

// Message type field in ARP messages 
//#define ARP_REQUEST           	1
#define ARP_RESPONSE          	2
#define RARP_REQUEST		        	3
#define RARP_RESPONSE         	4

// Hardware type field in ARP message
#define DIX_ETHERNET          	1
#define IEEE_ETHERNET         	6



typedef struct
{
   u8 ipaddr[4];
   u8 hwaddr[6];
   u8 timer; 
} ARP_CACHE;


typedef struct
{
	u8  * buf;
	u8 ipaddr[4];
	u8 proto_id;
	u16  len;
	u8 timer;
} WAIT;


typedef struct
{
   u16  hardware_type; 
   u16  protocol_type;           
   u8 hwaddr_len;
   u8 ipaddr_len;               
   u16  message_type;
   u8 source_hwaddr[6];              
   u8 source_ipaddr[4];
   u8 dest_hwaddr[6];    
   u8 dest_ipaddr[4];
} ARP_HEADER;


typedef struct
{
  u8 dest_hwaddr[6];
  u8 source_hwaddr[6];
  u16  frame_type;
} ETH_HEADER;


typedef struct
{
   u8 ver_len;
   u8 type_of_service;
   u16  total_length;
   u16  identifier;
   u16  fragment_info;
   u8 time_to_live;
   u8 protocol_id;
   u16  header_cksum;
   u8 source_ipaddr[4];
   u8 dest_ipaddr[4];
} IP_HEADER;


typedef struct
{
   u8 msg_type;
   u8 msg_code;
   u16  checksum;
   u16  identifier;
   u16  sequence;
	u8 echo_data;
} PING_HEADER;


typedef struct
{
   u8 msg_type;
   u8 msg_code;
   u16  checksum;
   u32 msg_data;
   u8 echo_data;
} ICMP_ERR_HEADER;


typedef struct 
{
   u16  source_port;
   u16  dest_port;
   u16  length;
   u16  checksum;
	u8 msg_data;
} UDP_HEADER;


typedef struct
{
   u16  source_port;
   u16  dest_port;
   u32 sequence;
   u32 ack_number;
   u16  flags;
   u16  window;
   u16  checksum;
   u16  urgent_ptr;
   u8 options;
} TCP_HEADER;


typedef struct
{
  u8 ipaddr[4];
  u8 hwaddr[6];
  u16  port;
  u32 his_sequence;
  u32 my_sequence;
  u32 old_sequence;
  u32 his_ack;
  u8 timer;
  u8 inactivity;	 
  u8 state;
  u16 resendlen;
  u8 RetryCount;
} CONNECTION;


 
typedef struct
{
	u8 NUMBER;
	u8 ADDR[3];
}MACH_LIST;

struct PooL
{
	u8 buffer[1521];
	u8 used;
	struct PooL *Next;
};

//#define DHCP_DEBUG
//#define NET_DEBUG
//#define HTTP_DEBUG
#define RTSP_DEBUG
#define RTCP_DEBUG

#ifdef NET_DEBUG 
    #define DEBUG_NET(fmt...)   printf(fmt)
#else
    #define DEBUG_NET(fmt...)   //do { } while (0)
#endif

#ifdef DHCP_DEBUG 
    #define DEBUG_DHCP(fmt...)   printf(fmt)
#else
    #define DEBUG_DHCP(fmt...)   //do { } while (0)
#endif

#ifdef HTTP_DEBUG 
    #define DEBUG_HTTP(fmt...)   printf(fmt)
#else
    #define DEBUG_HTTP(fmt...)   //do { } while (0)
#endif

#ifdef RTSP_DEBUG 
    #define DEBUG_RTSP(fmt...)   printf(fmt)
#else
    #define DEBUG_RTSP(fmt...)   //do { } while (0)
#endif

#ifdef RTCP_DEBUG 
    #define DEBUG_RTCP(fmt...)   printf(fmt)
#else
    #define DEBUG_RTCP(fmt...)   //do { } while (0)
#endif
typedef struct PooL Buferr_PooL;
//extern void eth_send(u8  * outbuf, u8 * hwaddr, u16 ptype, u16 len);
extern u8 strcmp_ip(u8 *s1,u8 *s2);

#endif