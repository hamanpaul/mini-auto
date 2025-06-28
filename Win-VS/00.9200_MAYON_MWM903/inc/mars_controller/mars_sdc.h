

#ifndef __MARS_SDC_H__
#define __MARS_SDC_H__

#include <osapi.h>

#define SD_ERROR_CODE_BASE                            0xFFF10000                    /* The base of SD host code error */

/* Error codes definitions */
#define SD_OK                                         1                             /* The process is ok */
#define SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT        (SD_ERROR_CODE_BASE | 0x01)   /* No command/response completion */
#define SD_ERROR_COMMAND_RESPONSE_TIME_OUT            (SD_ERROR_CODE_BASE | 0x02)   /* sdcSemEvt is time-out */
#define SD_ERROR_REPONSE_RX_CRC_ERROR                 (SD_ERROR_CODE_BASE | 0x03)   /* Reponse Rx CRC error */
#define SD_ERROR_NO_CARD_IS_INSERTED                  (SD_ERROR_CODE_BASE | 0x04)   /* No SD Card is inserted */
#define SD_ERROR_SD_TIME_OUT                          (SD_ERROR_CODE_BASE | 0x05)   /* SD Card is time out */
#define SD_ERROR_DMA_CH_ERROR                         (SD_ERROR_CODE_BASE | 0x06)   /* dmaChResp[DMA_CH_SD] is channel error */
#define SD_ERROR_DMA_CH_ERROR_2                       (SD_ERROR_CODE_BASE | 0x07)   /* dmaChResp[DMA_CH_SD] is channel error - 2 */
#define SD_ERROR_DATA_RX_CRC_ERROR                    (SD_ERROR_CODE_BASE | 0x08)   /* Data RX CRC Error (sdcDatRxCrcErr) */
#define SD_ERROR_DMA_DATA_RX_ERROR                    (SD_ERROR_CODE_BASE | 0x09)   /* DMA Data RX Error (sdcDmaDatRxErr) */
#define SD_ERROR_DATA_RX_ERROR                        (SD_ERROR_CODE_BASE | 0x0A)   /* Data RX CRC Error (sdcDatRxCrcErr) or DMA Data RX Error (sdcDmaDatRxErr) */
#define SD_ERROR_NO_DAT_MUL_WRITE_TO_SD_CARD          (SD_ERROR_CODE_BASE | 0x0B)   /* No Data Multiple WRITE to SD Card */
#define SD_ERROR_DATA_WRITE_TO_SD_CARD_TIME_OUT       (SD_ERROR_CODE_BASE | 0x0C)   /* sdcSemTx_INT is time-out */
#define SD_ERROR_CRC_ERROR                            (SD_ERROR_CODE_BASE | 0x0D)   /* CRC error */
#define SD_ERROR_DMA_DATA_TX_ERROR                    (SD_ERROR_CODE_BASE | 0x0E)   /* DMA Data Tx Error (sdcDmaDatTxErr) (DMA Controller -> SD Host Controller) */
#define SD_ERROR_DATA_TX_CRC_ERROR                    (SD_ERROR_CODE_BASE | 0x0F)   /* Data Tx CRC Error (sdcDatTxErr) (SD Host Controller -> SD Card) */
#define SD_ERROR_DMA_SD_CHANNEL_TIME_OUT              (SD_ERROR_CODE_BASE | 0x10)   /* dmaSemChFin[DMA_CH_SD] is time-out */
#define SD_ERROR_MUL_TX_TIME_OUT                      (SD_ERROR_CODE_BASE | 0x11)   /* sdcSemMulTx_INT is time-out */
#define SD_ERROR_WRITE_TO_SD_TIME_OUT                 (SD_ERROR_CODE_BASE | 0x12)   /* sdcWriteToSD is not done */
#define SD_ERROR_CARD_IS_NOT_CHANGED_TO_READY         (SD_ERROR_CODE_BASE | 0x13)   /* SDC is not changed from busy to ready */
#define SD_ERROR_WRONG_VOLTAGE_RANGE_OR_PATTERN       (SD_ERROR_CODE_BASE | 0x14)   /* Voltage range isn't supported or check pattern doesn't match */
#define SD_ERROR_WRONG_BLOCK_LENGTH                   (SD_ERROR_CODE_BASE | 0x15)   /* Blcok length isn't 512 Bytes */
#define SD_ERROR_NOT_TRASFER_STATE                    (SD_ERROR_CODE_BASE | 0x16)   /* Current state is not transfer state */
#define SD_ERROR_CARD_IS_WRITE_PROTECTED              (SD_ERROR_CODE_BASE | 0x17)   /* Card is write-protected */
#define SD_ERROR_FAIL_TO_GET_CARD_STATUS              (SD_ERROR_CODE_BASE | 0x18)   /* Fail to get card status */

#define SD_ERROR_FAIL_TO_WRITE_DATA_TO_CARD           (SD_ERROR_CODE_BASE | 0x19)   /* Fail to write data to card */
#define SD_ERROR_FAIL_TO_MUL_WRITE_DATA_TO_CARD       (SD_ERROR_CODE_BASE | 0x1A)   /* Fail to multiple write data to card */

#define SD_ERROR_DMA_CONTROLLER_READ_ERROR            (SD_ERROR_CODE_BASE | 0x1B)   /* DMA controller fails to set to read */
#define SD_ERROR_DMA_CONTROLLER_WRITE_ERROR           (SD_ERROR_CODE_BASE | 0x1C)   /* DMA controller fails to set to write */
#define SD_ERROR_DMA_FAILS                            (SD_ERROR_CODE_BASE | 0x1D)   /* DMA operation fails */

#define SD_ERROR_CARD_DONT_SUPPORT_HIGH_SPEED         (SD_ERROR_CODE_BASE | 0x1E)   /* This card doesn't support High-Speed Mode */
#define SD_ERROR_CHECK_FUNCTION_ERROR                 (SD_ERROR_CODE_BASE | 0x1F)   /* Check function with error */

#define SD_ERROR_CARD_IDENTIFICATION_ERROR            (SD_ERROR_CODE_BASE | 0x20)   /* SecurityDisk / MultiMedia Card identification error */



//===================================================================================
extern INT32S sdcInit(void);
extern INT32S sdcUnInit(void);
extern INT32S sdcMount(void);
extern int sdcDevStatus(unsigned long Unit);
extern int sdcDevRead(unsigned long Unit, unsigned long Sector, void *pBuffer);
extern int sdcDevMulRead(unsigned long Unit, unsigned long Sector, unsigned long NumofSector, void *pBuffer);
extern int sdcDevWrite(unsigned long Unit, unsigned long Sector, void *pBuffer);
extern int sdcDevMulWrite(unsigned long Unit, unsigned long Sector, unsigned long NumofSector, void *pBuffer);
extern int sdcDevIoCtl(unsigned long Unit, long Cmd, long Aux, void *pBuffer);
//===================================================================================

#endif    // __MARS_SDC_H__
