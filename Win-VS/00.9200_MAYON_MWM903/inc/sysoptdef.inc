;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;	sysopt.inc
;
;Abstract:
;
;   	The system option.
;
;Environment:
;
;   	ARM RealView Developer Suite
;
;Revision History:
;	
;	2006/07/18	David Tsai	Create	
;

;CY 0907

;
; BOOT_SRC_OPT
;	0 - From RAM.
;	1 - From ROM.
;	2 - From MASKROM.
;
	GBLA		BOOT_SRC_RAM
	GBLA		BOOT_SRC_ROM
	GBLA		BOOT_SRC_MASKROM
BOOT_SRC_RAM		SETA		0
BOOT_SRC_ROM		SETA		1
BOOT_SRC_MASKROM	SETA		2
	

; Twokey 061214
;
;
;CODE_DEVICE_OPT
CODE_STR_ROMTER     EQU        0x00000009
CODE_STR_EEPROM     EQU        0x00000009   
CODE_STR_RAM        EQU        0x00000001
	
;
; CLK_SRC_OPT
;	0x00 - CLK_SRC_XIN.
;	0x40 - CLK_SRC_PLLOUT_PA9001D_48M.
;	0x41 - CLK_SRC_PLLOUT_PA9001C_64_8M.
;	0x42 - CLK_SRC_PLLOUT_PA9001C_162M.
;	0x43 - CLK_SRC_PLLOUT_PA9001C_216M.
;
CLK_SRC_XIN			    EQU		0x00
CLK_SRC_PLLOUT_PA9001D_48M	    EQU		0x00
CLK_SRC_PLLOUT_PA9001C_64_8M	    EQU		0x01
CLK_SRC_PLLOUT_PA9001C_162M	    EQU		0x02
CLK_SRC_PLLOUT_PA9001C_216M	    EQU		0x03

CLK_SRC_PLLOUT_PA9002D_48M          EQU         0x3F680000
CLK_SRC_PLLOUT_PA9002D_192M         EQU         0x3F480000

CLK_SRC_PLLOUT_PA9003A_48M          EQU         0x3F680000
CLK_SRC_PLLOUT_PA9003A_96M          EQU         0x3F181000
CLK_SRC_PLLOUT_PA9003A_FPGA         EQU         0x00

;CY 1023

;
; CHIP_OPT
;	    0x00 - PA9001A.
;	    0x01 - PA9001C.
;	    0x02 - PA9002A.
;       0x03 - PA9001D
;       0x04 - PA9002C
;       0x05 - PA9002B on FPGA
;       0x06 - PA9002D


;----CPU: K310----//
CHIP_OPT_PA9001A	    EQU		        0x00
CHIP_OPT_PA9001C	    EQU		        0x01
CHIP_OPT_PA9002A	    EQU		        0x02
CHIP_OPT_PA9001D	    EQU		        0x03
CHIP_OPT_PA9002C        EQU             0x04
FPGA_OPT_PA9002B        EQU             0x05
CHIP_OPT_PA9002D        EQU             0x06

CHIP_OPT_A1016A         EQU             0x07   ;ASIC
CHIP_OPT_A1016A_FPGA    EQU             0x08   ;FPGA

CHIP_OPT_A1018A         EQU             0x09   ;ASIC
CHIP_OPT_A1018A_FPGA    EQU             0x0a   ;FPGA

;---CPU: ARM926EJ-----//
CHIP_OPT_A1013A         EQU             0x10    ;ASIC
CHIP_OPT_A1013A_FPGA    EQU             0x11    ;FPGA

CHIP_OPT_A1013B         EQU             0x12    ;ASIC
CHIP_OPT_A1013B_FPGA    EQU             0x13    ;FPGA

CHIP_OPT_A1019A         EQU             0x14    ;ASIC ARM926
CHIP_OPT_A1020A        EQU             0x15    ;ASIC
CHIP_OPT_A1021A         EQU            0x16    ;ASIC
CHIP_OPT_A1022A         EQU            0x17    ;ASIC
	END                          				; mark the end of this file
