;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;   initstmem.s
;
;Abstract:
;
;       The static memory initialization routines.
;
;Environment:
;
;       ARM RealView Developer Suite
;
;Revision History:
;   
;   2006/07/17  David Tsai  Create  
;

        PRESERVE8
; Twokey 061214
        INCLUDE ..\..\inc\sysopt.inc

        AREA   InitSTMEM, CODE, READONLY            ; name this block of code


; --- Static memory control register locations

STMEM_TACC          EQU     0xd0010000      ; Address of static memory tACC
STMEM_TACCH         EQU     0xd0010004      ; Address of static memory tACCH
STMEM_TWP           EQU     0xd0010008      ; Address of static memory tWP
STMEM_TWPH          EQU     0xd001000c      ; Address of static memory tWPH

; --- Static memory initialization
;CY 0907 TEST
;STMEM_TACC_SETTING     EQU 0x00000009      ; Setting of static memory tACC
;STMEM_TACCH_SETTING    EQU 0x00000009      ; Setting of static memory tACCH
;STMEM_TWP_SETTING      EQU 0x00000009      ; Setting of static memory tWP
;STMEM_TWPH_SETTING     EQU 0x00000009      ; Setting of static memory tWPH     
                    
STMEM_TACC_SETTING      EQU CODE_DEVICE_OPT     ; Setting of static memory tACC
STMEM_TACCH_SETTING     EQU CODE_DEVICE_OPT     ; Setting of static memory tACCH
STMEM_TWP_SETTING       EQU CODE_DEVICE_OPT     ; Setting of static memory tWP
STMEM_TWPH_SETTING      EQU CODE_DEVICE_OPT     ; Setting of static memory tWPH         
        EXPORT  InitStmem

InitStmem   FUNCTION

        LDR     r1, =STMEM_TACC
        LDR     r0, =STMEM_TACC_SETTING
        STR     r0, [r1]

        LDR     r1, =STMEM_TACCH
        LDR     r0, =STMEM_TACCH_SETTING
        STR     r0, [r1]       

        LDR     r1, =STMEM_TWP
        LDR     r0, =STMEM_TWP_SETTING
        STR     r0, [r1]       

        LDR     r1, =STMEM_TWPH
        LDR     r0, =STMEM_TWPH_SETTING
        STR     r0, [r1]       
        
    BX LR
    ENDFUNC

    END                                         ; mark the end of this file
