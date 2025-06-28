#ifndef _AMIC7130_H_

#define _AMIC7130_H_


extern void A7130_WriteReg_B1(BYTE addr, BYTE dataByte);
extern void ByteSend_B1(BYTE src);
extern  BYTE A7130_ReadReg_B1(BYTE addr);
extern BYTE ByteRead_B1();
extern void A7130_Cal_B1();
extern void A7130_Config_B1();

extern void A7130_WriteReg_B2(BYTE addr, BYTE dataByte);
extern void ByteSend_B2(BYTE src);
extern  BYTE A7130_ReadReg_B2(BYTE addr);
extern BYTE ByteRead_B2();
extern void A7130_Cal_B2();
extern void A7130_Config_B2();

extern void A7130_TxMode();
extern void A7130_RxMode();




#endif  //  AMIC7130 H_