;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;	initmmu.s
;
;Abstract:
;
;   	The MMU initialization routines.
;
;  	This code provides basic initialization for an ARM92XT including:
;  	- creation of a 16KB level 1 page table in physical memory (this will probably
;    		need to be changed for your target)
;  	- possible modifying of this table to perform ROM/RAM remapping
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

        AREA   InitMMU, CODE, READONLY      ; name this block of code

INDEX_WAY	EQU	0x40000000  ; value to add to way
INDEX_SET	EQU	0x00000020  ; value to add to index
INDEX_MAX	EQU  0x00000400  ; end value for index

        EXPORT board_activate_mmu
	 EXPORT Test_Clean_Dcache
	 EXPORT	Disable_Dcache
board_activate_mmu

;write TTB
	mcr     p15, 0, r0, c2, c0, 0
;write domains
	ldr     r0, = (3 << 2) | (3 << 4) 	;/* domain 1 = client, 2 = manager, others = none*/
	mcr     p15, 0, r0, c3, c0, 0
	
	mov     r2, #0
	
;clear FCSE registers (to avoid any unexpected address translation)
	mcr     p15, 0, r2, c13, c0, 0
;invalidate ALL TLBs in set-associative area
	mcr     p15, 0, r2, c8, c7, 0
;attempt to invalidate fully associative TLB part
	;mov     r1, #8 	;// there are 8 such entries
;LABEL_TLB_CLEAN
	;mrc     p15, 0, r0, c10, c0, 0
	;bic     r0, r0, #1
	;mcr     p15, 0, r0, c10, c0, 0
	;subs    r1, r1, #1
	;bne     LABEL_TLB_CLEAN
;invalidate all data cache, without flushing !
	mcr     p15, 0, r2, c7, c6, 0
;invalidate all instr cache
	mcr     p15, 0, r2, c7, c5, 0
;activate
	mrc     p15, 0, r0, c1, c0, 0
	ldr     r1, =  1 << 2 | 2 | 1		;D-cache enable, Align check, MMU enable
	orr     r0, r0, r1
	mcr     p15, 0, r0, c1, c0, 0
	nop
	nop
;done
	mov     pc, lr

Test_Clean_Dcache

tc_loop 
       MRC p15, 0, r15, c7, c14, 3 ; test and clean
	BNE tc_loop

	ldr	r0, =0
       mcr     p15, 0, r0, c7, c10, 4  ; Drain write buffer
       mov     pc, lr

Clean_DCache_ALL


        mov     r1, #0x0                    ; Initialize index counter

CleanD_outer_loop

        mov     r0, #0x0                    ; Initialize segment counter

CleanD_inner_loop

        orr     r2, r1, r0                  ; Make segment and line address

        mcr     p15, 0, r2, c7, c10, 2      ; Clean Data Cache entry
        add     r0, r0, #INDEX_SET
        cmp     r0, #INDEX_MAX
        bne     CleanD_inner_loop
        adds    r1, r1, #INDEX_WAY
        bne     CleanD_outer_loop

	ldr	r0, =0
        mcr     p15, 0, r0, c7, c10, 4  ; Drain write buffer
;clear FCSE registers (to avoid any unexpected address translation)
	mcr     p15, 0, r2, c13, c0, 0
;invalidate ALL TLBs in set-associative area
	mcr     p15, 0, r2, c8, c7, 0
        mov     pc, lr

Disable_Dcache
       ldr       r0, =0;
	mrc     p15, 0, r0, c7, c14, 3  ; flush the cache
	mrc     p15, 0, r0, c1, c0, 0
	bic      r0, r0, #(1<<2)
	mcr     p15, 0, r0, c1, c0, 0
	nop
	nop
	mov     pc, lr        
                END                          ; mark the end of this file

