//*----------------------------------------------------------------------------
//* ゅン			   : converter.c
//* 磞瓃		   : 絪絏锣てㄣ
//*     		   : 纒
//* セ			   : 1.0
//* ミら戳丁	   : 2005/06/24 10:21
//* 程эら戳丁 : 
//* э		   : 
//*----------------------------------------------------------------------------
//*------------------------------ include file --------------------------------
#include	"ucos_ii.h"
#include	"converter.h"
//*--------------------- Constants & Variable definition ----------------------
//*----------------------------------------------------------------------------
//* ㄧ计嘿 : PowerCal
//* 磞瓃 : 经笲衡
//* 把计 : <s32Original>[in] 砆よ计
//*			 :    <s32Power>[in] 经笲衡Ω计
//* 把计 : 礚
//*----------------------------------------------------------------------------
INT32S s32PowerCal(INT32S s32Original, INT32S s32Power)
{
	INT32S i, __s32RtnVal = 0;
	
	if(s32Power > 0)
	{
		__s32RtnVal = s32Original;
	
		for(i=1; i<s32Power; i++)
		{
			__s32RtnVal *= s32Original;
		}
	}	
	else
		__s32RtnVal = 1;
	
	return __s32RtnVal;
}
//*----------------------------------------------------------------------------
//* ㄧ计嘿 : ConvertArabNumToHexArray
//* 磞瓃 : 锣传┰计16秈
//* 把计 : <pu8Dest>[out] 钡Μ皐
//*			 :  <pszSrc>[in]  砆锣传ず甧皐
//*			 :  <u32Len>[in]  砆锣传ず甧
//* 把计 : 礚
//*----------------------------------------------------------------------------
void vConvertArabNumToHexArray(INT8U *pu8Dest, const INT8S *pszSrc, INT32S s32Len)
{
	INT32S 	i;
	
	for(i=0; i<s32Len; i++)
	{
		*(pu8Dest+i) = *(pszSrc + i) - 0x30;
	}
}
//*----------------------------------------------------------------------------
//* ㄧ计嘿 : s32GetIntFromArray
//* 磞瓃 : 眖竊计舱莉眔俱计沮ゑ{0x01, 0x02, 0x03}锣传秈123
//* 把计 : <lpszSrc>[in] 方计舱
//*			 :  <s32Len>[in] 计舱
//* 把计 : 俱计沮
//*----------------------------------------------------------------------------
INT32S s32GetIntFromArray(INT8S *ps8Src, INT32S s32Len)
{
	INT32S 	i, __s32RtnVal = 0, __s32Len = s32Len;
	
	for(i=0; i<s32Len; i++)
		__s32RtnVal += ((ps8Src[i] - 0x30) * s32PowerCal(10, --__s32Len));
		
	return __s32RtnVal;
}
//*----------------------------------------------------------------------------
//* ㄧ计嘿 : ConvertStringToOneByte_Hex
//* 磞瓃 : 锣传才﹃"255"ぃ禬筁16秈竊
//* 把计 : <pszSrc>[in] 方ず甧皐
//*			 :  <s8Len>[in] 砆锣传ず甧(竊虫)
//* 把计 : 816秈计
//*----------------------------------------------------------------------------
INT8U u8ConvertStringToOneByte_Hex(INT8S *pszSrc, INT8S s8Len)
{
	INT8S		i, k;
	INT16U		__u16RtnVal = 0;
	
	if(s8Len > 4)
		return 0xFF;
	
	for(i=s8Len, k=0; i>0; i--, k++)
	{
		__u16RtnVal += ((*(pszSrc + k) - 0x30) * (s32PowerCal(10, i-1)));
		if(__u16RtnVal > 0xFF)
			return 0xFF;
	}
	
	return (INT8U)__u16RtnVal;
}
