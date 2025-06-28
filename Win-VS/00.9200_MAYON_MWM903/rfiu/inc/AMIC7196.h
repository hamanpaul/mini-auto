#ifndef _AMIC7196_H_

#define _AMIC7196_H_


extern void A7196_WriteReg_B1(BYTE addr, BYTE dataByte);
extern  BYTE A7196_ReadReg_B1(BYTE addr);
extern void ByteSend_B1(BYTE src);
extern BYTE ByteRead_B1();
extern void ByteSend_B2(BYTE src);
extern BYTE ByteRead_B2();
extern void A7196_Cal_B1();
extern void A7196_Config_B1();

extern void A7196_WriteReg_B2(BYTE addr, BYTE dataByte);
extern  BYTE A7196_ReadReg_B2(BYTE addr);
extern void A7196_Cal_B2();
extern void A7196_Config_B2();

extern void A7196_TxMode();
extern void A7196_RxMode();




#endif  
