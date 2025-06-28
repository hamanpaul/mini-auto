/*************************************************************************/
/*                                                                       */
/*         Copyright (c) 1993-1999 Accelerated Technology, Inc.          */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/


#include <os_adpt.h>


long alloc = 0;
long dealloc = 0;
long alloctimes = 0;
long dealloctimes = 0;


/*------------------------------------------------------------------------
 * void OS_ADPT_Allocate_Memory
 * Purpose:    OS allocate memory.
 * Parameters:
 *    Input:    memory pool pointer and allocate memory size.
 * returns :    memory pointer, NULL for allocate failed.
 *------------------------------------------------------------------------
 */
void *OS_ADPT_Allocate_Memory(void *memory_pool, unsigned long size)
{
    void        *pointer = NULL ;


	alloc += size;
	alloctimes++;
	if	(memory_pool)	{}	/*To Avoid Compiler warnings*/
	pointer = (void *) malloc(size);

    return (pointer) ;
} /* end of OS_ADPT_Allocate_Memory */

/*------------------------------------------------------------------------
 * void OS_ADPT_Deallocate_Memory
 * Purpose:    OS deallocate memory.
 * Parameters:
 *    Input:    memory pointer.
 * returns :    None.
 *------------------------------------------------------------------------
 */
void OS_ADPT_Deallocate_Memory(void *memory)
{
	
	
	
	if (memory) {
		dealloc += _msize(memory);
		dealloctimes++;
		free(memory);
	}

} /* end of OS_ADPT_Deallocate_Memory */


