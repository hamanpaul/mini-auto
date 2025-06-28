;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;	initstmem.s
;
;Abstract:
;
;   	The static memory initialization routines.
;
;Environment:
;
;   	ARM RealView Developer Suite
;
;Revision History:
;	
;	2006/07/17	David Tsai	Create	
;

        PRESERVE8
; Twokey 061214
        INCLUDE	..\..\inc\sysopt.inc


        AREA   MMU_CODE, CODE, READONLY			; name this block of code
		
        EXPORT	boardActivateMMU

boardActivateMMU	FUNCTION

;write TTB
	mcr     p15, 0, r0, c2, c0, 0
;write domains
	ldr     r0, = (1 << 2) | (3 << 4) 	;/* domain 1 = client, 2 = manager, others = none*/
	mcr     p15, 0, r0, c3, c0, 0
;clear FCSE registers (to avoid any unexpected address translation)
	mcr     p15, 0, r2, c13, c0, 0
;invalidate ALL TLBs in set-associative area
	mcr     p15, 0, r2, c8, c7, 0
;attempt to invalidate fully associative TLB part
	mov     r1, #8 	;// there are 8 such entries
LABEL_TLB_CLEAN
	mrc     p15, 0, r0, c10, c0, 0
	bic     r0, r0, #1
	mcr     p15, 0, r0, c10, c0, 0
	subs    r1, r1, #1
	bne     LABEL_TLB_CLEAN
;invalidate all data cache, without flushing !
	mcr     p15, 0, r0, c7, c6, 0
;invalidate all instr cache
	mcr     p15, 0, r0, c7, c5, 0
;activate
	mrc     p15, 0, r0, c1, c0, 0
	ldr     r1, =  1 << 2 | 2 | 1		;D-cache enable, Align check, MMU enable
	orr     r0, r0, r1
	mcr     p15, 0, r0, c1, c0, 0
	nop
	nop
;done
	mov     pc, lr
    

 	ENDFUNC

	END                          				; mark the end of this file
