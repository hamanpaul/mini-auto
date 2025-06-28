//*----------------------------------------------------------------------------
//* 文件名			   : stringExtAPI.c
//* 功能描述		   : 字符串擴展函數庫
//* 作者    		   : 龍一
//* 版本			   : 1.0
//* 建立日期、時間	   : 2006/01/10 12:03
//* 最近修改日期、時間 : 
//* 修改原因		   : 
//*----------------------------------------------------------------------------
#ifndef	stringExtAPI_h
#define	stringExtAPI_h

//*------------------------------ include file -------------------------------

//*----------------------------- Constants definition ------------------------

//*-------------------- Structure & Variable definition ----------------------

//*---------------------------- Function Prototyping -------------------------
extern INT8S *strcatExt(INT8S *pbDest, INT32U u32DestLen, const INT8S *pbSrc, INT32U u32SrcLen);
extern INT32S strlenExt(const INT8S *pbSrc);

#endif