;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;       initchip.s
;
;Abstract:
;
;       The system control initialization routines.
;
;Environment:
;
;       ARM RealView Developer Suite
;
;Revision History:
;       
;       2006/07/17      David Tsai      Create  
;

        PRESERVE8
        
        INCLUDE ..\..\inc\sysopt.inc

        AREA   InitSYSCTRL, CODE, READONLY                      ; name this block of code

; --- Sytem control register locations
; CY 0907
SYSCTRL_CLK_ENA                 EQU     0xd00b0000              ; System control clock enable register
SYSCTRL_CLK_DIV1                EQU     0xd00b0004              ; System control clock divisor 1
SYSCTRL_RST                     EQU     0xd00b000c              ; System control reset
SYSCTRL_CPU_PLL                 EQU     0xd00b0010              ; System cotrol PLL
SYSCTRL_CLK_DIV2                EQU     0xd00b0018              ; System control clock divisor 2
SYSCTRL_CLK_DDR_PCTL            EQU     0xd00b001c              ; 0xd00b0014

HIU_BYPASS                      EQU     0xc0080038              ; Host interface unit bypass mode

SDRAM_TIME_PARAM0               EQU     0xc0040000              ; Address of SDRAM Timing Parameter 0 Register
SDRAM_TIME_PARAM1               EQU     0xc0040004              ; Address of SDRAM Timing Parameter 1 Register
SDRAM_TIME_PARAM0_INIT          EQU     0x00511312              ; tCL (10) - 2 clocks
                                                                ; tWR (00) - 0 + 1 clocks
                                                                ; tRC (0000) - 0 + 4 clocks
                                                                ; tRCD (000) - 0 + 2 clocks
                                                                ; tRP (000) - 0 + 2 clocks
                                                                ; tRAS (0001) - 1 clocks
                                                                                
SDRAM_TIME_PARAM1_INIT          EQU     0x00130540              ; tREF (0x0130) - of no use at this time                                        
                                                                ; auto refresh count (0011) - 3 times
                                                                ; precharge all count (0001) - 1 time


; --- System control initialization

SYSCTRL_CLK_ENA_SETTING         EQU     0xffffffff              ; All                   
SYSCTRL_RST_SETTING             EQU     0x07fff9ff              ; Set all except STMEM and MGPIO
SYSCTRL_RST_CLEAR               EQU     0x00000000              ; Clear all
SYSCTRL_CLK_DDR_PCTL_INT        EQU     0x00000012              ; 0xd00b001c
HIU_BYPASS_SETTING              EQU     0x00000003              ; Hardware bypass - depends on {HA[0], HCS2, HCS1} 
                                                        
        EXPORT  InitSysCtrl

InitSysCtrl     FUNCTION

        LDR     r0, =CLK_MCK_DIV
        SUB     r0, r0, #1
        LDR     r1, =CLK_IDU_DIV
        SUB     r1, r1, #1
        MOV     r1, r1, LSL #8
        ORR     r0, r0, r1
        LDR     r1, =CLK_WDT_DIV
        SUB     r1, r1, #1
        MOV     r1, r1, LSL #16
        ORR     r0, r0, r1

        LDR	    r1, =CLK_JPEG_VLC_DIV
	    SUB	    r1, r1, #1
	    MOV	    r1, r1, LSL #26
	    ORR     r0, r0, r1
	
        LDR     r1, =SYSCTRL_CLK_DIV1 
        STR     r0, [r1] 

 
        LDR     r0, =0    ; WDT clock source select: Exteral crystal (48MHz)        
        LDR     r1, =CLK_ADC_DIV
        SUB     r1, r1, #1
        MOV     r1, r1, LSL #8
        ORR     r0, r0, r1                
        LDR     r1, =SYSCTRL_CLK_DIV2 
        STR     r0, [r1]        

        LDR     r1, =SYSCTRL_CLK_ENA
        LDR     r0, =SYSCTRL_CLK_ENA_SETTING
        STR     r0, [r1]                
              
        BX LR
        ENDFUNC

        END                                                     ; mark the end of this file
