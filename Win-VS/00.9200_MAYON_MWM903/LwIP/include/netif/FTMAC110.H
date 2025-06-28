/**************************************************************************
   Copyright  Faraday Technology Corp 2011.  All rights reserved.     
  -------------------------------------------------------------------------
   Name: FTMAC110.h                                                         
   Description: FTMAC110 Non-OS firmware header file                                            
   Author: Jason Lin                                                      
   Date: 2011/08/08                                                        
   Version:1.0
***************************************************************************/

#ifndef __FTMAC110_H__
#define __FTMAC110_H__                            


#include <string.h>

#include "general.h"
#include "board.h"
#include "gpioapi.h"  
#include "sysapi.h"  
//#define FTMACBase       0xA0000000  
#if (CHIP_OPTION == CHIP_A1018A)
#define FTMAC110_BASE FTMACBase2
#else
#define FTMAC110_BASE FTMACBase
#endif    
#define IRQ_FTMAC110  28
               
typedef u32 UINT32;
typedef	u16	UINT16;
typedef	u8	UINT8;
typedef s32 INT32;
typedef	s16	INT16;
typedef	s8	INT8;

#define RX_QUEUE_ENTRIES                64
#define TX_QUEUE_ENTRIES                64
#define MAX_PKT_SIZE                    1518
#define RX_BUF_SIZE                     2044





/*
 * Interrupt status register & interrupt mask register (00h, 04h)
 */
#define FTMAC110_INT_RPKT_FINISH         (1 << 0)
#define FTMAC110_INT_NORXBUF             (1 << 1)
#define FTMAC110_INT_XPKT_FINISH         (1 << 2)
#define FTMAC110_INT_NOTXBUF             (1 << 3)

#define FTMAC110_INT_XPKT_OK             (1 << 4)
#define FTMAC110_INT_XPKT_LOST           (1 << 5)
#define FTMAC110_INT_RPKT_SAV            (1 << 6)
#define FTMAC110_INT_RPKT_LOST           (1 << 7)

#define FTMAC110_INT_AHB_ERR             (1 << 8)
#define FTMAC110_INT_PHYSTS_CHG          (1 << 9)

/*
 * Interrupt timer control register (28h)
 */
#define FTMAC110_ITC_RXINT_CNT(x)       (((x) & 0xf) << 0)
#define FTMAC110_ITC_RXINT_THR(x)       (((x) & 0x7) << 4)
#define FTMAC110_ITC_RXINT_TIME_SEL     (1 << 7)
#define FTMAC110_ITC_TXINT_CNT(x)       (((x) & 0xf) << 8)
#define FTMAC110_ITC_TXINT_THR(x)       (((x) & 0x7) << 12)
#define FTMAC110_ITC_TXINT_TIME_SEL     (1 << 15)

#define FTMAC110_ITC_DEFAULT            FTMAC110_ITC_RXINT_THR(1) | FTMAC110_ITC_TXINT_THR(1)

/*
 * Automatic polling timer control register (2Ch)
 */
#define FTMAC110_APTC_RXPOLL_CNT(x)     (((x) & 0xf) << 0)
#define FTMAC110_APTC_RXPOLL_TIME_SEL   (1 << 4)
#define FTMAC110_APTC_TXPOLL_CNT(x)     (((x) & 0xf) << 8)
#define FTMAC110_APTC_TXPOLL_TIME_SEL   (1 << 12)

#define FTMAC110_APTC_DEFAULT FTMAC110_APTC_RXPOLL_CNT(1)

/*
 * DMA burst length and arbitration control register (30h)
 */
#define FTMAC110_DBLAC_INCR4_EN         (1 << 0)
#define FTMAC110_DBLAC_INCR8_EN         (1 << 1)
#define FTMAC110_DBLAC_INCR16_EN        (1 << 2)
#define FTMAC110_DBLAC_RXFIFO_LTHR(x)   (((x) & 0x7) << 3)
#define FTMAC110_DBLAC_RXFIFO_HTHR(x)   (((x) & 0x7) << 6)
#define FTMAC110_DBLAC_RX_THR_EN        (1 << 9)

#define FTMAC110_DBLAC_DEFAULT          FTMAC110_DBLAC_RXFIFO_LTHR(2) | \
                                        FTMAC110_DBLAC_RXFIFO_HTHR(6) | \
				                        FTMAC110_DBLAC_RX_THR_EN

/*
 * MAC control register (88h)
 */
#define FTMAC110_MACCR_XDMA_EN          (1 << 0)
#define FTMAC110_MACCR_RDMA_EN          (1 << 1)
#define FTMAC110_MACCR_SW_RST           (1 << 2)
#define FTMAC110_MACCR_LOOP_EN          (1 << 3)
#define FTMAC110_MACCR_CRC_DIS          (1 << 4)
#define FTMAC110_MACCR_XMT_EN           (1 << 5)
#define FTMAC110_MACCR_ENRX_IN_HALFTX   (1 << 6)
#define FTMAC110_MACCR_RCV_EN           (1 << 8)
#define FTMAC110_MACCR_HT_MULTI_EN      (1 << 9)
#define FTMAC110_MACCR_RX_RUNT          (1 << 10)
#define FTMAC110_MACCR_RX_FTL           (1 << 11)
#define FTMAC110_MACCR_RCV_ALL          (1 << 12)
#define FTMAC110_MACCR_CRC_APD          (1 << 14)
#define FTMAC110_MACCR_FULLDUP          (1 << 15)
#define FTMAC110_MACCR_RX_MULTIPKT      (1 << 16)
#define FTMAC110_MACCR_RX_BROADPKT      (1 << 17)
#define FTMAC110_MACCR_MDC_SEL          (1 << 18)
#define FTMAC110_MACCR_ZERO_COPY        (1 << 19)

#define FTMAC110_MACCR_DEFAULT (FTMAC110_MACCR_XDMA_EN | FTMAC110_MACCR_RDMA_EN | \
								FTMAC110_MACCR_CRC_APD | FTMAC110_MACCR_XMT_EN  | \
								FTMAC110_MACCR_RCV_EN  | FTMAC110_MACCR_RX_BROADPKT)

/*
 * PHY control register (90h)
 */
#define FTMAC110_PHYCR_MIIRDATA         0xffff
#define FTMAC110_PHYCR_PHYAD(x)         (((x) & 0x1f) << 16)
#define FTMAC110_PHYCR_REGAD(x)         (((x) & 0x1f) << 21)
#define FTMAC110_PHYCR_MIIRD            (1 << 26)
#define FTMAC110_PHYCR_MIIWR            (1 << 27)

/*
 * PHY write data register (94h)
 */
#define FTMAC110_PHYWDATA_MIIWDATA(x)   ((x) & 0xffff)

/*
 * Flow control register (98h)
 */
#define FTMAC110_FCR_FC_EN              (1 << 0)
#define FTMAC110_FCR_TX_PAUSE           (1 << 1)
#define FTMAC110_FCR_FC_THREN           (1 << 2)
#define FTMAC110_FCR_TXPAUSED           (1 << 3)
#define FTMAC110_FCR_RX_PAUSE           (1 << 4)
#define FTMAC110_FCR_FC_LOW(x)          (((x) & 0xf) << 8)
#define FTMAC110_FCR_FC_HIGH(x)         (((x) & 0xf) << 12)
#define FTMAC110_FCR_FC_PAUSE_TIME(x)   (((x) & 0xffff) << 16)

/*
 * Back pressure register (9Ch)
 */
#define FTMAC110_BPR_EN                 (1 << 0)
#define FTMAC110_BPR_MODE               (1 << 1)
#define FTMAC110_BKJAM_LEN(x)           (((x) & 0xf) << 4)
#define FTMAC110_BK_LOW(x)              (((x) & 0xf) << 8)

/*
 * Wake-On-LAN control register (A0h)
 */
#define FTMAC110_WOLCR_LINKCHG0_EN      (1 << 0)
#define FTMAC110_WOLCR_LINKCHG1_EN      (1 << 1)
#define FTMAC110_WOLCR_MAGICPKT_EN      (1 << 2)
#define FTMAC110_WOLCR_WAKEUP1_EN       (1 << 3)
#define FTMAC110_WOLCR_WAKEUP2_EN       (1 << 4)
#define FTMAC110_WOLCR_WAKEUP3_EN       (1 << 5)
#define FTMAC110_WOLCR_WAKEUP4_EN       (1 << 6)
#define FTMAC110_WOLCR_POWER_STATE(x)   (((x) & 0x3) << 14)
#define FTMAC110_WOLCR_WAKEUP_SEL(x)    (((x) & 0x3) << 16)
#define FTMAC110_WOLCR_WOL_TYPE(x)      (((x) & 0x3) << 24)

/*
 * Wake-On-Lan status register (A4h)
 */
#define FTMAC110_WOLSR_LINKCHG0_STS     (1 << 0)
#define FTMAC110_WOLSR_LINKCHG1_STS     (1 << 1)
#define FTMAC110_WOLSR_MAGICPKT_STS     (1 << 2)
#define FTMAC110_WOLSR_WAKEUP1_STS      (1 << 3)
#define FTMAC110_WOLSR_WAKEUP2_STS      (1 << 4)
#define FTMAC110_WOLSR_WAKEUP3_STS      (1 << 5)
#define FTMAC110_WOLSR_WAKEUP4_STS      (1 << 6)

/*
 * DMA/FIFO state register (C8h)
 */
#define FTMAC110_RXDMA1_SM(x)           (((x) & 0xf) << 0)
#define FTMAC110_RXDMA2_SM(x)           (((x) & 0x7) << 4)
#define FTMAC110_TXDMA1_SM(x)           (((x) & 0xf) << 8)
#define FTMAC110_TXDMA2_SM(x)           (((x) & 0x7) << 12)
#define FTMAC110_RXFIFO_EMPTY           (1 << 26)
#define FTMAC110_TXFIFO_EMPTY           (1 << 27)
#define FTMAC110_DARB_RXGNT             (1 << 28)
#define FTMAC110_DARB_TXGNT             (1 << 29)
#define FTMAC110_RXD_REQ                (1 << 30)
#define FTMAC110_TXD_REQ                (1 << 31)

/*
 * Transmit descriptor, aligned to 16 bytes
 */
typedef struct _ftmac110_txdes {
        unsigned int    txdes0;
        unsigned int    txdes1;
        unsigned int    txdes2; /* TXBUF_BADR */
        unsigned int    txdes3; /* not used by HW */
}ftmac110_txdes;

#define FTMAC110_TXDES0_TXPKT_LATECOL   (1 << 0)
#define FTMAC110_TXDES0_TXPKT_EXSCOL    (1 << 1)
#define FTMAC110_TXDES0_TXDMA_OWN       (1 << 31)

#define FTMAC110_TXDES1_TXBUF_SIZE(x)   ((x) & 0x7ff)
#define FTMAC110_TXDES1_LTS             (1 << 27)
#define FTMAC110_TXDES1_FTS             (1 << 28)
#define FTMAC110_TXDES1_TX2FIC          (1 << 29)
#define FTMAC110_TXDES1_TXIC            (1 << 30)
#define FTMAC110_TXDES1_EDOTR           (1 << 31)

/*
 * Receive descriptor, aligned to 16 bytes
 */
typedef struct _ftmac110_rxdes {
        unsigned int    rxdes0;
        unsigned int    rxdes1;
        unsigned int    rxdes2; /* RXBUF_BADR */
        unsigned int    rxdes3; /* not used by HW */
}ftmac110_rxdes;

#define FTMAC110_RXDES0_RFL             0x7ff
#define FTMAC110_RXDES0_MULTICAST       (1 << 16)
#define FTMAC110_RXDES0_BROADCAST       (1 << 17)
#define FTMAC110_RXDES0_RX_ERR          (1 << 18)
#define FTMAC110_RXDES0_CRC_ERR         (1 << 19)
#define FTMAC110_RXDES0_FTL             (1 << 20)
#define FTMAC110_RXDES0_RUNT            (1 << 21)
#define FTMAC110_RXDES0_RX_ODD_NB       (1 << 22)
#define FTMAC110_RXDES0_LRS             (1 << 28)
#define FTMAC110_RXDES0_FRS             (1 << 29)
#define FTMAC110_RXDES0_RXDMA_OWN       (1 << 31)

#define FTMAC110_RXDES1_RXBUF_SIZE(x)   ((x) & 0x7ff)
#define FTMAC110_RXDES1_EDORR           (1 << 31)

/* 
 * multiple descriptor structure 
 */
typedef struct _FTMAC110_multides{
	UINT32 phy_tx_addr;
	UINT32 size;
}FTMAC110_multides;


typedef struct _FTMAC110_Device FTMAC110_Device;

/* 
 * private data of ftmac110 device, not be used by programmers 
 */
typedef struct _FTMAC110_Control{
	ftmac110_txdes *txdes_top;
	ftmac110_txdes *txdes_cur;
	ftmac110_txdes *txdes_end;
	UINT32 txdes_num;
	ftmac110_rxdes *rxdes_top;
	ftmac110_rxdes *rxdes_cur;
	ftmac110_rxdes *rxdes_end;
	UINT32 rxdes_num;
	UINT32 base;
	UINT16 phyad;
	void (*rx_isr);
	UINT32 rxbuf_size;
	UINT32 wakeup_frame_mask[4];
}FTMAC110_Control;


/**********************************************************************************************************
 * public function calls
 **********************************************************************************************************/


extern unsigned char ftmac110_pause(void);
extern void ftmac110_delaysecs(UINT32 i);
extern int ftmac110_addrcmp(const UINT8 *addr1, const UINT8 *addr2);
extern UINT32 ftmac110_fpga_version(void);
extern void ftmac110_reset_mac(void);
extern void ftmac110_reset_phy(void);

/**********************************************************************************************************
 * private function calls
 **********************************************************************************************************/
/*
 * ftmac110 device interface implementation
 */
 void ftmac110_open(void);
 void ftmac110_open_maccr(UINT32 maccr);
 void ftmac110_open_burst(UINT32 burst);
 void ftmac110_open_rx_buf_size(UINT32 rx_buf_size);
 void ftmac110_open_aptc(UINT32 aptc);
 void ftmac110_open_itc(UINT32 itc);
 void ftmac110_close(void);
void ftmac110_xmit(UINT32 phy_tx_addr, UINT32 size);
 void ftmac110_xmit_multi(const UINT32 number, FTMAC110_multides txdeses[]);
 void ftmac110_half_dup_10(void);
 void ftmac110_half_dup_100(void);
 void ftmac110_full_dup_10(void);
 void ftmac110_full_dup_100(void);
 UINT8 ftmac110_set_multicast_address(UINT8 *mac_addr);
 UINT8 *ftmac110_get_mac_addr(void);
 void ftmac110_rx_manual(void);
 void ftmac110_enter_power_down(UINT32 power_down_setting, UINT8 *wakeup_frame, UINT32 wakeup_frame_size);
 void ftmac110_exit_power_down(void);
 void ftmac110_set_wakeup_mask(UINT32 mask_value, UINT32 mask_index);
 void ftmac110_set_flow_control(UINT16 pause_time, UINT8 high_threshold, UINT8 low_threshold, UINT8 fc_thr_en, UINT8 fc_en);
 void ftmac110_xmit_pause_frame(void);
 UINT32 ftmac110_wait_pause_frame(void);

/*
 * register R/W function similar to linux kernel api 
 */
 void ftmac110_open_control(UINT32 _maccr, UINT32 _burst, UINT32 _rx_buff_size, UINT32 _aptc, UINT32 _itc);
 void iowrite32(UINT32 value, UINT32 addr);
 UINT32 ioread32(UINT32 addr);

 void initial_interrupt(void);
 void ftmac110_isr(void);

 void alloc_txdes(void);
 void alloc_rxdes(void);

typedef enum{
	FULL = 0,
	HALF = 1
}DUPLEX;

typedef enum{
	_100 = 0,
	_10  = 1
}SPEED;

 void change_duplex_speed(DUPLEX duplex, SPEED speed);

/*
 * multicast releated function calls
 */
 UINT8 ftmac110_crc32_ret_6_bit(UINT8 *mac_addr);

/*
 * wake up frame related function calls
 */
 UINT32 ftmac110_wakeup_crc32(UINT8 *wakeup_frame, UINT32 size, UINT32 *mask);

/*
 * isr handler function calls
 */
 void rpkt_finish_handler(void);
 void norxbuf_handler(void);
 void xpkt_finish_handler(void);
 void xpkt_ok_handler(void);
 void xpkt_lost_handler(void);
 void rpkt_sav_handler(void);
 void rpkt_lost_handler(void);
 void ahb_error_handler(void);
 void physts_chg_handler(void);
extern void reception_isr(void);
/*
 * PHY processing function calls
 */
/*
 * read the duplex of phy, and set it to mac
 */
UINT32 phy_processing(void);
UINT16 phyread16(UINT16 regad);
void phywrite16(UINT16 value, UINT16 regad);

/* --- PHY definitions (Model: IC PLUS IP101G)------------------------------------------------*/
#define FTPHY_REG_CONTROL 		        0x00
#define FTPHY_REG_STATUS 		        0x01
#define FTPHY_REG_ID1     		        0x02
#define FTPHY_REG_ID2    		        0x03
#define FTPHY_REG_AUTO_NEG 		        0x04
#define FTPHY_REG_LINK_PART_ABILITY     0x05
#define FTPHY_REG_AUTO_NEG_EXP          0x06
#define FTPHY_REG_AUTO_NEG_NEXT         0x07
#define FTPHY_REG_AUTO_NEG_NEXT_ABILITY 0x08
#define FTPHY_REG_CONTROL_TEST1         0x10
#define FTPHY_REG_CONTROL_TEST2         0x11
#define FTPHY_REG_CONTROL_TEST3         0x12
          
#define FTPHY_REG_CR_FULL           0x0100
#define FTPHY_REG_CR_AUTO           0x1000
#define FTPHY_REG_CR_SPEED          0x2000
#define FTPHY_REG_CR_LOOPBACK       0x4000
#define FTPHY_REG_CR_RESET          0x8000
#define FTPHY_REG_CR_RESTART_AUTO   0x0200
          
#define FTPHY_REG_STATUS_LINK       0x0004
#define FTPHY_REG_STATUS_AUTO_OK    0x0020
          
#define FTPHY_REG_04_10HALF         0x0020
#define FTPHY_REG_04_10FULL         0x0060
#define FTPHY_REG_04_100HALF        0x00E0 //0x0080
#define FTPHY_REG_04_100FULL        0x01E0 //0x0180
#define FTPHY_REG_04_MASK           0x01E0
          
#define FTPHY_ADD                   0x01
#define FTPHY_DAVICOM_ADD                   0x00
#define FTPHY_10                            0x00
#define FTPHY_100                           0x01
#define FTPHY_HALH                          0x00
#define FTPHY_FULL                          0x01

//FTPHY register definition => 00h: Basic Mode Control Register (BMCR)
#define FTPHY_RESET           (1 << 15)
#define FTPHY_LOOPBACK        (1 << 14)
#define FTPHY_SPEED           (1 << 13)
#define FTPHY_AUTONEG         (1 << 12)
#define FTPHY_POWER           (1 << 11)
#define FTPHY_ISOLATE         (1 << 10)
#define FTPHY_RESTART_AUTONEG (1 << 9)
#define FTPHY_DUPLEX          (1 << 8)
#define FTPHY_COLLISION       (1 << 7)

//FTPHY register definition => 01h: Basic Mode Status Register (BMSR)
#define FTPHY_100BASE_T4        (1 << 15)
#define FTPHY_100BASE_TX_FULL   (1 << 14)
#define FTPHY_100BASE_TX_HALF   (1 << 13)
#define FTPHY_10BASE_T_FULL     (1 << 12)
#define FTPHY_10BASE_T_HALF     (1 << 11)
#define FTPHY_MF_PREAMBLE       (1 << 6)
#define FTPHY_AUTONEG_COMPLETE  (1 << 5)
#define FTPHY_REMOTE_FAULT      (1 << 4) //not supported
#define FTPHY_AUTONEG_ABILITY   (1 << 3)
#define FTPHY_LINK_STATUS       (1 << 2)
#define FTPHY_JABBER_DETECT     (1 << 1)
#define FTPHY_EXTEND_CAPABILITY (1 << 0)

//FTPHY register definition => 02h: PHY identifier register 1
#define FTPHY_OUI_MSB           0xFFFF

//FTPHY register definition => 03h: PHY identifier register 2
#define FTPHY_OUI_LSB           0xFC00
#define FTPHY_VNDR_MDL          0x03F0
#define FTPHY_MDL_REV           0x000F

//FTPHY register definition => 04h: Auto Negotiation Advertisement Register (ANAR)
#define FTPHY_NP                0x8000
#define FTPHY_ACK               0x4000
#define FTPHY_RF                0x2000 //not supported
#define FTPHY_PAUSE             0x0400 //not supported
#define FTPHY_T4                0x0200
#define FTPHY_TX_FD             0x0100
#define FTPHY_TX_HD             0x0080
#define FTPHY_10_FD             0x0040
#define FTPHY_10_HD             0x0020
#define FTPHY_SELECTOR          0x001F

//FTPHY register definition => 05h: Auto Negotiation Link Partner Ability Register (ANLPAR)
#define FTPHY_PARTNER_NP        0x8000
#define FTPHY_PARTNER_ACK       0x4000
#define FTPHY_PARTNER_RF        0x2000 //not supported
#define FTPHY_PARTNER_PAUSE     0x0400 //not supported
#define FTPHY_PARTNER_T4        0x0200
#define FTPHY_PARTNER_TX_FD     0x0100
#define FTPHY_PARTNER_TX_HD     0x0080
#define FTPHY_PARTNER_10_FD     0x0040
#define FTPHY_PARTNER_10_HD     0x0020
#define FTPHY_PARTNER_SELECTOR  0x001F

//FTPHY register definition => 06h: Auto Negotiation Expansion Register (ANER)
#define FTPHY_PDF               0x0010
#define FTPHY_LP_NP_AB          0x0008
#define FTPHY_NP_AB             0x0004
#define FTPHY_PAGE_RX           0x0002
#define FTPHY_LP_AN_AB          0x0001

//FTPHY register definition => 07h: Auto Negotiation Next Page Transmit  Register (ANNPTR)
#define FTPHY_NEXT_PAGE         0x8000
#define FTPHY_MESSAGE_PAGE      0x2000
#define FTPHY_ACK2              0x1000
#define FTPHY_TOGGLE            0x0800
#define FTPHY_UNFORMATTED       0x07FF

//FTPHY register definition => 08h: Auto Negotiation Link Partner Next Page Ability Register (ANLPNP)
#define FTPHY_PARTNER_NEXT_PAGE         0x8000
#define FTPHY_PARTNER_MESSAGE_PAGE      0x2000
#define FTPHY_PARTNER_ACK2              0x1000
#define FTPHY_PARTNER_TOGGLE            0x0800
#define FTPHY_PARTNER_UNFORMATTED       0x07FF

#endif
