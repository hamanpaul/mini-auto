;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;       init.s
;
;Abstract:
;
;       The initialization routines.
;
;Environment:
;
;       ARM RealView Developer Suite
;
;Revision History:
;       
;       2005/08/26      David Tsai      Create  
;

        PRESERVE8
        
        INCLUDE ..\..\inc\sysopt.inc
        
; --- Standard definitions of mode bits and interrupt (I & F) flags in PSRs

Mode_USR        EQU     0x10
Mode_FIQ        EQU     0x11
Mode_IRQ        EQU     0x12
Mode_SVC        EQU     0x13
Mode_ABT        EQU     0x17
Mode_UND        EQU     0x1B
Mode_SYS        EQU     0x1F ; available on ARM Arch 4 and later

I_Bit           EQU     0x80 ; when I bit is set, IRQ is disabled
F_Bit           EQU     0x40 ; when F bit is set, FIQ is disabled


Len_FIQ_Stack    EQU     2048
Len_IRQ_Stack    EQU     2048

Offset_FIQ_Stack         EQU     0
Offset_IRQ_Stack         EQU     Offset_FIQ_Stack + Len_FIQ_Stack
Offset_SVC_Stack         EQU     Offset_IRQ_Stack + Len_IRQ_Stack

        AREA   INIT, CODE, READONLY   ; name this block of code

        ENTRY

        EXPORT  Reset_Handler
Reset_Handler   FUNCTION

; stack_base could be defined above, or located in a scatter file

        LDR     r0, stack_base ;

; Enter each mode in turn and set up the stack pointer
        MSR     CPSR_c, #Mode_FIQ:OR:I_Bit:OR:F_Bit
        SUB     sp, r0, #Offset_FIQ_Stack

        MSR     CPSR_c, #Mode_IRQ:OR:I_Bit:OR:F_Bit
        SUB     sp, r0, #Offset_IRQ_Stack
 
        MSR     CPSR_c, #Mode_SVC:OR:I_Bit:OR:F_Bit
        SUB     sp, r0, #Offset_SVC_Stack

; Leave core in SVC mode

        IMPORT  InitTCMentry [WEAK]         ; Import label to TCM init code, but don't fault if not present
        ;BL      InitTCMentry               ; ignore call if function not present

    IF ( (CHIP_OPT == CHIP_OPT_A1016A) || (CHIP_OPT == CHIP_OPT_A1018A) )        ; CY 1023  

    ELSE
        IMPORT  InitSysCtrl [WEAK]          ; Import label to system control init code, but don't fault if not present
        BL      InitSysCtrl                 ; And execute it.  
    ENDIF
        
        IMPORT  InitStmem [WEAK]            ; Import label to static memory init code, but don't fault if not present
        BL      InitStmem                   ; And execute it.
          
        IF (BOOT_SRC_OPT == BOOT_SRC_ROM)   ; CY 0718 CY 1023   
          
        IMPORT  InitSdram [WEAK]            ; Import label to SDRAM init code, but don't fault if not present
        BL      InitSdram                   ; And execute it.  
        
        ENDIF
        
        IMPORT  InitMMUentry [WEAK]         ; Import label to MMU init code, but don't fault if not present 
        ;BL      InitMMUentry               ; And execute it.
        
    IF (CHIP_OPT == CHIP_OPT_PA9001D)       ; CY 1023        
        IMPORT  InitCache [WEAK]            ; Import label to cache init code, but don't fault if not present
        BL      InitCache                       ; And execute it.       
    ENDIF
        
        MRS     r0, CPSR                    ; Enable FIQ and IRQ        
        BIC     r0, r0, #0xc0
        MSR     CPSR_c, r0

; Branch to C Library entry point

        IMPORT  __main                      ; before MMU enabled import label to __main
        LDR     r12,=__main                 ; save this in register for possible long jump

        BX      r12                         ; branch to __main
        ENDFUNC
        
; --- Location and sizes of stacks
                IF :DEF: LOCATIONS_IN_CODE
stack_base      DCD      0x40000
                ELSE

                IF :DEF: USE_SCATTER_SYMS ;CY 1023

                IMPORT      ||Image$$RAM_STACK$$ZI$$Base||
                IMPORT      ||Image$$RAM_STACK$$ZI$$Limit||

stack_base      DCD         ||Image$$RAM_STACK$$ZI$$Limit||
stack_limit     DCD         ||Image$$RAM_STACK$$ZI$$Base||

                ENDIF
                ENDIF

        END                                 ; mark the end of this file

