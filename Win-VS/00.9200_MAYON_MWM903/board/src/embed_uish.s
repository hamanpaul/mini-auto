;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;	embed_uish.s
;
;Abstract:
;
;   	The heap region.
;
;  	This implementation of __user_initial_stackheap places the
;  	heap at the location of the symbol bottom_of_heap.  
;
;  	This symbol is placed by the scatter file.  
;  	It assumes that the application mode stack has been placed
;  	in the reset handler.  
;
;Environment:
;
;   	ARM RealView Developer Suite
;
;Revision History:
;	
;	2005/08/26	David Tsai	Create	
;

        PRESERVE8

        AREA   UISH, CODE, READONLY      ; name this block of code

    IF :DEF: LOCATIONS_IN_CODE
heap_base       DCD     0x14000
    ELSE
    
    IF :DEF: USE_SCATTER_SYMS	;CY 1023

                IMPORT      ||Image$$RAM_HEAP$$ZI$$Base||
                IMPORT      ||Image$$RAM_HEAP$$ZI$$Limit||

heap_base       DCD     ||Image$$RAM_HEAP$$ZI$$Base||
heap_limit      DCD     ||Image$$RAM_HEAP$$ZI$$Limit||

    ENDIF
    ENDIF

        EXPORT __user_initial_stackheap

__user_initial_stackheap FUNCTION
    LDR   r0,heap_base
    MOV   pc,lr
    ENDFUNC
    
        END
        

;; The following is the equivalent implementation of the above in C
;;
;; extern unsigned int bottom_of_heap;    //defined in heap.s
;;
;; __value_in_regs struct __initial_stackheap __user_initial_stackheap(
;;         unsigned R0, unsigned SP, unsigned R2, unsigned SL)
;; {
;;     struct __initial_stackheap config;
;;
;;     config.heap_base = (unsigned int)&bottom_of_heap; // defined in heap.s
;;                                                       // placed by scatterfile
;;     config.stack_base = SP;   // inherit SP from the execution environment
;;
;;     return config;
;; }

