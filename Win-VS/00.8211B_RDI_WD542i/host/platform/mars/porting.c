/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <rtos.h>
#include <porting.h>
#include <ssv_lib.h>
//aher test for icomm
#include "uartapi.h"

#define APP_INCLUDE_WIFI_FW
//aher test for icomm
extern OS_MEM *icomBuffer;
extern OS_MEM *icomBuffer2;
extern OS_MEM *icomBuffer3;
#if 1
extern u8 *icommbuf;	/*for icomm 6030p used. by aher 20160627*/
extern u8 *icommbuf2;	/*for icomm 6030p used. by aher 20160627*/
extern u8 *icommbuf3;	/*for icomm 6030p used. by aher 20160627*/
#else
extern INT8U	icommbuf[8][2560];
extern INT8U	icommbuf2[1024][256];
extern INT8U 	icommbuf3[10][24576];
#endif

int gnum=0;
int pnum=0;

u8 minibuff[90];

/*=============console==================*/
void hal_putchar(u8 ch)
{
	sendchar(DEBUG_UART_ID, &ch);	
}

u8 hal_getchar(void)
{
	u8 ret_ch;
	ret_ch=receive_char(DEBUG_UART_ID);
	return ret_ch;

}

/*=============Memory==================*/

OS_APIs void *OS_MemAlloc( u32 size )
{

	u8 err;
	u32 *memAddr;
	OS_MEM_DATA icomm_mem_data;
	
	//printf("Mem alloc size: %d\n",size);

	//if (size ==88)   // aher test temp. 20160729
		//return minibuff;
//------------------------------------------


	if(size<=256)
	{
		//OSMemQuery(icomBuffer2,&icomm_mem_data);
		//printf("Size=%d, Blks=%d, F=%d, U=%d\n ",icomm_mem_data.OSBlkSize,icomm_mem_data.OSNBlks,icomm_mem_data.OSNFree,icomm_mem_data.OSNUsed);

	
		memAddr=OSMemGet(icomBuffer2,&err);
		if(err!=OS_NO_ERR)
		{
			printf("OSMemGet 256 Fail : %d\n",err);
			return NULL;
		}	
		else	
		{
			//printf("ADDR = %x\n",memAddr);
			memset_hw(memAddr,0x00,256);
			return memAddr;
		}
	}
	else if	((size >256)&&(size <= 2560))
	{
		//OSMemQuery(icomBuffer,&icomm_mem_data);
		//printf("Size=%d, Blks=%d, F=%d, U=%d\n ",icomm_mem_data.OSBlkSize,icomm_mem_data.OSNBlks,icomm_mem_data.OSNFree,icomm_mem_data.OSNUsed);
	
		memAddr=OSMemGet(icomBuffer,&err);
		if(err!=OS_NO_ERR)
		{
			printf("OSMemGet 2560 Fail : %d\n",err);
			return NULL;
		}	
		else	
		{
			//printf("ADDR = %x\n",memAddr);
			memset_hw(memAddr,0x00,2560);
			return memAddr;
		}
	}
	else if	((size >2560)&&(size <= 4096*6))
	{
		OSMemQuery(icomBuffer3,&icomm_mem_data);
		printf("\x1B[91m Size=%d, Blks=%d, F=%d, U=%d \x1B[0m\n",icomm_mem_data.OSBlkSize,icomm_mem_data.OSNBlks,icomm_mem_data.OSNFree,icomm_mem_data.OSNUsed);
		memAddr=OSMemGet(icomBuffer3,&err);
		if(err!=OS_NO_ERR)
		{
			printf("OSMemGet 4096 Fail : %d\n",err);
			return NULL;
		}	
		else	
		{
			//printf("ADDR = %x\n",memAddr);
			memset_hw(memAddr,0x00,4096*6);
			return memAddr;
		}
	}
	else if (size > 4096*6)
	{
		printf("Mem alloc over 4096! : %d\n",size);
		return NULL;
	}
	return NULL;
}


/**
 *  We do not recommend using OS_MemFree() API
 *  because we do not want to support memory
 *  management mechanism in embedded system.
 */
OS_APIs void __OS_MemFree( void *m )
{
    /**
        *  Platform depedent code. Please rewrite
        *  this piece of code for different system.
        */
//aher test for icomm

	u8 err=1;
//printf("Free: %x\n",(u32)m);

	

	

	if(((u8 *)m >=(u8 *)icommbuf)&&((u8 *)m <((u8 *)icommbuf+8*2560)))
	{
		//printf("\x1B[96mFree icommbuf: %x\x1B[0m\n",(u8 *)m);	
		err=OSMemPut(icomBuffer, m);
		if(err!=OS_NO_ERR)
			printf("OSMemPut 2560 Fail : %d\n",err);

	}
	else if(((u8 *)m >=(u8 *)icommbuf2)&&((u8 *)m <((u8 *)icommbuf2+1024*256)))
	{
		//printf("Free icommbuf2: %x\n",(u8 *)m);	
		err=OSMemPut(icomBuffer2, m);
		if(err!=OS_NO_ERR)
			printf("OSMemPut 256 Fail : %d\n",err);
	}	
	else if(((u8 *)m >=(u8 *)icommbuf3)&&((u8 *)m <((u8 *)icommbuf3+10*4096*6))) //Sean 20170426
	{
		//printf("\x1B[91mFree icommbuf3: %x\x1B[0m\n",(u8 *)m);	
		err=OSMemPut(icomBuffer3, m);
		if(err!=OS_NO_ERR)
			printf("OSMemPut 4096 Fail : %d\n",err);
	}	
	else
	{
	printf("\x1B[96m %08x, %08x \x1B[0m\n",(u8 *)m ,((u8 *)icommbuf3+(10*4096*6)));
	
	printf("Free: %08x\n",(u8*)m);
		printf("OSMemPut Fail !!\n");
	}	

}

void OS_MemCPY(void *pdest, const void *psrc, u32 size)
{
// memcpy不需要 4 bytes aligement 20160815
	memcpy_hw(pdest,psrc,size);
}

void OS_MemSET(void *pdest, u8 byte, u32 size)
{
	memset_hw(pdest,byte,size);
}

//=====================Platform LDO EN ping setting=======================

void platform_ldo_en_pin_init(void)
{

}

void platform_ldo_en(bool en)
{

}

//=====================find fw to download=======================

static void data_conver_endianness(u8 *buf,u32 unSize)
{
	u8 *tmp_addr;
	int i;
	
	tmp_addr=buf;
/*	
	printf("\033[40;31mC1 : \n\033[0m");	
	for (i=0;i<16;i++)	
		printf("%x ",buf[i]);
	printf("\n");
*/
	
	for(;unSize>0;)
	{
		*(u32 *)tmp_addr=((*(u32 *)tmp_addr>>24)&0xff) | // move byte 3 to byte 0
                    ((*(u32 *)tmp_addr<<8)&0xff0000) | // move byte 1 to byte 2
                    ((*(u32 *)tmp_addr>>8)&0xff00) | // move byte 2 to byte 1
                    ((*(u32 *)tmp_addr<<24)&0xff000000); // byte 0 to byte 3

		unSize=unSize-4;             
		tmp_addr=tmp_addr+4;
/*
		printf("C2 : \n");	
	for (i=0;i<16;i++)	
		printf("%x ",buf[i]);
	printf("\n");
*/
	}
		
}

#if 1
#include <../../hal/SSV6030/firmware/ssv6200_uart_bin.h>
//注意fw的路徑

bool platform_download_firmware(void)
{
  int i;
u8 *test_addr;
/*
memcpy(icommbuf2,ssv6200_uart_bin,0x2422c);
data_conver_endianness(icommbuf2,sizeof(ssv6200_uart_bin));

printf("BIN : \n");
for (i=0;i<16;i++)
	printf("%x ",icommbuf2+i);
printf("\n");
*/
    //LOG_PRINTF("bin size =%d\r\n",sizeof(ssv6200_uart_bin));

    test_addr=(u8 *)(&ssv6200_uart_bin[0]);
    return ssv6xxx_download_fw((test_addr),sizeof(ssv6200_uart_bin));

#else  //this option is to set fw.bin as resource to open. please check resource_wifi_fw.s
void platform_download_firmware(void)
{
    u32 fw_size;
    extern u8 *RES_WIFI_FW_START;
    extern u8 *RES_WIFI_END;

    fw_size = ((u32)&RES_WIFI_END) - ((u32)&RES_WIFI_FW_START);
    LOG_PRINTF("fw_size=%d\r\n",fw_size);
    ssv6xxx_download_fw((u8 *)&RES_WIFI_FW_START,fw_size);//??? u8* bin
#endif
}
void platform_read_firmware(void *d,void *s,u32 len)
{
    OS_MemCPY(d,s,len);
}
