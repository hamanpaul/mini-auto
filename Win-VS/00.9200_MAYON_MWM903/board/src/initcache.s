;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;	initcache.s
;
;Abstract:
;
;   	The cache initialization routines.
;	It is called by $Sub$$main(), before the main application is entered
;
;Environment:
;
;   	ARM RealView Developer Suite
;
;Revision History:
;	
;	2005/08/26	David Tsai	Create	
;

;CY 1023

        PRESERVE8

        AREA	InitCACHE, CODE, READONLY      ; name this block of code

        EXPORT	InitCache
InitCache	FUNCTION

;
; Set global core configurations
;===============================
;
        MRC     p15, 0, r0, c1, c0, 0       ; read CP15 register 1 into r0

        ORR     r0, r0, #(0x1  <<12)        ; enable I Cache
        ORR     r0, r0, #(0x1  <<2)         ; enable D Cache

        MCR     p15, 0, r0, c1, c0, 0       ; write CP15 register 1
        MOV     pc, lr
        ENDFUNC
        
        END                                 ; mark the end of this file

