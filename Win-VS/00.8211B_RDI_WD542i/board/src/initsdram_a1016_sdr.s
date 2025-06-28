;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;       initsdram.s
;
;Abstract:
;
;       The SDRAM initialization routines.
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

        AREA   InitSDRAM, CODE, READONLY                        ; name this block of code

; --- SDRAM Control Register locations

SDRAM_TIME_PARAM0               EQU     0xc0040000              ; Address of DDR-SDRAM Timing Parameter 0 Register

SDRAM_TIME_PARAM1               EQU     0xc0040004              ; Address of DDR-SDRAM Timing Parameter 1 Register

SDRAM_CONFIG                    EQU     0xc0040008              ; Address of DDR-SDRAM Configuration Register

SDRAM_EXT_BANK                  EQU     0xc004000c              ; Address of DDR-SDRAM External Bank Base/Size Register

SDRAM_EXT_MODE                  EQU     0xc0040010              ; Address of DDR-SDRAM External Mode Register

SDRAM_DFI_RW                    EQU     0xc0040014              ; Address of DDR-SDRAM DFI/ RW mode register

SDRAM_DFI_PHYA                  EQU     0xc0040018              ; Address of DDR-SDRAM DFI/ operation mode A register

SDRAM_DFI_PHYB                  EQU     0xc004001c              ; Address of DDR-SDRAM DFI/ operation mode B register

SDRAM_CHANELC                   EQU     0xc0040034              ; Address of DDR-SDRAM Chanel Counter Register

SDRAM_DDLCTL                    EQU     0xc0040038              ; Address of DDR-SDRAM DLL Control Register


; --- SDRAM initialization

SDRAM_DDLCTL_INIT               EQU     0x80000002

SDRAM_DFI_RW_INIT               EQU     0x20000000

SDRAM_CHANELC_INIT              EQU     0x00000000

SDRAM_EXT_BANK_INIT             EQU     0x18003228              ; 32 Mb per bank (0101)
                                                                ; 32 bit external bus width (10)
                                                                ; 128 Mb size (10)
                                                                ; x32 column address (11)
                                                                ; 0x80000000 SDRAM base address (0x800)
                                                                ; Enable Bank (1)               
                                                                ; i.e.
                                                                ; 128 Mb = 32 Mb/bank * 4 bank = 2^25 Mb/bank * 4 bank =
                                                                ; 2^12 *  2^8 *   2^5 Mb/bank * 4 bank
                                                                ; rowAddr colAddr x32
                                                        
SDRAM_EXT_MODE_INIT             EQU     0x00000100              ; DDR-SDRAM External Mode Register initial value

SDRAM_TIME_PARAM0_INIT          EQU     0x0410030a              ; tCL (10) - 2 clocks
                                                                ; tWR (00) - 0 + 1 clocks
                                                                ; tRC (0000) - 0 + 4 clocks
                                                                ; tRCD (000) - 0 + 2 clocks
                                                                ; tRP (000) - 0 + 2 clocks
                                                                ; tRAS (0001) - 1 clocks
                                                                
SDRAM_TIME_PARAM1_INIT          EQU     0x03130200              ; tREF (0x0130) - of no use at this time                                                
                                                                ; auto refresh count (0011) - 3 times
                                                                ; precharge all count (0001) - 1 time
                                                                                                                
SDRAM_CK_CKB                    EQU     0x00000200              ; CK and CKB (1)

SDRAM_CKE                       EQU     0x00000B00              ; CK and CKB (1)
                                                                ; CKE (1)
                                                                ; SDR SDRAM controller select (1)
                                                        

SDRAM_PRECHARGE_ALL0            EQU     0x00000B30              ; precharge all count of precharge all (1)
                                                                ; stagger type refresh (1)
                                                                ; CK and CKB (1)
                                                                ; CKE (1)
                                                                ; SDR SDRAM controller select (1)
                                                                                                                
SDRAM_AUTO_REFRESH              EQU     0x00003B38              ; auto refresh of auto refresh count (1)
                                                                ; precharge all count of precharge all (1)
                                                                ; stagger type refresh (1)
                                                                ; CK and CKB (1)
                                                                ; CKE (1)
                                                                ; SDR SDRAM controller select (1)                                                                                                                       

SDRAM_MODE_REG_SET              EQU     0x00001B04              ; mode register set - MRS (1)
                                                                ; CK and CKB (1)
                                                                ; CKE (1)
                                                                ; SDR SDRAM controller select (1)                                                                                                                                       

SDRAM_DELAY_200US               EQU     0x00000100              ; loop count for 200 us delay
                                                        
SDRAM_DELAY_200CLK              EQU     0x00000100              ; loop count for 200 clock delay

SDRAM_MODE_REG_SET_ONGO         EQU     0x00000004              ; ongoing of mode register set (1)

SDRAM_AUTO_REFRESH_ONGO         EQU     0x00000008              ; ongoing of auto prefetch count of auto prefetch (1)

SDRAM_PRECHARGE_ALL_ONGO        EQU     0x00000010              ; ongoing of precharge all count of precharge all (1)                                                           

        EXPORT  InitSdram

InitSdram       FUNCTION

;
; Set DDR-SDRAM DLL Control Register 0xc0040038
;
        LDR     r1, =SDRAM_DDLCTL
        LDR     r0, =SDRAM_DDLCTL_INIT
        STR     r0, [r1]

;
; Set DDR-SDRAM Chanel Counter Register 0xc0040034
;
        LDR     r1, =SDRAM_CHANELC
        LDR     r0, =SDRAM_CHANELC_INIT
        STR     r0, [r1]

;
; Set SDRAM External Bank Register 0xc004000c
;
        LDR     r1, =SDRAM_EXT_BANK
        LDR     r0, =SDRAM_EXT_BANK_INIT
        STR     r0, [r1]

;
; Set DDR-SDRAM DFI/ RW mode register 0xc0040014
;
        LDR     r1, =SDRAM_DFI_RW
        LDR     r0, =SDRAM_DFI_RW_INIT
        STR     r0, [r1]

;
; Set DDR-SDRAM External Mode Register 0xc0040010
;
        LDR     r1, =SDRAM_EXT_MODE
        LDR     r0, =SDRAM_EXT_MODE_INIT
        STR     r0, [r1]

;
; Set SDRAM Timing Parameter 0 Register 0xc0040000
;
        LDR     r1, =SDRAM_TIME_PARAM0
        LDR     r0, =SDRAM_TIME_PARAM0_INIT
        STR     r0, [r1]

;
; Set SDRAM Timing Parameter 1 Register 0xc0040004
;
        LDR     r1, =SDRAM_TIME_PARAM1
        LDR     r0, =SDRAM_TIME_PARAM1_INIT
        STR     r0, [r1]

;
; CK and CKB    
;
        LDR     r1, =SDRAM_CONFIG   ; 0xc0040008
        LDR     r0, =SDRAM_CK_CKB
        STR     r0, [r1]
        
;
; CKE   
;
        LDR     r1, =SDRAM_CONFIG   ; 0xc0040008
        LDR     r0, =SDRAM_CKE
        STR     r0, [r1] 

;
; 200 us Delay
;
        MOV     r0, #SDRAM_DELAY_200US
LABEL_SDRAM_DELAY_200US
        SUBS    r0, r0, #0x1
        BHI     LABEL_SDRAM_DELAY_200US
        
;
; Precharge All 
;
        LDR     r1, =SDRAM_CONFIG           ; 0xc0040008
        LDR     r0, =SDRAM_PRECHARGE_ALL0
        STR     r0, [r1]
LABEL_SDRAM_PRECHARGE_ALL0        
        LDR     r0, [r1, #0]
        TST     r0, #SDRAM_PRECHARGE_ALL_ONGO
        BNE     LABEL_SDRAM_PRECHARGE_ALL0 

;
; Auto Refresh  
;
        LDR     r1, =SDRAM_CONFIG           ; 0xc0040008
        LDR     r0, =SDRAM_AUTO_REFRESH
        STR     r0, [r1]
LABEL_SDRAM_AUTO_REFRESH       
        LDR     r0, [r1, #0]
        TST     r0, #SDRAM_AUTO_REFRESH_ONGO
        BNE     LABEL_SDRAM_AUTO_REFRESH 
        
;
; Mode Register Set     
;
        LDR     r1, =SDRAM_CONFIG           ; 0xc0040008
        LDR     r0, =SDRAM_MODE_REG_SET
        STR     r0, [r1]              
LABEL_SDRAM_MODE_REG_SET        
        LDR     r0, [r1, #0]
        TST     r0, #SDRAM_MODE_REG_SET_ONGO
        BNE     LABEL_SDRAM_MODE_REG_SET 
                        
;
; 200 Clock Delay
;
        MOV     r0, #SDRAM_DELAY_200CLK
LABEL_SDRAM_DELAY_200CLK
        SUBS    r0, r0, #0x1
        BHI     LABEL_SDRAM_DELAY_200CLK
                
        BX LR
        ENDFUNC

        END                                                     ; mark the end of this file
