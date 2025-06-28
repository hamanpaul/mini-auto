/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	hiureg.h

Abstract:

   	The declarations of host interface unit.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __HIU_REG_H__
#define __HIU_REG_H__

#include "general.h"

typedef struct _HIU_REG 
{

	volatile u32 HIU_CTL0;		// 0x00 HIU_CTL0 (HIU control register 0)
					//[0] 	HIU_RST 		W		Reset all the registers of HIU module. When set this register to 1, it will be set to 0, automatically.
					//[1] 	HIU_EN		R/W 	0: disable
					//					 		1:enable , only enable HIU, the data output is enabled !
					//[2] 	reserved		
					//[3] 	DMA_EN		R/W 	Using DMA channel to transfer data fifo, used when Host want to read data, enable this will trigger DMA to read data from DRAM.
					//							Note: DMAMODE must set to 1 (use DMA)
					//[4] 	FIFO_DIR		R/W 	Data FIFO driection
					//							0: The data FIFO sends data to host
					//							1: The data FIFO receives the data from host
					//[5] 	WR_RISE 	R/W 	nRW (in I80 mode) and R/W (in M68 mode) latch timing select
					//							1: falling edge latch data
					//							0: rising edge latch data
					//[6] 	ER_RISE 		R/W 	nRD (in I80 mode) and E (in M68 mode) latch timing select
					//							1: falling edge latch data
					//							0: rising edge latch data
					//[7] 	HIU_ENDIAN	R/W 	Endian setting of baseband processor
					//							0: big endian
					//							1: little endian
					//[9:8]	HIU_IF				R/W The interface mode of baseband processor
					//							2b'00: reseved (asynchronous mode) 
					//							2'b01: M68 mode
					//							2'b10: I80 mode
					//							2'b11: reserved (synchronous mode) 
					//[11:10] HIU_SGL 	R/W 	The interface signals for host interface
					//							2'b00: host interface with RS signal 
					//							2'b01: host interface with address (a3~a0) signals
					//							2'b10: host interface with both the RS signal and address signals. The operation is the same as its with RS signal 
					//							2'b11: reserved
					//[14:12] HIU_DT		R/W 	The data type of HIU interface
					//							3'b000: 1 bits data bus
					//							3'b001: 8 bits data bus
					//							3'b010: 9bits data bus
					//							3'b011: 16 bits data bus
					//							3'b100: 18 bits data bus
					//							3'b101 ~ 3'b111:Reserved
					//[15]	Reserved		
					//[16]	WR_EB_SYNC	R/W   	The data latch of HIU
					//							0: latch data use system clock
					//							1: using WR signal as a synchronous signal to latch data 
					//[21:17] WR_DLY* 	R/W 	The internal delay setting for WE signal 
					//22	HIU_IF_SEL	R/W 	Select from outer hardware pin or inner register
					//  							0: from outer pin, default
					// 							1: selection from register0 [9:8] 
					//[24:23] Reserved		
					//[29:25] EB_DLY* 	R/W 	The internal delay setting for EB signal 

	volatile u32 HIU_CTL1;		    // 0x04  	HIU_CTL1 (HIU control register 1)
					//[2:0]	P_SIZE		R/W 	The maximum data size in each packet
					//							3'b000: 256 bytes
					//							3'b001: 512 bytes
					//							3'b010: 1kbytes
					//							3'b011: 2 kbytes
					//							3'b100: 4 k bytes
					//							3'b101 ~ 3'b110: Reserved
					//							3'b111: no boundary
					//[5:3]	Reserved
					//[6] 	CPU_WDF 	W		1: CPU write data fifo finish.	When at CPU mode, if cpu want to write to datafifo and transfer to HOST, after write data finish, cpu need to write 1 this bit. So that HOST will be notified data is ready for read. After Host finish reading, this bit will be clear to 0.
					//[7] 	CPU_WDF2	W		1: CPU write data fifo finish.	When at CPU mode, if cpu want to write to datafifo and transfer to HOST, after write data finish, cpu need to write 1 this bit.
					//[11:8]	CMD_SIZE	R/W 	The size of command code
					//[15:12] CMD_CNT*	R		The current byte counts of input command code
					//[23:16] CLK_DIV	R/W 	HIU clock speed. The frequency of HIU should be twice than the transmitted data rate in host end. <Notes> The CLK_DIV = 0 value is prohibited.
					//[24]	DMAMODE 	R/W 	1: use DMA to read/write data fifo
					//							0: use CPU to read/write data fifo
					//[26]	MBIST		R/W 	1: start internal memory BIST, should polling Status for BIST end and report
	volatile u32 INT_HIU; 	        // 0x08	 	INT_HIU (Interrupt enable bits for HIU) 
					//[2:0]	Reserved		
					//[3] 	INT_DN		R/W 	Interrupt enable bit when data transfer/reveiver is complete
					//[4] 	INT_CMD 	R/W 	Interrupt enable bit when received command is complete
					//[31:5]	Reserved		

	volatile u32 STATUS_HIU; 	        // 0x0C	 	INTRPT_HIU (Interrupt status report for HIU) 
					//[0] 	CMD_RDY 	R		The status report for command fifo, if host is writing cmd into this fifo, the flag will be busy until host finish writting.
					//							0: command fifo is busy now
					//							1: command fifo is ready.
					//[1] 	reserved		
					//[2] 	RPT_CMD 	R		Interrupt status report when data transfer/reveiver is complete
					//[3] 	RPT_DN		R		Interrupt status report when an OP command is received complete
					//[5:4]	reserved		
					//[6] 	RPT_RE		W/R 	DataFIFO readable.	1: readable  0: not readable
					//[7] 	RPT_WE		W/R 	DataFIFO writable.
					//[8] 	H_WDF1		W/R 	Used at CPU mode only, CPU need to write 1 to ack FIFO readout finish.
					//							1: Host write DataFIFO1 full
					//							0: DataFIFO1 not full,
					//[9] 	H_WDF2		W/R 	Used at CPU mode only, CPU need to write 1 to ack FIFO readout finish.
					//							1: Host write DataFIFO2 full
					//							0: DataFIFO2 not full,
					//[10]	MBIST_END2	R		End of testing sram2
					//							1: BIST testing finish,
					//[11]	MBIST_END1	R		End of testing sram1
					//							1: BIST testing finish,
					//[12]	MBIST_TEST2 R		Testflag 
					//							1: SRAM2 testing fail
					//							0: SRAM2 testing ok
					//[13]	MBIST_TEST1 R		Testflag
					//							1: SRAM1 testing fail
					//							0: SRAM1 testing ok

	volatile u32 CMD_HIU[4]; 	// 0x10	 	CMD0_HIU (HIU Command code)
/*
	volatile u32 CMD1_HIU; 	// 0x14	 	CMD1_HIU (HIU Command code)

	volatile u32 CMD2_HIU; 	// 0x18	 	CMD2_HIU (HIU Command code)

	volatile u32 CMD3_HIU; 	// 0x1C	 	CMD3_HIU (HIU Command code)
*/
	volatile u32 DATA_HIU; 	// 0x20  		DATA_HIU (HIU Data FIFO)

	volatile u32 DRAM_SADDR; // 0x24  		DRAM_SADDR (DRAM starting address) 

	volatile u32 LatchData; 	// 0x28  		Host Latch Data 

	volatile u32 RxDataCount;// 0x2C  		Receive Data count

	volatile u32 RxCmdCount; // 0x30  		Receive Cmd count

	volatile u32 DataPin; 	// 0x34 		Host input Data Pin

	volatile u32 Bypass; 	// 0x38		ByPass setting
					//[2:0]	Bypass_CTL	W		2'b11:	bridge
					//							2'b01:	bypass LCM1
					//							2'b10:	bypass LCM2
					//							2"b00:	H/W bypass - depends on {HA[0], HCS2, HCS1 } =
					//								3'b000: bypass LCM1
					//								3'b100: bypass LCM2
					//								3'bx10: Host module active
					//[3] 	Bypass_SER	R		Bypass serial
					//							0: disable
					//							1: bypass serial interface
					//[6:4]	Bypass_DFT	R		3'b000: 
					//							3'b000: 
					//							3'b000: 
					//							3'b000: 
					//							3'b000: 

} HIU_REG, *PHIU_REG;





// 0xc0080000
#define HIU_RST			0x00000001
#define HIU_OUTPUT_EN	0x00000002
//#define HIU_BYPASS		0x00000004 // not using
#define HIU_DMA_EN		0x00000008   // Host want to read data.

#define HIU_FIFO_SEND	0x00000000   //FIFO direction
#define HIU_FIFO_RECV	0x00000010   //FIFO direction

#define HIU_WR_RISE		0x00000000
#define HIU_WR_FALL		0x00000020

#define HIU_ER_RISE		0x00000000
#define HIU_ER_FALL		0x00000040

#define HIU_ENDIAN_BIG	0x00000000
#define HIU_ENDIAN_LIT	0x00000080

#define HIU_IF_M68		0x00000100
#define HIU_IF_I80		0x00000200


	// The interface signals for host interface
#define HIU_SGL_RS		0x00000000  // with RS signal
#define HIU_SGL_ADR		0x00000400  // with ADDRESS signal
#define HIU_SGL_RSADR	0x00000800  // with RS and ADDRESS

	// Data TYPE OF Hiu interface
#define HIU_DT_1b		0x00000000 // 8 bits 
#define HIU_DT_8b		0x00001000 // 8 bits
#define HIU_DT_9b		0x00002000 // 8 bits
#define HIU_DT_16b		0x00003000 // 16 bits
#define HIU_DT_18b		0x00004000 // 16 bits

	// The data latch of HIU
#define HIU_LATCH_SYSCLK	0x00000000
#define HIU_LATCH_WR		0x00010000

// 0xc0080004

#define HIU_P_SIZE_256	0x00000000 //maximum data size is 256 bytes
#define HIU_P_SIZE_512	0x00000001 //maximum data size is 512 bytes
#define HIU_P_SIZE_1K	0x00000002 //maximum data size is 1K bytes
#define HIU_P_SIZE_2K	0x00000003 //maximum data size is 2K bytes
#define HIU_P_SIZE_4K	0x00000004 //maximum data size is 4K bytes
#define HIU_P_SIZE_NOB	0x00000007 //maximum data size is no boundary

#define HIU_CMD_SIZE_MASK	0x00000F00 //maximum data size is no boundary
#define HIU_CMD_SIZE_SHIFT	8

#define HIU_DMAMODE_CPU	0x00000000 // 	use CPU to RW data fifo
#define HIU_DMAMODE_DMA	0x01000000 // 	use DMA to RW data fifo

//0xc0080008
#define HIU_INT_CMD_MASK	0x00000008
#define HIU_INT_DATA_MASK	0x00000010

//0xc008000C
#define HIU_INT_CMD_STS		0x00000004
#define HIU_INT_DATA_STS	0x00000008

#endif

