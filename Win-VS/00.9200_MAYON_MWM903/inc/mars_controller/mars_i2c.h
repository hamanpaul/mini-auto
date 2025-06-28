

#ifndef __MARS_I2C_H__
#define __MARS_I2C_H__

#include <osapi.h>

typedef struct _I2C_CFG {
    INT32U  uiSlaveAddr;
    INT32U  uiI2CFreq;
    INT32U  uiTxBytes;
} I2C_CFG;

//=================================================================
extern void marsI2CInit(void);
extern INT32U marsI2COpen(I2C_CFG *pI2CCfg);
extern INT32U marsI2CClose(void);
extern INT32U marsI2CWriteByte(INT32U uiRegAddr, INT32U uiRegData);
extern INT32U marsI2CReadByte(INT32U uiRegAddr, INT32U *pRegData);

//=================================================================

#endif    // __MARS_I2C_H__
