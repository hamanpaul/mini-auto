;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;	uish.s
;
;Abstract:
;
;   	The stack and heap region.
;
;  	This implementation of __user_initial_stackheap places the stack and heap
;  	using absolute value parameters.
;  	The memory locations were chosen to be suitable to the Integrator platform.
;
;  	The default build implements a one region stack and heap model
;  	To implement a two region model, predefine TWO_REGION_MODEL on the command line.
;  	ie: armasm -g --predefine "TWO_REGION_MODEL SETL {TRUE}" uish.s
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
        
        IF :DEF: TWO_REGION_MODEL        ; if a two-region memory model is needed
        IMPORT __use_two_region_memory   ; this symbol must be imported into the image
        ENDIF
        
        EXPORT __user_initial_stackheap

__user_initial_stackheap FUNCTION

        IF :DEF: TWO_REGION_MODEL       ; Two-region model for stack and heap

    LDR   r0,=0x28000000 ;HB            ; The values chosen here place the heap in the
    LDR   r1,=0x40000    ;SB            ; AP SSRAM module (512KB).
    LDR   r2,=0x28080000 ;HL            ; The stack is placed in Core Module SSRAM.
    LDR   r3,=0x20000    ;SL            ; Compile all modules with --apcs /swst to enable software stack checking

        ELSE                            ; One-region model for stack and heap

    LDR   r0,=0x20000 ;HB               ; The stack and heap are placed in Core Module SSRAM.
    LDR   r1,=0x40000 ;SB
    ; r2 not used (HL)                  ; The heap limit and stack limit are not used
    ; r3 not used (SL)                  ; in a one-region model

        ENDIF

    MOV   pc,lr
    ENDFUNC
    
        END

;; The following is an equivalent implementation of the above in C
;;
;; #ifdef TWO_REGION_MODEL
;; #pragma import(__use_two_region_memory)
;; #endif
;;
;; __value_in_regs struct __initial_stackheap __user_initial_stackheap(
;;         unsigned R0, unsigned SP, unsigned R2, unsigned SL)
;; {
;;     struct __initial_stackheap config;
;; #ifdef TWO_REGION_MODEL 
;;     config.heap_base = (unsigned int)0x28000000;
;;     config.stack_base = (unsigned int)0x40000;
;;     config.heap_limit = (unsigned int)0x28080000;
;;     config.stack_limit = (unsigned int)0x20000;
;; #else
;;     config.heap_base = (unsigned int)0x20000;
;;     config.stack_base = (unsigned int)0x40000;
;; #endif
;;     return config;
;; }
;;

