#ifndef  OS_ADPT_H
#define  OS_ADPT_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <winsock2.h>


typedef		char				INT8;
typedef		unsigned char		UINT8;
typedef		short int			INT16;
typedef		short unsigned int	UINT16;
typedef		int					INT32;
typedef		unsigned int		UINT32;

void *OS_ADPT_Allocate_Memory(void *memory_pool, unsigned long size);
void OS_ADPT_Deallocate_Memory(void *memory);
#endif /* OS_ADPT_H */