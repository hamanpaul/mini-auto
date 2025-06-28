/*

Copyright (c) 2007  Himax Technologies, Inc.

Module Name:

    cfreg.h

Abstract:

    The definitions of SD Card registers.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2007/10/20  Ven-Chin Chen   Creates

*/

#ifndef __CF_REG_H__
#define __CF_REG_H__


/*CY 0601 S*/
/* 
 * SYS_CTL0 0xD00B:0x0000
 */
#define SYS_CTL0_CF_CK_EN   0x00020000


/*CY 0601 E*/

/***********************************************
 * CF/ATA Host Controller Registers
 ***********************************************/

/*
 * 0x0000   CfIntEn       CF Interrupt Enable Register
 * 0x0004   CfIntStatus   CF Interrupt Status Register
 */
#define CF_INT_CF_DMA_COMPLETE               0x00000001
#define CF_INT_CF_ACCESS_TIMEOUT             0x00000002
#define CF_INT_DMA_MODE_ACCESS_CF_CARD_ERR   0x00000004
#define CF_INT_CF_CARD_CD1_SIGNAL            0x00000008
#define CF_INT_CF_CARD_CD2_SIGNAL            0x00000010
#define CF_INT_CF_CARD_IRQRDY_SIGNAL         0x00000020

/*
 * 0x0008   CfSelAccmode   CF Select Access Mode Register
 */
#define CF_SEL_ACCESS_DATA_WIDTH_WORD_ACCESS       0x00000000
#define CF_SEL_ACCESS_DATA_WIDTH_BYTE_ACCESS       0x00000001
#define CF_SEL_ACCESS_ACCESS_MODE_TRUE_IDE         0x00000000
#define CF_SEL_ACCESS_ACCESS_MODE_PC_CARD_MEMORY   0x00000002

/*                                                            
 * 0x000C   CfWaitTime   CF Card WAIT# Signal Time Out Register     
 */                                                           
#define CF_WAIT_TIME_CLOCK_20    0x00000000 
#define CF_WAIT_TIME_CLOCK_150   0x00000001                                               

/*                                                            
 * 0x0010   CfHostCtrlReg   CF Host Controller Register     
 */                                                           
#define CF_HOST_CONTROL_WRITE_DATA                       0x00000000 
#define CF_HOST_CONTROL_READ_DATA                        0x00000001 
#define CF_HOST_CONTROL_MEMORY_MODE_IN_DMA               0x00000004 
#define CF_HOST_CONTROL_TRUE_IDE_MODE_IN_DMA             0x00000008 
#define CF_HOST_CONTROL_ATTRIBUTE_MODE_IN_CPU            0x00000010 
#define CF_HOST_CONTROL_MEMORY_MODE_IN_CPU               0x00000020 
#define CF_HOST_CONTROL_TRUE_IDE_MODE_IN_CPU             0x00000040 
#define CF_HOST_CONTROL_TRUE_IDE_MODE_CE2_IN_CPU         0x00000080 
#define CF_HOST_CONTROL_TRUE_IDE_MODE_IN_MULTIWORD_DMA   0x00000100 

#define CF_HOST_CONTROL_WRITE_DATA_PIO                   (CF_HOST_CONTROL_WRITE_DATA |   \
                                                          CF_HOST_CONTROL_TRUE_IDE_MODE_IN_DMA)
#define CF_HOST_CONTROL_READ_DATA_PIO                    (CF_HOST_CONTROL_READ_DATA |    \
                                                          CF_HOST_CONTROL_TRUE_IDE_MODE_IN_DMA)
#define CF_HOST_CONTROL_WRITE_DATA_DMA                   (CF_HOST_CONTROL_WRITE_DATA |   \
                                                          CF_HOST_CONTROL_TRUE_IDE_MODE_IN_MULTIWORD_DMA)
#define CF_HOST_CONTROL_READ_DATA_DMA                    (CF_HOST_CONTROL_READ_DATA |    \
                                                          CF_HOST_CONTROL_TRUE_IDE_MODE_IN_MULTIWORD_DMA)                                          


/*                                                            
 * 0x0014   CfSectorSize   CF Sector Size Register     
 */                                                           
#define CF_SECTOR_SIZE_128_BYTES                 0x00000000 
#define CF_SECTOR_SIZE_256_BYTES                 0x00000001 
#define CF_SECTOR_SIZE_512_BYTES                 0x00000002 
#define CF_SECTOR_SIZE_01_KBYTES                 0x00000003 
#define CF_SECTOR_SIZE_02_KBYTES                 0x00000004 
#define CF_SECTOR_SIZE_04_KBYTES                 0x00000005 
#define CF_SECTOR_SIZE_08_KBYTES                 0x00000006 
#define CF_SECTOR_SIZE_16_KBYTES                 0x00000007 

/*                                                            
 * 0x0018   CfAddr   CF Address Register   
 * Please refer to p105/p121 of CF Spec.  
 */                                                           
#define CF_ADDRESS_PIO_RD_DATA        0x00000000 
#define CF_ADDRESS_PIO_WR_DATA        0x00000000 
#define CF_ADDRESS_DMA_RD_DATA        0x00000000   /* Don't care */
#define CF_ADDRESS_DMA_WR_DATA        0x00000000   /* Don't care */ 
#define CF_ADDRESS_ERROR              0x00000001 
#define CF_ADDRESS_FEATURES           0x00000001 
#define CF_ADDRESS_SECTOR_COUNT       0x00000002 
#define CF_ADDRESS_SECTOR_NO          0x00000003 
#define CF_ADDRESS_CYLINDER_LOW       0x00000004 
#define CF_ADDRESS_CYLINDER_HIGH      0x00000005 
#define CF_ADDRESS_SELECT_CARD_HEAD   0x00000006 
#define CF_ADDRESS_STATUS             0x00000007 
#define CF_ADDRESS_COMMAND            0x00000007 
#define CF_ADDRESS_ALT_STATUS         0x00000006
#define CF_ADDRESS_DEVICE_CONTROL     0x00000006

/*                                                            
 * 0x0024   CfAttmodeReadTime   CF Attribute Mode Read Timing Control Register   
 */                                                           

/*                                                            
 * 0x0028   CfCommodeReadTime   CF Common Memory Mode Read Timing Control Register   
 */                                                           

/*                                                            
 * 0x002c   CfIdemodeReadTime   CF True-IDE Mode Timing Read Control Register   
 */ 

/*                                                            
 * 0x0030   CfErrorData   CF Error Data Register  
 */ 

/*                                                            
 * 0x0034   CfStatusChkloopcnt   CF Status Check Loop Count Register   
 */ 

/*                                                            
 * 0x0038   CfCardPinlevel   CF Card Pin Level Register   
 * 0: Low
 * 1: High
 */ 
#define CF_CARD_PIN_LEVEL_CD1          0x00000001
#define CF_CARD_PIN_LEVEL_CD2          0x00000002
#define CF_CARD_PIN_LEVEL_IRQRDY       0x00000004
#define CF_CARD_PIN_LEVEL_IORDY_WAIT   0x00000008

/*                                                            
 * 0x003c   CfCardTrigen   CF Card Trigger Enable Register  
 */ 
#define CF_CARD_TRIGGER_ENABLE_CD1_RISING_EDGE_TRIGGER       0x00000001
#define CF_CARD_TRIGGER_ENABLE_CD2_RISING_EDGE_TRIGGER       0x00000002
#define CF_CARD_TRIGGER_ENABLE_IRQRDY_RISING_EDGE_TRIGGER    0x00000004
#define CF_CARD_TRIGGER_ENABLE_CD1_FALLING_EDGE_TRIGGER      0x00000010
#define CF_CARD_TRIGGER_ENABLE_CD2_FALLING_EDGE_TRIGGER      0x00000020
#define CF_CARD_TRIGGER_ENABLE_IRQRDY_FALLING_EDGE_TRIGGER   0x00000040

/*                                                            
 * 0x0040   CfCardReset   CF Card Reset Register  
 */ 
#define CF_CARD_RESET_TRUE_IDE         0x00000000
#define CF_CARD_RESET_PC_CARD_MEMORY   0x00000001

/*                                                            
 * 0x0050   CfAutoCheckCardStatus   CF Card -> DMA Mode Auto Check Status Register  
 */ 
#define CF_AUTO_CHECK_CARD_STATUS_ENA    0x00000100

/*                                                            
 * 0x0058   CfAttmodeWriteTime   CF Attribute Mode Timing Write Control Register   
 */                                                           

/*                                                            
 * 0x005c   CfCommodeWriteTime   CF Common Memory Mode Write Timing Control Register   
 */                                                           

/*                                                            
 * 0x0060   CfIdemodeWriteTime   CF True-IDE Mode Timing Write Control Register   
 */ 
/***********************************************
 * CF/ATA Device Controller Registers
 ***********************************************/

/*                                                            
 * Offsets 1h   Error Register
 */ 
#define CF_CARD_DEVICE_ERROR_BBKICRC              0x00000080
#define CF_CARD_DEVICE_ERROR_UNC                  0x00000040
#define CF_CARD_DEVICE_ERROR_IDNF                 0x00000010
#define CF_CARD_DEVICE_ERROR_ABRT                 0x00000004
#define CF_CARD_DEVICE_ERROR_AMNF                 0x00000001

/*                                                            
 * Offsets 6h   Drive/Head Register
 */ 
#define CF_CARD_DEVICE_DRIVE_HEAD_LBA             0x00000040
#define CF_CARD_DEVICE_DRIVE_HEAD_DRV             0x00000010

/*                                                            
 * Offsets 7h & Eh   Status & Alternate Status Registers
 */ 
#define CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY      0x00000080
#define CF_CARD_DEVICE_STATUS_ALTERNATE_RDY       0x00000040
#define CF_CARD_DEVICE_STATUS_ALTERNATE_DWF       0x00000020
#define CF_CARD_DEVICE_STATUS_ALTERNATE_DSC       0x00000010
#define CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ       0x00000008
#define CF_CARD_DEVICE_STATUS_ALTERNATE_CORR      0x00000004
#define CF_CARD_DEVICE_STATUS_ALTERNATE_IDX       0x00000002
#define CF_CARD_DEVICE_STATUS_ALTERNATE_ERR       0x00000001
#define CF_CARD_DEVICE_STATUS_ALTERNATE_ALL       (CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY |   \
                                                   CF_CARD_DEVICE_STATUS_ALTERNATE_RDY |    \
                                                   CF_CARD_DEVICE_STATUS_ALTERNATE_DWF |    \
                                                   CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ |    \
                                                   CF_CARD_DEVICE_STATUS_ALTERNATE_ERR)



/*
 *
 */
#define OUTPW(port, value)   (*((volatile s32 *) (port)) = value)
#define INPW(port)           (*((volatile s32 *) (port)))

#define CF_LASTREGOFFSET   0x60                        /*  */
#define CF_REGNUM          (CF_LASTREGOFFSET / 4 + 1) /* The number of CF Host Controller registers. */

#endif
