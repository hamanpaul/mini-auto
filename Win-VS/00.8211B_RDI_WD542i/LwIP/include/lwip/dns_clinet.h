#include "sysopt.h"
#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
//*------------------------------------------------------------------------------------------------
//* ¤å¥ó¦W				: dns_clinet.h
//* ¥\¯à´y­z			: dns«È‚þºÝ‰³¤å¥ó
//* §@ªÌ    			: µJ®üªi
//* ª©¥»				: 0.1
//* «Ø¥ß¤é´Á¡Bƒº…}		: 2006/08/10 15:27
//* ³Ìªñ­×§ï¤é´Á¡Bƒº…}	: 
//* ­×§ï­ì¦]			: 
//*------------------------------------------------------------------------------------------------

#ifndef	__dns_client_h__
#define	__dns_client_h__

//*---------------------------------------- …CÌÛÊ^¤Î§»©w…ó -----------------------------------------
#define	DNS_PORT					53			//* «ü©w­n„²±µªººÝ¤f†A
#define	DNS_PACKET_QUERY_TYPE		0x01		//* «ü©wªº¬d†R‹Ý«¬ƒoIP¦a§}¬d†R
#define	DNS_PACKET_QUERY_CLASS		0x01		//* «ü©wªº¬d†R‹Ýƒo¤¬ŠTÊI¦a§}

#define	DNS1_IP_ADDR	0x08080808				//* ¥DDNSªAƒï¾¹¦a§}¡]0xC0A80A47¡^
#define	DNS2_IP_ADDR	0xA85FC001				//* „Â¥ÎDNSªAƒï¾¹¦a§}
#define	DNS_OK			0						//* ¬d†R¦¨¥\
#define DNS_NOT_EXIST	1						//* «ü©wªº°ì¦W¤£¦s¦b
#define DNS_NOT_CONN	2						//* ÆÓªk„²±µ«ü©wªº°ì¦WªAƒï¾¹¡]¥D¡B„ÂªAƒï¾¹§¡¤£¥i¥Î¡^
#define DNS_OTHER		3						//* ¥¼ª¾‰}‡R

#define ST_NETCONN struct netconn
#define ST_NETBUF struct netbuf
#define ST_PBUF struct pbuf
#define ST_UDP_PCB   struct udp_pcb 
  /* source IP address */
#define ST_IP_ADDR   struct ip_addr 


//#define ST_ID_AND_FLAGS struct BST_ID_AND_FLAGS


typedef struct {
	INT32U bitID		:16;
	INT32U bitIsRD		:1;        				//* ¬O§_´Á±æ‡cŠÐ¬d†R¡A¬d†Rƒºƒo1¡A§_ƒh¡A¦bDNSªAƒï¾¹ÆÓªk§ä¨ì¬Û
	                          		 			//* ‹×°ì¦WƒÓ‰£ƒº«È‚þºÝ¥u¯à±o¨ì¤@ƒª¦s¦b†GƒÓ‰£ªº°ì¦WªAƒï¾¹¦a§}
	
	INT32U bitIsTC		:1;						//* ¬O§_¤¹„¥ºIŠÊ		
	INT32U bitOpcode	:4;						//* 0¡A‡á‹¿‡á­ã¬d†R¡F1¬O¤Ï¦V¬d†R¡F2ƒoªAƒï¾¹ƒ]†Èˆ[¨D		
	INT32U bitIsAA		:1;						//* ¬O§_ƒo±ÂŒÐ¦^µª                               		 		
	INT32U bitCmd		:1;        				//* 0¬O¬d†R¡A1¬OŒ·‰Ø
	                               		
	INT32U bitOptResult	:4;        				//* «ü©w¬d†Rªº°ì¦WªAƒï¾¹ªð¦^ªº¾Þ§@…CªG
	INT32U bitRsrved    :3;        				//* «O¯d¦r¬q¡A¥²…§ƒo0
	INT32U bitIsRA      :1;        				//* «ü©w°ì¦WªAƒï¾¹¤ä«ù‡cŠÐ¬d†R
}BST_ID_AND_FLAGS;

//PACK_STRUCT_BEGIN
typedef struct {
	INT32U u32IDAndFlags;						//* ‡á‹¿©M‡á§Ó
	INT16U u16QCount;							//* ƒô‹_‡Û 
	INT16U u16ACount;							//* ¦^µª‡Û
	INT16U u16AuthCount;						//* ±ÂŒÐ†V·½‡Û¶q
	INT16U u16ARC;								//* ‹]¥~†V·½ƒÓ‰£‡Û
}ST_DNS_PACKET_HDR;

//PACK_STRUCT_END

#define DNS_PACKET_HDR_LEN	12					//* DNS¥]‰³³¡ƒc«×

//* UDP±µ¦¬¨ç‡Û¤J¤fƒò‡Û
typedef struct {
	INT32U u32RtnCode;
	INT32U *pu32IP;
	INT32S s32DNLen;
}ST_RECV_FUN_ARG;

//*-------------------------------------- ¨ç‡Û­ì«¬ŠR©ú ---------------------------------------------
extern INT32U u32DNToIP(INT8S *pszDN, INT32S s32DNLen, INT32U *pu32IP);
#endif
#endif //#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)