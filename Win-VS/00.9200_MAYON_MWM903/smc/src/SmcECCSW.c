/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	SmcECCSW.c

Abstract:

	ECC algorithm implemetation in S/W.

Environment:

	ARM RealView Developer Suite

Revision History:

	2006/11/15	Lori Chen	Create

*/



/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

#include "board.h"
#include "SmcECCSW.h"
#include "Smc.h"


u8	ucdall;
/*

Routine Description:

	S/W Implementation of RS Algorithm.
	Only in logical Process.

Arguments:

	punBufAddr - the address of data buffer

Return Value:

	RS Correction Result

*/
u32 smcRsCorrection(u8* punBufAddr)
{

    u32	uErrpage;
	u32	uErrNum;
	u16	uErrpost[4];
    u8	i,j,CorrSym[4];
    u8 *pData;
    u32	Result;
    volatile unsigned *pPost,*pCorrSym; 
	
    pData = punBufAddr;
	
	uErrpage = SmcChkErrAddr;
	
	if (uErrpage ==0) return 0;
	else 
		{
			for (i=0;i<16 ;i++)
				{
//				DEBUG_SMC("uErrpage %x \n",uErrpage);
                Result = uErrpage & 0x01;
				if(Result)
					{
					 DEBUG_SMC("====Sector %d Error====\n",i+1);
					 uErrNum = SmcChkErrNum;	
					 uErrNum = (uErrNum >> i*2)&3;
					 uErrNum++;
					 DEBUG_SMC("Error Num %d \n",uErrNum);
					if(i<9)
						{
							pPost =	 i*2 + (&SmcCheErrPos1st);//SmcCheErrPos1st
							pCorrSym = i*2 + (&SmcCorrtSym1st);//SmcCorrtSym1st
						}
					else 
						{
							pPost =	(i-9)*2 + (&SmcCheErrPos9th);	
							pCorrSym = (i-9)*2 + (&SmcCorrtSym9th);
						}
					
						 DEBUG_SMC("Error SmcCheErrPos1 %x=%x\n",&pPost[0],pPost[0]);
						 DEBUG_SMC("Error SmcCorrtSym1  %x=%x\n",&pCorrSym[0],pCorrSym[0]);
						 
						 DEBUG_SMC("Error SmcCheErrPos2 %x=%x\n",&pPost[1],pPost[1]);
						 DEBUG_SMC("Error SmcCorrtSym2  %x=%x\n",&pCorrSym[1],pCorrSym[1]);
						 
							uErrpost[0] = pPost[0];
							uErrpost[1] = pPost[0]>>16;
							uErrpost[2] = pPost[1];
							uErrpost[3] = pPost[1]>>16;
	
							CorrSym[0] = pCorrSym[0];
							CorrSym[1] = pCorrSym[0]>>16;
							CorrSym[2] = pCorrSym[1];
							CorrSym[3] = pCorrSym[1]>>16;
							
							uErrpost[0] = 512*i + (519 - uErrpost[0]);
							uErrpost[1] = 512*i + (519 - uErrpost[1]);
							uErrpost[2] = 512*i + (519 - uErrpost[2]);
							uErrpost[3] = 512*i + (519 - uErrpost[3]);

					for (j=0;j<uErrNum ;j++)
				 		{
				 		    if (uErrpost[j] < 8) DEBUG_SMC("Redundant Area Error  %x \n",uErrNum);
					 		pData[uErrpost[j]] ^= CorrSym[j];
							DEBUG_SMC("Error Position: %d CorrtData: %x\n",uErrpost[j],pData[uErrpost[j]]);
				 		}
					}
					
				uErrpage = uErrpage >> 1;
				}
		}
	
    return 1;

}
/*

Routine Description:

	S/W Implementation of ECC Algorithm.
	Only in logical Process.

Arguments:

	punBufAddr - the address of data buffer

Return Value:

	24-bit ECC Result

*/
u32 smcECC_SW(u32* punBufAddr)
{


    u8	uccpn = 0x00;
    u16	uslpn = 0x0000;
    u8	ucCnt = 0x00;		//initialize clock counter
    u8	i;
    u8	ucdallStore[256];
    u32*		punDataToWrite;
    u32	unECCResult;


    punDataToWrite = (u32*)punBufAddr;


    for (i = 0; i < 256/4; i++)	//only test 256 bytes (half-page)
    {

        uccpn = smcCPN_Process((u8)(*(u32*)punDataToWrite & 0x000000FF), uccpn);
        ucdallStore[ucCnt] = smcOneBitXOR((u8)(*(u32*)punDataToWrite & 0x000000FF), 0x00FF, 8);
        uslpn = smcLPN_Process(ucCnt , uslpn, ucdallStore[ucCnt]);
        ucCnt++;


        unECCResult = (  (~((u32)uccpn << 16)) & 0x00FC0000);
        unECCResult |= 0x00030000;
        unECCResult |= ((u32)(~uslpn) & 0x0000FFFF);


        uccpn = smcCPN_Process((u8)((*(u32*)punDataToWrite >> 8) & 0x000000FF), uccpn);
        ucdallStore[ucCnt] = smcOneBitXOR((u8)((*(u32*)punDataToWrite >> 8) & 0x000000FF), 0x00FF, 8);
        uslpn = smcLPN_Process(ucCnt , uslpn, ucdallStore[ucCnt]);
        ucCnt++;

        unECCResult = (  (~((u32)uccpn << 16)) & 0x00FC0000);
        unECCResult |= 0x00030000;
        unECCResult |= ((u32)(~uslpn) & 0x0000FFFF);

        uccpn = smcCPN_Process((u8)((*(u32*)punDataToWrite >> 16) & 0x000000FF), uccpn);
        ucdallStore[ucCnt] =  smcOneBitXOR((u8)((*(u32*)punDataToWrite >> 16) & 0x000000FF), 0x00FF, 8);
        uslpn = smcLPN_Process(ucCnt , uslpn, ucdallStore[ucCnt]);
        ucCnt++;

        unECCResult = (  (~((u32)uccpn << 16)) & 0x00FC0000);
        unECCResult |= 0x00030000;
        unECCResult |= ((u32)(~uslpn) & 0x0000FFFF);

        uccpn = smcCPN_Process((u8)((*(u32*)punDataToWrite >> 24) & 0x000000FF), uccpn);
        ucdallStore[ucCnt] = smcOneBitXOR((u8)((*(u32*)punDataToWrite >> 24) & 0x000000FF), 0x00FF, 8);
        uslpn = smcLPN_Process(ucCnt , uslpn, ucdallStore[ucCnt]);
        ucCnt++;

        unECCResult = (  (~((u32)uccpn << 16)) & 0x00FC0000);
        unECCResult |= 0x00030000;
        unECCResult |= ((u32)(~uslpn) & 0x0000FFFF);

        *punDataToWrite ++;

    }

    unECCResult = (  (~((u32)uccpn << 16)) & 0x00FC0000);
    unECCResult |= 0x00030000;
    unECCResult |= ((u32)(~uslpn) & 0x0000FFFF);

    return unECCResult;



}


/*

Routine Description:

	Calculate the ECC S/W Algorithm Result.
	CPN are parts of ECC results.
	CPN[5:0] => ECC[23:18]

Arguments:

	ucDataBufForCPN - Data that will be process with ECC algorithm.
	ucCPNPrevStage - The previous stage data of CPN[5:0].

Return Value:

	8-bit result where 	[7:6] => don't care.
					[5:0] => CPN[5:0].

*/

u8 smcCPN_Process(u8 ucDataBufForCPN, u8 ucCPNPrevStage)
{

    u16	usDataInput;
    u8	ucCPNResult = 0x00;


    usDataInput = (u16)ucDataBufForCPN;

    //cpn0
    usDataInput = (u16)ucDataBufForCPN | (((u16)ucCPNPrevStage) << 6);
    ucCPNResult |= (smcOneBitXOR(usDataInput, 0x0155, 9)<<2);

    //cpn1
    usDataInput = (u16)ucDataBufForCPN | ((((u16)ucCPNPrevStage) << 5) & 0x0100);
    ucCPNResult |= (smcOneBitXOR(usDataInput, 0x01AA, 9)<<3);

    //cpn2
    usDataInput = (u16)ucDataBufForCPN | ((((u16)ucCPNPrevStage) << 4) & 0x0100);
    ucCPNResult |= (smcOneBitXOR(usDataInput, 0x0133, 9)<<4);

    //cpn3
    usDataInput = (u16)ucDataBufForCPN | ((((u16)ucCPNPrevStage) << 3) & 0x0100);
    ucCPNResult |= (smcOneBitXOR(usDataInput, 0x01CC, 9)<<5);

    //cpn4
    usDataInput = (u16)ucDataBufForCPN | ((((u16)ucCPNPrevStage) << 2) & 0x0100);
    ucCPNResult |= (smcOneBitXOR(usDataInput, 0x010F, 9)<<6);

    //cpn5
    usDataInput = (u16)ucDataBufForCPN | ((((u16)ucCPNPrevStage) << 1) & 0x0100);
    ucCPNResult |= (smcOneBitXOR(usDataInput, 0x01F0, 9)<<7);

    return ucCPNResult;

}


/*

Routine Description:

	XOR process

Arguments:

	usXORData -  data will be XORed.
	usOperationBits - bits indication that will be refered to
	ucBitLength - the bit length that will be processed

Return Value:

	1-bit result of XOR

*/

u8 smcOneBitXOR(u16 usXORData, u16 usOperationBits, u8 ucBitLength)
{
    u8	i;
    u8	ucBitIndicator = 0;
    u16	usDataTemp;

    usXORData &= usOperationBits;

    usDataTemp = usXORData;

    for (i=0; i<ucBitLength; i++)
    {
        usDataTemp &= 0x0001;

        ucBitIndicator ^= (u8)usDataTemp;

        usDataTemp = usXORData >>( i+1);
    }

    return ucBitIndicator;

}


/*

Routine Description:

	Calculate the ECC S/W Algorithm Result.
	LPN are parts of ECC results.
	LPN[15:0] => ECC[15:0]

Arguments:

	ucClkCnt - Clock Cycle Count
	usLPNPrevStage - the LPN in the previous stage
	ucdall - the result after input data XORed

Return Value:

	16-bit result where


*/
u16 smcLPN_Process(u8 ucClkCnt, u16 usLPNPrevStage, u8 ucdall)
{

    //u16	usDataInput;
    //u16	usDataTemp;
    u8	i;
    //u8	ucCPNResult = 0x00;

    u8	ucClkCntIdx;
    u16	usLPNResult;
    u16	usLPNOutputBitIdx = 0x0001;

    usLPNResult = usLPNPrevStage;

    for ( i=0; i<8; i++)
    {
        ucClkCntIdx= (ucClkCnt >> i) & 0x01;

        if (ucClkCntIdx == 1)
        {

            if (( ((usLPNPrevStage >> (i*2+1)) & usLPNOutputBitIdx) ^ ((u16)ucdall) ))
                usLPNResult = usLPNResult | (1 << (i*2+1));

            else
                usLPNResult = usLPNResult & ~(1 << (i*2+1));

        }
        else
        {

            if (( ((usLPNPrevStage >> (i*2)) & usLPNOutputBitIdx) ^ ((u16)ucdall) ))
                usLPNResult = usLPNResult | (1 << (i*2));

            else
                usLPNResult = usLPNResult & ~(1 << (i*2));

        }



    }

    return usLPNResult;

}
