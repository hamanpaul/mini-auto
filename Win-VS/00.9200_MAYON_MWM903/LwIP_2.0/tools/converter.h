//*-------------------------------------------------------------------------------------------------
//* 文件名			   : converter.h
//* 功能描述		   : 編碼轉化工具
//* 作者    		   : 龍一
//* 版本			   : 1.0
//* 建立日期、時間	   : 2005/06/24 10:21
//* 最近修改日期、時間 : 
//* 修改原因		   : 
//*-------------------------------------------------------------------------------------------------
#ifndef	__converter_h
#define	__converter_h
//*---------------------------------------- 結構體及宏定義 -----------------------------------------
#define	macHighToLowForWord(uwHex)		((uwHex & 0xFF) << 8) | ((uwHex & 0xFF00) >> 8)
//*-------------------------------------- 函數原型聲明 ---------------------------------------------
extern INT32S s32PowerCal(INT32S s32Original, INT32S s32Power);
extern void vConvertArabNumToHexArray(INT8U *pu8Dest, const INT8S *pszSrc, INT32S s32Len);
extern INT32S s32GetIntFromArray(INT8S *ps8Src, INT32S s32Len);
extern INT8U u8ConvertStringToOneByte_Hex(INT8S *pszSrc, INT8S s8Len);

#endif