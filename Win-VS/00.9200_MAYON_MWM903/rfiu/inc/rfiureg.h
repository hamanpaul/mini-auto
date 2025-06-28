/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	rfiureg.h

Abstract:

   	The declarations of RF Interface Unit registers.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2010/10/21	Lucian Yuan	Create	

*/


/*RFU_CNTL_0*/

#define RFI_TRIG             0x00000008

#define RFI_ENA              0x00000002
#define RFI_DISA             0x00000000

#define RFI_MODE_TX          0x00000001
#define RFI_MODE_RX          0x00000000

#define RFI_TYPE_CMD         0x00000000
#define RFI_TYPE_DATA        0x00000004

#define RFI_VITBI_EN         0x00000010
#define RFI_VITBI_DISA       0x00000000

#define RFI_DATA_DECI_INTE   0x00000020
#define RFI_DATA_DECI_CEN    0x00000000

#define RFI_VITBI_CR1_2      0x00000000
#define RFI_VITBI_CR2_3      0x00000040
#define RFI_VITBI_CR3_4      0x00000080
#define RFI_VITBI_CR4_5      0x000000c0

#define RFI_RS_T2            0x00000000
#define RFI_RS_T4            0x00000100
#define RFI_RS_T8            0x00000200
#define RFI_RS_T12           0x00000300

#define RFI_SYNCWD_LEN_16    0x00000000
#define RFI_SYNCWD_LEN_8     0x00000400
#define RFI_SYNCWD_LEN_NONE  0x00000800
#define RFI_SYNCWD_LEN_32    0x00000c00

#define RFI_CUSTOMER_ID_EXT_EN  0x00001000
#define RFI_SUPER_BURST_EN      0x00002000

#define RFI_BYTEINTX_DISA       0x00004000
#define RFI_BYTEINTX_EN         0x00000000

#define RFI_TRAIL_DISA          0x00008000
#define RFI_TRAIL_EN            0x00000000

#define RFI_DUMMYDATA_NUM_SHFT      16
#define RFI_PREAMBLE_NUM_SHFT       24
#define RFI_DUMMYPREAMBLE_NUM_SHFT  28

#define RFI_RESET            0x80000000


/*RFU_CNTL_1*/
#define RFI_TXCLK_DIV_4             4
#define RFI_TXCLK_DIV_5             5 
#define RFI_TXCLK_DIV_6             6
#define RFI_TXCLK_DIV_8             8
#define RFI_TXCLK_DIV_9             9
#define RFI_TXCLK_DIV_10            10
#define RFI_TXCLK_DIV_12            12
#define RFI_TXCLK_DIV_16            16
#define RFI_TXCLK_DIV_32            32 

#define RFI_TXCLK_DIV_SHFT          0
#define RFI_USER_DATA_H_SHFT        16

#define RFI_USER_DATA_H_MASK        0x1f

#define RFI_USER_DATA_L_MASK        0x0ffff


#define RFI_SEQU_PKT_ENA            0x00000100
#define RFI_NEWPACKETSYNC_ENA       0x00000200

/*RFU_INT_EN*/
#define RFI_INTCMPL_TX_1_MASK  0x00000001
#define RFI_INTCMPL_RX_1_MASK  0x00000002

#define RFI_INTCMPL_TX_2_MASK  0x00000004
#define RFI_INTCMPL_RX_2_MASK  0x00000008

#define RFI_INTCMPL_TX_3_MASK  0x00000010
#define RFI_INTCMPL_RX_3_MASK  0x00000020

#define RFI_INTCMPL_TX_4_MASK  0x00000040
#define RFI_INTCMPL_RX_4_MASK  0x00000080

/*RFU_MAC_ID*/
#define RFI_USRDATA_L_SHFT        0
#define RFI_SYNCWORD_NUM_SHFT  16

/*RFU_TxClockConfig*/

/*RFU_RxClockAdjust*/

/*RFU_DclkConfig*/

/*RFU_PacketCNTL*/
#define RFU_PKT_BURST_NUM_SHFT    0
#define RFU_CUSTOMER_ID_SHFT      8

#define RFU_CUSTOMER_ID_MASK      0x1fff

/*RFU_PKT_GRP_0_ADDR*/

/*RFU_Packet_Group_0_Addr*/

/*RFU_RxTimingFineTune*/

/* SYS_PIN_MUX_SEL */
#define DEBUG_RFI13_EN         0x00000090
#define LOWER20BITs            0x00000000

#define PACKAGE_216PIN         0x00000000
#define PACKAGE_128PIN         0x00000100

