/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <rtos.h>
#include <Porting.h>
#include "lwip/opt.h"

typedef struct{
    int alloc1;
    int alloc2;
    int alloc3;
    int free1;
    int free2;
    int free3;
}BufferStat;

static BufferStat buf_stat = {0, 0, 0, 0, 0, 0};

#ifdef LWIP_BUF_STAT

#define INC_BUF_ALLOC(x) (++buf_stat.alloc##x)
#define INC_BUF_FREE(x)  (++buf_stat.free##x)

#else
    
#define INC_BUF_ALLOC(x)
#define INC_BUF_FREE(x)


#endif

static OS_MEM *__lwipBuffer = 0;
static OS_MEM *__lwipBuffer2 = 0;
static OS_MEM *__lwipBuffer3 = 0;    
static u8 *__lwipbuf = 0;	
static u8 *__lwipbuf2 = 0;	
static u8 *__lwipbuf3 = 0;
static u8 *lwip_ram_heap = 0;

int gnum=0;
int pnum=0;



/*=============Memory==================*/

OS_APIs void *OS_MemAlloc( u32 size )
{

	u8 err;
	u32 *memAddr;
//	OS_MEM_DATA icomm_mem_data;
	//------------------------------------------


	if(size <= LWIP2_BLK_SIZE)
	{
		memAddr = OSMemGet(__lwipBuffer2, &err);
		if(err!=OS_NO_ERR)
		{
			printf("OSMemGet %d Fail : %d\n", LWIP2_BLK_SIZE, err);
			return NULL;
		}	
		else	
		{
			//printf("ADDR2 = %x\n",memAddr);
            INC_BUF_ALLOC(2);
			memset_hw(memAddr,0x00,LWIP2_BLK_SIZE);
			return memAddr;
		}
	}
	else if	((size > LWIP2_BLK_SIZE)&& (size <= LWIP_BLK_SIZE))
	{
		memAddr = OSMemGet(__lwipBuffer,&err);
		if(err != OS_NO_ERR)            
		{
			printf("OSMemGet %d Fail : %d\n", LWIP_BLK_SIZE, err);
			return NULL;
		}	
		else	
		{
			//printf("ADDR = %x\n",memAddr);
            INC_BUF_ALLOC(1);
			memset_hw(memAddr,0x00,LWIP_BLK_SIZE);
			return memAddr;
		}
	}
	else if	((size > LWIP_BLK_SIZE)&&(size <= LWIP3_BLK_SIZE))
	{
		//OSMemQuery(icomBuffer3,&icomm_mem_data);
		//printf("\x1B[91m Size=%d, Blks=%d, F=%d, U=%d \x1B[0m\n",icomm_mem_data.OSBlkSize,icomm_mem_data.OSNBlks,icomm_mem_data.OSNFree,icomm_mem_data.OSNUsed);
		memAddr = OSMemGet(__lwipBuffer3,&err);
		if(err!=OS_NO_ERR)
		{
			printf("OSMemGet 4096 Fail : %d\n",err);
			return NULL;
		}	
		else	
		{
			//printf("ADDR3 = %x\n",memAddr);
            INC_BUF_ALLOC(3);
			memset_hw(memAddr,0x00,LWIP3_BLK_SIZE);
			return memAddr;
		}
	}
	else if (size > LWIP3_BLK_SIZE)
	{
		printf("Mem alloc over %d! : %d\n", LWIP3_BLK_SIZE, size);
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
	u8 err=1;

	if(((u8 *)m >=(u8 *)__lwipbuf)&&((u8 *)m <((u8 *)__lwipbuf + LWIP_ALL_BLK_SIZE)))
	{        
		err = OSMemPut(__lwipBuffer, m);
		if(err!=OS_NO_ERR)
			printf("OSMemPut 2560 Fail : %d\n",err);        
        INC_BUF_FREE(1);
	}
	else if(((u8 *)m >=(u8 *)__lwipbuf2)&&((u8 *)m <((u8 *)__lwipbuf2 + LWIP2_ALL_BLK_SIZE)))
	{
		err=OSMemPut(__lwipBuffer2, m);
		if(err!=OS_NO_ERR)
			printf("OSMemPut 256 Fail : %d\n",err);
        INC_BUF_FREE(2);
	}	
	else if(((u8 *)m >=(u8 *)__lwipbuf3)&&((u8 *)m <((u8 *)__lwipbuf3 + LWIP3_ALL_BLK_SIZE))) //Sean 20170426
	{
		err=OSMemPut(__lwipBuffer3, m);
		if(err!=OS_NO_ERR)
			printf("OSMemPut 4096 Fail : %d\n",err);
        INC_BUF_FREE(3);
	}	
	else
	{
        printf("Free: %08x\n",(u8*)m);
		printf("OSMemPut Fail !!\n");
	}	

}

void OS_MemCPY(void *pdest, const void *psrc, u32 size)
{
// memcpy¤£»Ý­n 4 bytes aligement 20160815
	memcpy_hw(pdest,psrc,size);
}

void OS_MemSET(void *pdest, u8 byte, u32 size)
{
	memset_hw(pdest,byte,size);
}


void DispLwipBufStat(void)
{
#ifdef LWIP_BUF_STAT
    printf("\nlwipbuf123={%d,%d,%d,%d,%d,%d}\n", buf_stat.alloc1, buf_stat.alloc2, buf_stat.alloc3, buf_stat.free1, buf_stat.free2, buf_stat.free3);
#endif    
}


void SetLwipBuf(u8 *buf, u8 *buf2, u8 *buf3)
{
    __lwipbuf = buf;
    __lwipbuf2 = buf2;
    __lwipbuf3 = buf3;
}


void lwipBufInit(void)
{
    u8 err;

   printf("OSMemCreate : lwip used.\n");
   
   printf("\x1B[96m lwipbuf:%08x \x1B[0m\n", __lwipbuf);
   __lwipBuffer = OSMemCreate(__lwipbuf, LWIP_NBLKS, LWIP_BLK_SIZE, &err);
   if(__lwipBuffer == 0)
       printf("OSMemCreate fail.\n");
   else
       printf("OSMemCreate success.\n");
   printf("\x1B[96m lwipbuf2:%08x \x1B[0m\n", __lwipbuf2);
   
   __lwipBuffer2 = OSMemCreate(__lwipbuf2, LWIP2_NBLKS, LWIP2_BLK_SIZE, &err);
   if(__lwipBuffer2 == 0)
       printf("OSMemCreate2 fail.\n");
   else
       printf("OSMemCreate2 success.\n");
   
   printf("\x1B[96m lwipbuf3:%08x \x1B[0m\n", __lwipbuf3);
   __lwipBuffer3 = OSMemCreate(__lwipbuf3, LWIP3_NBLKS, LWIP3_BLK_SIZE, &err);
   if(__lwipBuffer3==0)
       printf("OSMemCreate3 fail.\n");
   else
       printf("OSMemCreate3 success.\n");
}


void SetLwipRamHeap(u8 *heap)
{
    lwip_ram_heap = heap;
}


u8* GetLwipRamHeap(void)
{
    return lwip_ram_heap;
}