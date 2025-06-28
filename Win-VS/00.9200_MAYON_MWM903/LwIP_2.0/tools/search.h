//*----------------------------------------------------------------------------
//* 文件名			   : search.h
//* 功能描述		   : 內存查找函數庫頭文件
//* 作者    		   : 龍一
//* 版本			   : 1.0
//* 建立日期、時間	   : 2004/07/17 10:21
//* 最近修改日期、時間 : 
//* 修改原因		   : 
//*----------------------------------------------------------------------------
#ifndef	__search_h
#define	__search_h

//*---------------------------- Function Prototyping -------------------------
extern void *MemSearch(void *pvMem, INT32U u32MemSize, INT32U u32Data, INT32U u32DataSize);
extern INT8S *MemChr(INT8S *pbMem, INT8S bBYTE, INT32U u32MemSize);
extern BOOLEAN memstr(INT8S *pbMem, INT8S *pbStr, INT32U u32StrSize, INT32S s32MemSize);
extern INT8S *memstrExt(INT8S *pbMem, INT8S *pbStr, INT32U u32StrSize, INT32U u32MemSize);

#endif