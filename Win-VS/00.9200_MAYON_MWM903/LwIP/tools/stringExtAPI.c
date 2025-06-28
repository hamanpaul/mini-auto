//*----------------------------------------------------------------------------
//* ゅン			   : stringExtAPI.c
//* 磞瓃		   : 才﹃耎甶ㄧ计畐
//*     		   : 纒
//* セ			   : 1.0
//* ミら戳丁	   : 2006/01/10 12:03
//* 程эら戳丁 : 
//* э		   : 
//*----------------------------------------------------------------------------
//*------------------------------ include file --------------------------------
#include	"ucos_ii.h"
#include	"stringExtAPI.h"
//*----------------------------------------------------------------------------
//* ㄧ计嘿 : strcatExt
//* 磞瓃 : strcat夹非畐ㄧ计耎ㄧ计
//* 把计 :     <pbDest>[out] 钡Μず皐
//*			 : <u32DestLen>[in]  磷ず簗赣ㄧ计惠璶發ぇ玡耞钡
//*			 :		     		 Μ絯侥跋
//* 		 :      <pbSrc>[in]  砆發ず甧皐
//*			 :  <u32SrcLen>[in]  砆發ず甧
//* 把计 : 砆發程竚
//*----------------------------------------------------------------------------
INT8S *strcatExt(INT8S *pbDest, INT32U u32DestLen, const INT8S *pbSrc, INT32U u32SrcLen)
{
	INT8S		*__pbStartAddr;
	INT8S		*__pbEndAddr;
	INT32S		i;

	//* т砆發秨﹍狦挡竒钡Μ絯侥跋Ю场玥ぃ發
	__pbStartAddr = pbDest + strlenExt(pbDest);
	__pbEndAddr = pbDest + u32DestLen;	
	i = 0;
	while((__pbStartAddr + i) < __pbEndAddr && i < u32SrcLen)
	{
		*(__pbStartAddr + i) = *(pbSrc + i);		
		i++;
	}
	
	return pbDest;
}
//*----------------------------------------------------------------------------
//* ㄧ计嘿 : strlenExt
//* 磞瓃 : strlenExt夹非畐ㄧ计耎ㄧ计
//* 把计 : <pbSrc>[in] 砆璸衡才﹃皐
//* 把计 : 才﹃
//*----------------------------------------------------------------------------
INT32S strlenExt(const INT8S *pbSrc)
{
	INT32S 		i = 0;
	const INT8S	*__pbNext = pbSrc;
	
	while(*(__pbNext+i) != 0x00)
		i++;
		
	return i;
}
