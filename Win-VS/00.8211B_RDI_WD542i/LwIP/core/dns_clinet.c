#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)

//*------------------------------------------------------------------------------------------------
//* ゅン			: dns_clinet.c
//* 磞瓃			: dns傼狠
//*     			: 礘猧
//* セ				: 0.1
//* ミら戳兒厎		: 2006/08/10 15:23
//* 程эら戳兒厎	: 
//* э			: 
//*------------------------------------------------------------------------------------------------
//*------------------------------------------ 壋ゅン -----------------------------------------------
#include	"ucos_ii.h"
#include	"os_cpu.h"
#include	"../LwIP/include/lwip/tcpip.h"
#include	"../LwIP/include/lwip/api.h"

#include	"../LwIP/tools/stringExtAPI.h"
#include	"../LwIP/tools/search.h"
#include	"../LwIP/tools/converter.h"

#include	"../LwIP/include/lwip/dns_clinet.h"

//*================================================================================================
//*ㄧ囒凂
//*================================================================================================
//*------------------------------------------------------------------------------------------------
//* ㄧ囒嗭 : __ilvPacketDNSQuest
//* 磞瓃 : 咵DNS琩哛
//*          :     <pszDN>[in] 办冡
//*          :  <s32DNLen>[in] 办僣
//*          :   <pstPbuf>[in] st_pbuf匔疼冡ウ劚儶匔疼玂璶匉癳DNS琩哛
//* 凃囒 : 朴
//*------------------------------------------------------------------------------------------------
__inline void __ilvPacketDNSQuest(INT8S *pszDN, INT32S s32DNLen, ST_PBUF *pstPbuf)
{
	ST_DNS_PACKET_HDR*	__pstDNSPacket;
	BST_ID_AND_FLAGS* __pbstIDAndFlags;
	 
	__pstDNSPacket = (ST_DNS_PACKET_HDR*)pstPbuf->payload;
	/*Do query recursively.  Modified by aher 2014/04/02*/
	//__pstDNSPacket->u32IDAndFlags = 0x00000000;
	__pstDNSPacket->u32IDAndFlags = 0x00010000;		
	//
	__pbstIDAndFlags =(BST_ID_AND_FLAGS*)__pstDNSPacket->u32IDAndFlags;
	__pbstIDAndFlags->bitID = 0x0608;
	__pbstIDAndFlags->bitIsRD = 0x1;
	__pstDNSPacket->u16QCount = 0x0100;
	__pstDNSPacket->u16ACount = 0x0000;
	__pstDNSPacket->u16AuthCount = 0x0000;
	__pstDNSPacket->u16ARC = 0x0000;
	memcpy((INT8U*)pstPbuf->payload + DNS_PACKET_HDR_LEN, pszDN, s32DNLen);
	*((INT8U*)pstPbuf->payload + DNS_PACKET_HDR_LEN + s32DNLen) = 0x00;
	*((INT8U*)pstPbuf->payload + DNS_PACKET_HDR_LEN + s32DNLen + 1) = DNS_PACKET_QUERY_TYPE;
	*((INT8U*)pstPbuf->payload + DNS_PACKET_HDR_LEN + s32DNLen + 2) = 0x00;
	*((INT8U*)pstPbuf->payload + DNS_PACKET_HDR_LEN + s32DNLen + 3) = DNS_PACKET_QUERY_CLASS;
}
//*------------------------------------------------------------------------------------------------
//* ㄧ囒嗭 : __ilu32ParseRespDNSPacket
//* 磞瓃 : 秆猂DNS狝冿竟尫壺囒誹
//* 凃囒 :  <pstPbuf>[in] ST_NETBUF冡
//*          :   <pu32IP>[in] 钡Μ圙僉凂玂IP冡
//*          : <s32DNlen>[in] 璶琩哛办僣
//* 凃囒 : - DNS_OK	       : 琩哛Θ
//*          : - DNS_NOT_EXIST : ﹚办ぃ
//*          : - DNS_OTHER     : ゼ墋嘡
//*------------------------------------------------------------------------------------------------
__inline INT32U __ilu32ParseRespDNSPacket(ST_PBUF *pstPbuf, INT32U *pu32IP, INT32S s32DNLen)
{
	ST_DNS_PACKET_HDR	*__pstDNSPacket;

	BST_ID_AND_FLAGS	*__pbstIDAndFlags;
	void 				*__pvData;
	INT16U				__u16ACount, __u16AnswerDataLen, __u16Offset;
	
	__pstDNSPacket = (ST_DNS_PACKET_HDR	*)pstPbuf->payload;

	__pbstIDAndFlags = (BST_ID_AND_FLAGS*)__pstDNSPacket->u32IDAndFlags;
	

/*Sometimes the value of bitOptResult was error, but the IP address was right,shouldn't return error.     Modified by AHER 2014/04/07*/
	if(__pbstIDAndFlags->bitOptResult == 3)
	{
		return  DNS_NOT_EXIST;
	}
	else// if(__pbstIDAndFlags->bitOptResult == 0)
	{
		__u16ACount = macHighToLowForWord(__pstDNSPacket->u16ACount);
		__u16Offset = s32DNLen + 14;
		while(__u16ACount > 0)
		{
			__pvData = (INT8U*)pstPbuf->payload + DNS_PACKET_HDR_LEN + __u16Offset;
			__u16AnswerDataLen = macHighToLowForWord(*((__packed INT16U*)__pvData));
			
			if((__u16AnswerDataLen == 4) && macHighToLowForWord(*((__packed INT16U*)((INT8U*)__pvData - 8))) == DNS_PACKET_QUERY_TYPE)
			{
				*pu32IP = *((__packed INT32U *)((INT8U*)__pvData + 2));				
				
				return DNS_OK;
			}
			
			__u16Offset += 12 + __u16AnswerDataLen;
			
			__u16ACount--;
		}
		
		return DNS_OTHER;
	}
	/*
	else if(__pbstIDAndFlags->bitOptResult == 3)
	{
		return  DNS_NOT_EXIST;
	}
	
	return DNS_OTHER;
	*/
}
//*------------------------------------------------------------------------------------------------
//* ㄧ囒嗭 : __vDNSRecv
//* 磞瓃 : 钡Μ秆猂DNS狝冿竟尫壺囒誹ウ琌儶坄ㄧ囒パudp_recv()劍竚
//* 凃囒 :    <pstArg>[in] ST_RECV_FUN_ARG匔疼冡劚儶匔疼玂昅咷ㄧ囒惠璶凃囒
//*          : <pstUDPPCB>[in] ST_UDP_PCB匔疼冡
//*          :   <pstPbuf>[in] st_pbuf匔疼冡ウ玂昅Μ乆甧
//*          :     <pstIP>[in] IP
//*          :   <u16Port>[in] 狠咥
//* 凃囒 : - DNS_OK	       : 琩哛Θ
//*          : - DNS_NOT_EXIST : ﹚办ぃ
//*          : - DNS_OTHER     : ゼ墋嘡
//*------------------------------------------------------------------------------------------------
static void __vDNSRecv(ST_RECV_FUN_ARG *pstArg, ST_UDP_PCB *pstUDPPCB, 
						ST_PBUF *pstPbuf, ST_IP_ADDR *pstIP, INT16U u16Port)
{
	if(pstPbuf != NULL)
	{
		pstArg->u32RtnCode = __ilu32ParseRespDNSPacket(pstPbuf, pstArg->pu32IP, pstArg->s32DNLen);
	
		pbuf_free(pstPbuf);
	}
}
//*------------------------------------------------------------------------------------------------
//* ㄧ囒嗭 : __u32GetIP
//* 磞瓃 : 琩哛DNS狝冿竟夨﹚办咅﹚IP劚ńㄏノ僔孷勭钡钡喛刢ㄏノㄤ矗ㄑ
//*          : API朴猭喼ì惠―狦钡ΜDNS尫壺ㄏノnetconn_recvㄧ囒獶盽甧峨╰刬僶UDP
//*          : ぃ綼
//*          :  <pstPbuf>[in] st_pbuf匔疼冡ウ劚儶匔疼玂璶匉癳DNS琩哛
//*          :   <pu32IP>[in] 钡Μ圙僉凂玂IP冡
//*          : <s32DNLen>[in] 璶琩哛办僣
//*          : <u32DNSIP>[in] DNS狝冿竟IP
//* 凃囒 : - DNS_OK	       : 琩哛Θ
//*          : - DNS_NOT_EXIST : ﹚办ぃ
//*          : - DNS_NOT_CONN  : 朴猭劜钡﹚办狝冿竟劼狝冿竟Аぃノ
//*          : - DNS_OTHER     : ゼ墋嘡
//*------------------------------------------------------------------------------------------------
INT32U __u32GetIP(ST_PBUF *pstPbuf, INT32U *pu32IP, INT32S s32DNLen, INT32U u32DNSIP)
{
	ST_IP_ADDR 			__stIPAddr;
	ST_UDP_PCB			*__pstUDPPCB;
	ST_RECV_FUN_ARG		__stRecvArg;
	INT32S				i, k = 0;
	INT32U				__u32RtnCode;
	
	__pstUDPPCB = udp_new();
	if(__pstUDPPCB == NULL)
		return DNS_OTHER;
	
	__stIPAddr.addr = htonl(u32DNSIP);
	if(udp_connect(__pstUDPPCB, &__stIPAddr, DNS_PORT) != ERR_OK)
		return DNS_OTHER;
		
	memset(&__stRecvArg, 0, sizeof(ST_RECV_FUN_ARG));
	__stRecvArg.u32RtnCode = -1;
	__stRecvArg.pu32IP = pu32IP;
	__stRecvArg.s32DNLen = s32DNLen;
	
	udp_recv(__pstUDPPCB, __vDNSRecv, &__stRecvArg);
	
	//* 匉癳㎝钡Μ
	while(k<15)
	{
		udp_send(__pstUDPPCB, pstPbuf);		

		i = 0;	
		while(i < 30)
		{
			__u32RtnCode = __stRecvArg.u32RtnCode;
			
			if(__u32RtnCode != -1)
				goto __lblEnd;
			
			OSTimeDlyHMSM(0, 0, 0, 100);
			i++;
		}
		
		k++;
	}				
	
__lblEnd:
	//* ΜЮΜノ哣方
	udp_disconnect(__pstUDPPCB);
	udp_remove(__pstUDPPCB);
	
	if(__u32RtnCode != -1)
		return __stRecvArg.u32RtnCode;
	else
		return DNS_NOT_CONN;
}
//*------------------------------------------------------------------------------------------------
//* ㄧ囒嗭 : u32DNToIP
//* 磞瓃 : 琩哛DNS狝冿竟掆﹚办婬勩僶IP
//* 凃囒 :    <pszDN>[in] 办冡
//*          : <s32DNLen>[in] 办僣
//*          :   <pu32IP>[in] 钡Μ圙僉凂玂IP冡
//* 凃囒 : - DNS_OK	       : 琩哛Θ
//*          : - DNS_NOT_EXIST : ﹚办ぃ
//*          : - DNS_NOT_CONN  : 朴猭劜钡﹚办狝冿竟劼狝冿竟Аぃノ
//*          : - DNS_OTHER     : ゼ墋嘡
//*------------------------------------------------------------------------------------------------
INT32U u32DNToIP(INT8S *pszDN, INT32S s32DNLen, INT32U *pu32IP)
{
	ST_PBUF         	*__pstPbuf = NULL;
	INT32S				__s32TotLen;
	INT32U				__u32RtnCode;

	__s32TotLen = DNS_PACKET_HDR_LEN + s32DNLen + 4;
	__pstPbuf = pbuf_alloc(PBUF_RAW, __s32TotLen, PBUF_POOL);
	if(__pstPbuf == NULL)
		return DNS_OTHER;
		
	__ilvPacketDNSQuest(pszDN, s32DNLen, __pstPbuf);

	__u32RtnCode = __u32GetIP(__pstPbuf, pu32IP, s32DNLen, DNS1_IP_ADDR);
	if(__u32RtnCode == DNS_NOT_CONN || __u32RtnCode == DNS_OTHER)
		__u32RtnCode = __u32GetIP(__pstPbuf, pu32IP, s32DNLen, DNS2_IP_ADDR);
	
	pbuf_free(__pstPbuf);
		
	return __u32RtnCode;
}
#endif //#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)