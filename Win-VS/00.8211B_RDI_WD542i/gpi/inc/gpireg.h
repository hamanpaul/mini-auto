/*

Copyright (c) 2010  Himax Technologies, Inc.

Module Name:

    gpireg.h

Abstract:

    The definitions of GPI registers.

Environment:

     ARM RealView Developer Suite

Revision History:
    
    2010/08/02  Raymond Creates

*/

#ifndef __GPI_REG_H__
#define __GPI_REG_H__

/* 
 * SYS_CTL0 0xD00B:0x0000
 */
#define SYS_CTL0_GPI_CK_EN   0x01000000

#define GPIU_EN   0x00000100

/***********************************************
 * GPIU Controller Registers
 ***********************************************/

/*
 * 0x0000   GPICtrlReg    
*/

#define	GPI_START_TRIG  	0x00000001
#define	GPI_Reset 			0x00000002
#define	GPI_OUT_BE			0x00000004
#define GPI_IN_BE   		0x00000008
#define GPI_CHIP_SEL_1      0x00000010 
#define GPI_CHIP_SEL_2      0x00000020 
#define GPI_CHIP_SEL_3      0x00000040 
#define GPI_CHIP_SEL_4      0x00000080

#if (SYS_CPU_CLK_FREQ == 24000000)
#define GPI_POS_DUTY_TIM_R  0x00000100  //for read
#define GPI_POS_DUTY_TIM_W  0x00000100  //for write

#define GPI_NEG_DUTY_TIM_R    	0x00001000   
#define GPI_NEG_DUTY_TIM_W    	0x00001000   
#elif(SYS_CPU_CLK_FREQ == 32000000)
#if 0//fail, can't get ip 
#define GPI_POS_DUTY_TIM_R  0x00000000 //for read
#define GPI_POS_DUTY_TIM_W  0x00000000 //for write

#define GPI_NEG_DUTY_TIM_R  0x00001000 
#define GPI_NEG_DUTY_TIM_W  0x00001000
#endif
#if 0//fail, can't get ip 
#define GPI_POS_DUTY_TIM_R  0x00000100 //for read
#define GPI_POS_DUTY_TIM_W  0x00000100 //for write

#define GPI_NEG_DUTY_TIM_R  0x00000000 
#define GPI_NEG_DUTY_TIM_W  0x00000000
#endif
#if 0 //fail, can't get ip 
#define GPI_POS_DUTY_TIM_R  0x00000100 //for read
#define GPI_POS_DUTY_TIM_W  0x00000000 //for write

#define GPI_NEG_DUTY_TIM_R  0x00000000 
#define GPI_NEG_DUTY_TIM_W  0x00000000
#endif

#if 0 //fail, can't get ip
#define GPI_POS_DUTY_TIM_R  0x00000000 //for read
#define GPI_POS_DUTY_TIM_W  0x00000000 //for write

#define GPI_NEG_DUTY_TIM_R  0x00000000 
#define GPI_NEG_DUTY_TIM_W  0x00000000
#endif
#if 1 //dma: 3.6ms, tmp: 3.7ms, dm9000: 3.9ms
#define GPI_POS_DUTY_TIM_R  0x00000100 //for read
#define GPI_POS_DUTY_TIM_W  0x00000100 //for write

#define GPI_NEG_DUTY_TIM_R  0x00001000 
#define GPI_NEG_DUTY_TIM_W  0x00001000
#endif
#if 0 //dma: 2.8ms, tmp: 2.4ms, dm9000: 4s
#define GPI_POS_DUTY_TIM_R  0x00000600 //for read
#define GPI_POS_DUTY_TIM_W  0x00000600 //for write

#define GPI_NEG_DUTY_TIM_R  0x00006000 
#define GPI_NEG_DUTY_TIM_W  0x00006000
#endif

#elif(SYS_CPU_CLK_FREQ == 72000000)
#define GPI_POS_DUTY_TIM_R  0x00000300 //for read
#define GPI_POS_DUTY_TIM_W  0x00000200 //for write

#define GPI_NEG_DUTY_TIM_R  0x00002000 
#define GPI_NEG_DUTY_TIM_W  0x00002000

#elif(SYS_CPU_CLK_FREQ == 96000000)
#define GPI_POS_DUTY_TIM_R  0x00000400 //for read
#define GPI_POS_DUTY_TIM_W  0x00000200 //for write

#define GPI_NEG_DUTY_TIM_R  0x00002000 
#define GPI_NEG_DUTY_TIM_W  0x00002000

#elif(SYS_CPU_CLK_FREQ == 108000000)
#define GPI_POS_DUTY_TIM_R  0x00000600 //for read
#define GPI_POS_DUTY_TIM_W  0x00000400 //for write

#define GPI_NEG_DUTY_TIM_R  0x00004000 
#define GPI_NEG_DUTY_TIM_W  0x00004000

#elif(SYS_CPU_CLK_FREQ == 160000000)
#define GPI_POS_DUTY_TIM_R  0x00000400 //for read
#define GPI_POS_DUTY_TIM_W  0x00000200 //for write

#define GPI_NEG_DUTY_TIM_R  0x00002000 
#define GPI_NEG_DUTY_TIM_W  0x00002000

#elif(SYS_CPU_CLK_FREQ == 192000000)
#if 1 
#define GPI_POS_DUTY_TIM_R  0x00000600 //for read
#define GPI_POS_DUTY_TIM_W  0x00000400 //for write

#define GPI_NEG_DUTY_TIM_R  0x00004000 
#define GPI_NEG_DUTY_TIM_W  0x00004000
#endif

#endif 
#define GPI_CHIP_CMD_IOW_1      0x00010000 
#define GPI_CHIP_CMD_IOW_2      0x00020000 
#define GPI_CHIP_CMD_IOW_3      0x00040000 
#define GPI_CHIP_CMD_IOW_4      0x00080000
#define GPI_CHIP_CMD_IOR        0x00100000
#define GPI_DMA_MODE            0x01000000
#define GPI_BURST4_MODE         0x02000000

/*
 * 0x0004  GPILength GPI Data Length Register
 */
#define GPI_OUT_LEN              0x0000FFFF
#define GPI_IN_LEN               0xFFFF0000
/*
 * 0x0008  GPICount GPI Data Counter Register
 */
#define GPI_OUT_COUNT              0x0000FFFF
#define GPI_IN_COUNT               0xFFFF0000

/*                                                            
 * 0x000C   GPIIntCtrl   GPI Interrupt Control Register     
 */                                                           
#define GPI_FINISH_INT    	0x00000001 
#define GPI_RX_VALID_INT   	0x00000002                                               
#define GPI_TX_REQ_INT   	0x00000004  
#define GPI_FINISH_MSK   	0x00000100  
#define GPI_RX_MSK   		0x00000200  
#define GPI_TX_MSK   		0x00000400  
#define GPI_FIFO_LEN        0x0F000000 
#define GPI_BUSY            0x10000000 
/*                                                            
 * 0x0010   GPIOutput   GPI Output Data Register     
 */                                                           
                           

/*                                                            
 * 0x0014   GPIInput   GPI Input Data Register     
 */                                                           


#endif
