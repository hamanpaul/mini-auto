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
;	2011/11/10	Lucian Yuan	Create	
;


	INCLUDE	.\sysoptdef.inc
	
;------------選擇BOOT source: RAM(via ICE/boot-loader), ROM(EEPROM/NOR/ROMTER)------------------
; BOOT_SRC_OPT
;	BOOT_SRC_RAM - From RAM. 
;	BOOT_SRC_ROM - From ROM.
;	BOOT_SRC_MASKROM - From MASK ROM.
;

	GBLA	BOOT_SRC_OPT	
BOOT_SRC_OPT	SETA		BOOT_SRC_RAM


;---------------設定 Static RAM timing--------------
;CODE_DEVICE_OPT
;CODE_STR_ROMTER  - Program code is stored in Romter
;CODE_STR_EEPROM  - Program code is stored in EEPROM
;CODE_STR_RAM     - Program code is stored in RAM
;
  
CODE_DEVICE_OPT     EQU      CODE_STR_RAM

;-----------------IC 版本選擇: 9001D 要打開Cache,其他則否----------------------
; CHIP_OPT
;	CHIP_OPT_PA9001D     - PA9001D.
;	CHIP_OPT_PA9002C     - PA9002C.
;   FPGA_OPT_PA9002B     - FPGA_9002B
;	CHIP_OPT_PA9002D     - PA9002D.

;	CHIP_OPT_A1013A      - PA9003A.
;   CHIP_OPT_A1013A_FPGA - FPGA_PA9003A

;	CHIP_OPT_A1013B      - PA9003B.
;   CHIP_OPT_A1013B_FPGA - FPGA_PA9003B

;   CHIP_OPT_A1016A      - A1016
;   CHIP_OPT_A1016A_FPGA - A1016FPGA

	GBLA	CHIP_OPT
CHIP_OPT	SETA	     CHIP_OPT_A1016A
	
;------------------PLL clock output selection--------------------
; CLK_SRC_OPT

;   CLK_SRC_PLLOUT_PA9001D_48M - From PLL out 48MHz.
;   CLK_SRC_PLLOUT_PA9001C_64_8M - From PLL out 64.8MHz.
;   CLK_SRC_PLLOUT_PA9001C_162M - From PLL out 162MHz.
;   CLK_SRC_PLLOUT_PA9001C_216M - From PLL out 216MHz.

;   CLK_SRC_PLLOUT_PA9002D_192M - From PLL out 192MHz
;   CLK_SRC_PLLOUT_PA9003A_FPGA - For FPGA test

;CLK_SRC_OPT	EQU		CLK_SRC_XIN ; for FPGA

CLK_SRC_OPT	EQU		CLK_SRC_PLLOUT_PA9003A_FPGA 


;-----------------選擇DRAM Clock 反相時間點-----------------
;用於PA9002D
;由於我們的DRAM Timing 比較sensitive. 會跟顆粒與Layout 有關. 若無法開機請調整.
; 目前已知 SUNWAY 必須設定為 0
; 其他 Project 為 1 

DRAM_CLK_INV_FIRST   EQU   0


;------------------推算PLL clock -----------------
; CLK_SRC_FREQ
;	Clock source frequency
;
CLK_SRC_FREQ	EQU		32000000	; 32/48M

;------設定 Sensor MCLK -------
; CLK_MCK_DIV 
;	Sensor Master ClocK divisor
;
CLK_MCK_DIV	EQU		0x02		; 32/2=16M

;------設定 IDU clock -----
; CLK_IDU_DIV
;	Image Display Unit clock divisor
;
CLK_IDU_DIV	EQU		0x01		; 32M / 1 = 32M
;------設定 Watch dog clock ------
; CLK_WDT_DIV
;	Watch Dog Timer clock divisor
;
CLK_WDT_DIV	EQU		0x0200		; 32M /512 = 62.5K


;------設定 ADC clock----
; CLK_ADC_DIV
;	Analog-to-Digital Converter clock divisor
;
CLK_ADC_DIV	EQU		0x01		; 32M / 1 = 32M

;------JPEG VLC clock
;JPEG_VLC_DIV
CLK_JPEG_VLC_DIV EQU  0x02      ; 32M/2=16M


    END
