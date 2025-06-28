/*

Copyright (c) 2007  Himax Technologies, Inc.

Module Name:

    sdcerr.h

Abstract:

    Error Code Definitions for CF Host driver.

Environment:

    ARM RealView Developer Suite

Revision History:
    
    2007/10/20  Ven-Chin Chen   Creates        

*/

#ifndef __CF_ERR_H__
#define __CF_ERR_H__

#define CF_ERROR_CODE_BASE                            0xFFF20000                    /* The base of CF host code error */
                                                        
/* Error codes definitions */                           
#define CF_OK                                         1                             /* The process is ok */
#define CF_Fail                                       0                             /* The process is fail */
#define CF_ERROR_NO_COMMAND_RESPONSE_INTERRUPT        (CF_ERROR_CODE_BASE | 0x01)   /* No command/response completion */
#define CF_ERROR_COMMAND_RESPONSE_TIME_OUT            (CF_ERROR_CODE_BASE | 0x02)   /* cfSemINTRQ is time-out */
#define CF_ERROR_REPONSE_RX_CRC_ERROR                 (CF_ERROR_CODE_BASE | 0x03)   /* Reponse Rx CRC error */
#define CF_ERROR_NO_CARD_IS_INSERTED                  (CF_ERROR_CODE_BASE | 0x04)   /* No SD Card is inserted */
#define CF_ERROR_CF_TIME_OUT                          (CF_ERROR_CODE_BASE | 0x05)   /* SD Card is time out */
#define CF_ERROR_DMA_CH_ERROR                         (CF_ERROR_CODE_BASE | 0x06)   /* dmaChResp[DMA_CH_SD] is channel error */
#define CF_ERROR_DMA_CH_ERROR_2                       (CF_ERROR_CODE_BASE | 0x07)   /* dmaChResp[DMA_CH_SD] is channel error - 2 */
#define CF_ERROR_DATA_RX_CRC_ERROR                    (CF_ERROR_CODE_BASE | 0x08)   /* Data RX CRC Error (sdcDatRxCrcErr) */
#define CF_ERROR_DMA_DATA_RX_ERROR                    (CF_ERROR_CODE_BASE | 0x09)   /* DMA Data RX Error (sdcDmaDatRxErr) */
#define CF_ERROR_DATA_RX_ERROR                        (CF_ERROR_CODE_BASE | 0x0A)   /* Data RX CRC Error (sdcDatRxCrcErr) or DMA Data RX Error (sdcDmaDatRxErr) */
#define CF_ERROR_NO_DAT_MUL_WRITE_TO_CF_CARD          (CF_ERROR_CODE_BASE | 0x0B)   /* No Data Multiple WRITE to SD Card */
#define CF_ERROR_DATA_WRITE_TO_CF_CARD_TIME_OUT       (CF_ERROR_CODE_BASE | 0x0C)   /* sdcSemTx_INT is time-out */
#define CF_ERROR_CRC_ERROR                            (CF_ERROR_CODE_BASE | 0x0D)   /* CRC error */
#define CF_ERROR_DMA_DATA_TX_ERROR                    (CF_ERROR_CODE_BASE | 0x0E)   /* DMA Data Tx Error (sdcDmaDatTxErr) (DMA Controller -> SD Host Controller) */
#define CF_ERROR_DATA_TX_CRC_ERROR                    (CF_ERROR_CODE_BASE | 0x0F)   /* Data Tx CRC Error (sdcDatTxErr) (SD Host Controller -> SD Card) */
#define CF_ERROR_DMA_CF_CHANNEL_TIME_OUT              (CF_ERROR_CODE_BASE | 0x10)   /* dmaSemChFin[DMA_CH_SD] is time-out */
#define CF_ERROR_MUL_TX_TIME_OUT                      (CF_ERROR_CODE_BASE | 0x11)   /* sdcSemMulTx_INT is time-out */
#define CF_ERROR_WRITE_TO_CF_TIME_OUT                 (CF_ERROR_CODE_BASE | 0x12)   /* sdcWriteToSD is not done */
#define CF_ERROR_DEVICE_IS_NOT_CHANGED_TO_READY       (CF_ERROR_CODE_BASE | 0x13)   /* CF/ATA Device is not changed from busy to ready */
#define CF_ERROR_DEVICE_IS_NOT_CHANGED_TO_READY_RW    (CF_ERROR_CODE_BASE | 0x13)   /* CF/ATA Device is not changed to ready to Read/Write Data Register */
#define CF_ERROR_WRONG_VOLTAGE_RANGE_OR_PATTERN       (CF_ERROR_CODE_BASE | 0x14)   /* Voltage range isn't supported or check pattern doesn't match */
#define CF_ERROR_WRONG_BLOCK_LENGTH                   (CF_ERROR_CODE_BASE | 0x15)   /* Blcok length isn't 512 Bytes */
#define CF_ERROR_NOT_TRASFER_STATE                    (CF_ERROR_CODE_BASE | 0x16)   /* Current state is not transfer state */
#define CF_ERROR_CARD_IS_WRITE_PROTECTED              (CF_ERROR_CODE_BASE | 0x17)   /* Card is write-protected */
#define CF_ERROR_FAIL_TO_GET_CARD_STATUS              (CF_ERROR_CODE_BASE | 0x18)   /* Fail to get card status */

#define CF_ERROR_FAIL_TO_WRITE_DATA_TO_CARD           (CF_ERROR_CODE_BASE | 0x19)   /* Fail to write data to card */
#define CF_ERROR_FAIL_TO_MUL_WRITE_DATA_TO_CARD       (CF_ERROR_CODE_BASE | 0x1A)   /* Fail to multiple write data to card */

#define CF_ERROR_DMA_CONTROLLER_READ_ERROR            (CF_ERROR_CODE_BASE | 0x1B)   /* DMA controller fails to set to read */
#define CF_ERROR_DMA_CONTROLLER_WRITE_ERROR           (CF_ERROR_CODE_BASE | 0x1C)   /* DMA controller fails to set to write */
#define CF_ERROR_DMA_FAILS                            (CF_ERROR_CODE_BASE | 0x1D)   /* DMA operation fails */

#define CF_ERROR_CARD_DONT_SUPPORT_HIGH_SPEED         (CF_ERROR_CODE_BASE | 0x1E)   /* This card doesn't support High-Speed Mode */
#define CF_ERROR_CHECK_FUNCTION_ERROR                 (CF_ERROR_CODE_BASE | 0x1F)   /* Check function with error */

#define CF_ERROR_CARD_IDENTIFICATION_ERROR            (CF_ERROR_CODE_BASE | 0x20)   /* SecurityDisk / MultiMedia Card identification error */
#define CF_ERROR_UNDEF_COMMAND                        (CF_ERROR_CODE_BASE | 0x21)   /* Undefine command */
#define CF_ERROR_INVALID_SECTOR_COUNT                 (CF_ERROR_CODE_BASE | 0x22)   /* Invalid Sector Count */
#define CF_ERROR_UNDEF                                (CF_ERROR_CODE_BASE | 0x23)   /* Undefine error */
#define CF_ERROR_OCCURS                               (CF_ERROR_CODE_BASE | 0x24)   /* Error occurs */

#endif
