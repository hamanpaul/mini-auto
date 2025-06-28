;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;	vectors.s
;
;Abstract:
;
;   	The interrupt vectors.
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

        AREA Vect, CODE, READONLY

; A vector table and dummy exception handlers


; *****************
; Exception Vectors
; *****************

; Note: LDR PC instructions are used here, though branch (B) instructions
; could also be used, unless the ROM is at an address >32MB.

        ENTRY
        EXPORT  START_0   
START_0 FUNCTION            

        LDR     PC, Reset_Addr
        LDR     PC, Undefined_Addr
        LDR     PC, SWI_Addr
        LDR     PC, Prefetch_Addr
        LDR     PC, Abort_Addr
        NOP                             ; Reserved vector
        LDR     PC, IRQ_Addr
        LDR     PC, FIQ_Addr
        
        IMPORT  Reset_Handler           ; In init.s
        ;//IMPORT  IRQ_Handler             ; In int_handler.c
        ;//IMPORT  FIQ_Handler             ; In ini_handler.c
        IMPORT  UCOS_IRQHandler         ; In os_cpu_a.s
        IMPORT  UCOS_FIQHandler        	; In os_cpu_a.s
;        IMPORT  inc_clock   [WEAK]      ;
        
Reset_Addr      DCD     Reset_Handler
Undefined_Addr  DCD     Undefined_Handler
SWI_Addr        DCD     SWI_Handler
Prefetch_Addr   DCD     Prefetch_Handler
Abort_Addr      DCD     Abort_Handler
; cytsai: modify for uCOS-II interrupt handling
;IRQ_Addr        DCD     IRQ_Handler
;FIQ_Addr        DCD     FIQ_Handler
IRQ_Addr        DCD     UCOS_IRQHandler
FIQ_Addr        DCD     UCOS_FIQHandler

; ************************
; Exception Handlers
; ************************

      IMPORT  marsUndefineHandler
      IMPORT  marsSWIHandler
      IMPORT  marsPrefetchHandler
      IMPORT  marsAbortHandler

; The following dummy handlers do not do anything useful in this example.
; They are set up here for completeness.

Undefined_Handler
        bl marsUndefineHandler
        B       Undefined_Handler
        
SWI_Handler
        bl marsSWIHandler
        B       SWI_Handler     
        
Prefetch_Handler
        bl marsPrefetchHandler
        B       Prefetch_Handler
        
Abort_Handler
        bl marsAbortHandler
        B       Abort_Handler
        
;IRQ_Handler
;        B       IRQ_Handler
;FIQ_Handler
;        B       FIQ_Handler
        
        END

