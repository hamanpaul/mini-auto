;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;	initsdram.s
;
;Abstract:
;
;   	The SDRAM initialization routines.
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

        AREA   InitSDRAM, CODE, READONLY			; name this block of code


; --- SDRAM Control Register locations

SDRAM_TIME_PARAM0		EQU     0xc0040000		; Address of SDRAM Timing Parameter 0 Register

SDRAM_TIME_PARAM1		EQU     0xc0040004		; Address of SDRAM Timing Parameter 1 Register

SDRAM_CONFIG			EQU     0xc0040008		; Address of SDRAM Configuration Register

SDRAM_EXT_BANK			EQU     0xc004000c		; Address of SDRAM External Bank Base/Size Register

SDRAM_EXT_MODE			EQU     0xc0040010		; Address of SDRAM External Mode Register

SDRAM_ARBIT			EQU     0xc0040034		; Address of SDRAM Arbiter Control Register


; --- DDR SDRAM initialization

SDRAM_EXT_BANK_INIT		EQU	0x18002124		; 32 Mb per bank (0100)
								; 16 bit external bus width (10)
								; 128 Mb size (01)
								; x16 column address (10)
								; 0x80000000 SDRAM base address (0x800)
								; Enable Bank (1) 		
								; i.e.
								; 128 Mb = 32 Mb/bank * 4 bank = 2^25 Mb/bank * 4 bank =
								; 2^12 *  2^9 *   2^4 Mb/bank * 4 bank
								; rowAddr colAddr x16
							
SDRAM_TIME_PARAM0_INIT		EQU	0x00a53a32		; tCL (10) - 2 clocks
								; tWR (11) - 3 + 1 clocks
								; tRC (1010) - 10 + 4 clocks
								; tRCD (011) - 3 + 2 clocks
								; tRP (101) - 5 + 2 clocks
								; tRAS (1010) - 10 clocks
								
SDRAM_TIME_PARAM1_INIT	 	EQU	0x00130120		; tREF (0x0120)	- of no use at this time						
								; auto refresh count (0011) - 3 times
								; precharge all count (0001) - 1 time
														
SDRAM_CK_CKB			EQU	0x00000200		; CK and CKB (1)

SDRAM_CKE			EQU	0x00000300		; CK and CKB (1)
								; CKE (1)
								; DDR SDRAM controller select (0)
							

SDRAM_PRECHARGE_ALL0		EQU	0x00000330		; precharge all count of precharge all (1)
								; stagger type refresh (1)
								; CK and CKB (1)
								; CKE (1)
														
SDRAM_EXT_MODE_REG_SELECT	EQU	0x00000004		; (BA1, BA0) = (0, 1)

SDRAM_EXT_MODE_REG_SET		EQU	0x00001304		; extended mode register set (1) - EMRS
								; CK and CKB (1)
								; CKE (1)
								; tWTR (1) - 1 clock (obsolete)
								
SDRAM_MODE_REG_DLL_RESET_SELECT	EQU	0x00000110		; (BA1, BA0) = (0, 0)							
								; reset dll (1)
								; burst length (001)
								
SDRAM_MODE_REG_DLL_RESET_SET	EQU	0x00001304		; mode register set - MRS (1)
								; CK and CKB (1)
								; CKE (1)
								; tWTR (1) - 1 clock obsolete	
								
SDRAM_PRECHARGE_ALL1		EQU	0x00003330		; precharge all count of precharge all (1)
								; stagger type refresh (1)
								; CK and CKB (1)
								; CKE (1)
								; tWTR (3) - 3 clock (obsolete)	
								
SDRAM_AUTO_REFRESH		EQU	0x00003328		; auto refresh of auto refresh count (1)
								; stagger type refresh (1)
								; CK and CKB (1)
								; CKE (1)
								; tWTR (3) - 3 clocks (obsolete)																

SDRAM_MODE_REG_SELECT		EQU	0x00000100		; (BA1, BA0) = (0, 0)							
								; burst length (001)
								
SDRAM_MODE_REG_SET		EQU	0x00001304		; mode register set - MRS (1)
								; CK and CKB (1)
								; CKE (1)
								; tWTR (1) - 1 clock (obsolete)																		

SDRAM_DELAY_200US		EQU	0x00000100		; loop count for 200 us delay
							
SDRAM_DELAY_200CLK		EQU	0x00000100		; loop count for 200 clock delay

SDRAM_MODE_REG_SET_ONGO		EQU	0x00000004		; ongoing of mode register set (1)

SDRAM_AUTO_REFRESH_ONGO		EQU	0x00000008		; ongoing of auto prefetch count of auto prefetch (1)

SDRAM_PRECHARGE_ALL_ONGO	EQU	0x00000010		; ongoing of precharge all count of precharge all (1)								

        EXPORT	InitSdram


InitSdram	FUNCTION


;
; Set SDRAM External Bank Register	
;
        LDR     r1, =SDRAM_EXT_BANK
        LDR     r0, =SDRAM_EXT_BANK_INIT
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

;
; CK and CKB	
;
        LDR     r1, =SDRAM_CONFIG
        LDR     r0, =SDRAM_CK_CKB
        STR     r0, [r1]
        
;
; CKE	
;
        LDR     r1, =SDRAM_CONFIG
        LDR     r0, =SDRAM_CKE
        STR     r0, [r1] 

;
; 200 us Delay
;
	MOV	r0, #SDRAM_DELAY_200US
LABEL_SDRAM_DELAY_200US
	SUBS	r0, r0, #0x1
	BHI	LABEL_SDRAM_DELAY_200US
	
;
; Precharge All	
;
        LDR     r1, =SDRAM_CONFIG
        LDR     r0, =SDRAM_PRECHARGE_ALL0
        STR     r0, [r1]
LABEL_SDRAM_PRECHARGE_ALL0        
	LDR	r0, [r1, #0]
	TST	r0, #SDRAM_PRECHARGE_ALL_ONGO
	BNE	LABEL_SDRAM_PRECHARGE_ALL0 
	
;
; Extended Mode Register Select	
;
        LDR     r1, =SDRAM_EXT_MODE
        LDR     r0, =SDRAM_EXT_MODE_REG_SELECT
        STR     r0, [r1]          
        
;
; Extended Mode Register Set	
;
        LDR     r1, =SDRAM_CONFIG
        LDR     r0, =SDRAM_EXT_MODE_REG_SET
        STR     r0, [r1]              
LABEL_SDRAM_EXT_MODE_REG_SET        
	LDR	r0, [r1, #0]
	TST	r0, #SDRAM_MODE_REG_SET_ONGO
	BNE	LABEL_SDRAM_EXT_MODE_REG_SET  

;
; Mode Register DLL Reset Select	
;
        LDR     r1, =SDRAM_EXT_MODE
        LDR     r0, =SDRAM_MODE_REG_DLL_RESET_SELECT
        STR     r0, [r1]          
        
;
; Mode Register DLL Reset Set	
;
        LDR     r1, =SDRAM_CONFIG
        LDR     r0, =SDRAM_MODE_REG_DLL_RESET_SET
        STR     r0, [r1]              
LABEL_SDRAM_MODE_REG_DLL_RESET_SET       
	LDR	r0, [r1, #0]
	TST	r0, #SDRAM_MODE_REG_SET_ONGO
	BNE	LABEL_SDRAM_MODE_REG_DLL_RESET_SET
        
;
; Precharge All	
;
        LDR     r1, =SDRAM_CONFIG
        LDR     r0, =SDRAM_PRECHARGE_ALL1
        STR     r0, [r1]
LABEL_SDRAM_PRECHARGE_ALL1        
	LDR	r0, [r1, #0]
	TST	r0, #SDRAM_PRECHARGE_ALL_ONGO
	BNE	LABEL_SDRAM_PRECHARGE_ALL1   

;
; Auto Refresh	
;
        LDR     r1, =SDRAM_CONFIG
        LDR     r0, =SDRAM_AUTO_REFRESH
        STR     r0, [r1]
LABEL_SDRAM_AUTO_REFRESH       
	LDR	r0, [r1, #0]
	TST	r0, #SDRAM_AUTO_REFRESH_ONGO
	BNE	LABEL_SDRAM_AUTO_REFRESH 
	
;
; Mode Register Select	
;
        LDR     r1, =SDRAM_EXT_MODE
        LDR     r0, =SDRAM_MODE_REG_SELECT
        STR     r0, [r1]          
        
;
; Mode Register Set	
;
        LDR     r1, =SDRAM_CONFIG
        LDR     r0, =SDRAM_MODE_REG_SET
        STR     r0, [r1]              
LABEL_SDRAM_MODE_REG_SET        
	LDR	r0, [r1, #0]
	TST	r0, #SDRAM_MODE_REG_SET_ONGO
	BNE	LABEL_SDRAM_MODE_REG_SET 
                        
;
; 200 Clock Delay
;
	MOV	r0, #SDRAM_DELAY_200CLK
LABEL_SDRAM_DELAY_200CLK
	SUBS	r0, r0, #0x1
	BHI	LABEL_SDRAM_DELAY_200CLK
                
	BX LR
	ENDFUNC


	END                          			; mark the end of this file
