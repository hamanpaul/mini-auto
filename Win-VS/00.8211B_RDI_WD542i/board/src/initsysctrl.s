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
        

 IF (CHIP_OPT == CHIP_OPT_PA9002D)      
    ;Lucian:  ©w96MHz DRAM Confiug

		LDR     r1, =SYSCTRL_CPU_PLL   ;Switch to Xin and initialize PLL 
        LDR     r0, =CLK_SRC_OPT
        STR     r0, [r1] 

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
        LDR     r1, =SYSCTRL_CLK_DIV1 
        STR     r0, [r1] 
    
        LDR     r1, =SYSCTRL_CLK_DDR_PCTL 
        LDR     r0, =SYSCTRL_CLK_DDR_PCTL_INT
        STR     r0, [r1] 
        
    ;
    ; Set SDRAM Timing Parameter 0 Register     
    ;
        LDR     r1, =SDRAM_TIME_PARAM0
        LDR     r0, =SDRAM_TIME_PARAM0_INIT
        STR     r0, [r1]
    ;
    ; Set SDRAM Timing Parameter 1 Register     
    ;
        LDR     r1, =SDRAM_TIME_PARAM1
        LDR     r0, =SDRAM_TIME_PARAM1_INIT
        STR     r0, [r1]
 ELSE
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
 ENDIF
 
 IF (CHIP_OPT >= CHIP_OPT_A1016A)
        LDR     r0, =0    ; WDT clock source select: System crystal (24MHz)    
        LDR     r1, =CLK_ADC_DIV
        SUB     r1, r1, #1
        MOV     r1, r1, LSL #8
        ORR     r0, r0, r1
                
        LDR     r1, =SYSCTRL_CLK_DIV2 
        STR     r0, [r1]         
 ENDIF

 IF (CHIP_OPT < CHIP_OPT_A1016A)
       ;invert DRAM clock for 96 MHz
     IF ( (CHIP_OPT == CHIP_OPT_PA9002D) && (CLK_DDRSYS_DIV == 2) && (DRAM_CLK_INV_FIRST == 1))
        ldr r0, =0xc0040038
        ldr r1, [r0]
        bic r1, r1, #0x20000000
        orr r1, r1, #0x20000000
        str r1, [r0]
     ENDIF
        LDR     r0, =CLK_USB_DIV
        SUB     r0, r0, #1
        LDR     r1, =CLK_ADC_DIV
        SUB     r1, r1, #1
        MOV     r1, r1, LSL #8
        ORR     r0, r0, r1
        LDR     r1, =CLK_SYS_DIV
        SUB     r1, r1, #1
        MOV     r1, r1, LSL #16
        ORR     r0, r0, r1
        LDR     r1, =CLK_DDRSYS_DIV
        SUB     r1, r1, #1
        MOV     r1, r1, LSL #24
        ORR     r0, r0, r1
        LDR     r1, =SYSCTRL_CLK_DIV2 
        STR     r0, [r1] 
        
        ;Adjust DRAM driving current: Default=4mA,High slew. for ESMT
        ;Adjust DRAM driving current: Default=4mA,slow slew. for Elixir
        ldr r0, =0xd00b001c
        ldr r1, [r0]
        bic r1, r1, #0x0000000f
        orr r1, r1, #0x00000003 
        ;orr r1, r1, #0x00000002 
        str r1, [r0]
    
        
        ;Switch to PLL 
        LDR     r1, =SYSCTRL_CPU_PLL  
        LDR     r0, [r1]
        ORR     r0, r0, #4
        STR     r0, [r1] 

     IF ( (CHIP_OPT == CHIP_OPT_PA9002D) && (CLK_DDRSYS_DIV == 2) && (DRAM_CLK_INV_FIRST == 0) )
        ldr r0, =0xc0040038
        ldr r1, [r0]
        bic r1, r1, #0x20000000
        orr r1, r1, #0x20000000
        str r1, [r0]
     ENDIF
 ENDIF

        LDR     r1, =SYSCTRL_CLK_ENA
        LDR     r0, =SYSCTRL_CLK_ENA_SETTING
        STR     r0, [r1]                
        
 IF (BOOT_SRC_OPT == BOOT_SRC_ROM)       ;CY 1023    
        LDR     r1, =SYSCTRL_RST
        LDR     r0, =SYSCTRL_RST_SETTING
        STR     r0, [r1]  
        
        LDR     r1, =SYSCTRL_RST
        LDR     r0, =SYSCTRL_RST_CLEAR
        STR     r0, [r1]               
 ENDIF

 IF (CHIP_OPT <= CHIP_OPT_PA9002D)       
        LDR     r1, =HIU_BYPASS
        LDR     r0, =HIU_BYPASS_SETTING
        STR     r0, [r1]          
 ENDIF       
        BX LR
        ENDFUNC

        END                                                     ; mark the end of this file
