
#include "general.h"
#include <stdio.h>
#include "intapi.h"

#include <mars_int.h>
#include <mars_intreg.h>


FP_VOID  gpIntIRQIsrTbl[INT_IRQID_MAX];
FP_VOID  gpIntFIQIsrTbl[INT_FIQID_MAX];

u32     FiqError=0;

extern u32  Image$$RAM_STACK$$ZI$$Base;
extern u32  Image$$RAM_STACK$$ZI$$Limit;

void marsIntInit(void)
{
    INT32U i;
    volatile INT32U *pIntIrqMask, *pIntFiqMask;
    
    for(i=0; i<INT_IRQID_MAX; i++)
        gpIntIRQIsrTbl[i] = NULL;
    	
    for(i=0; i<INT_FIQID_MAX; i++)
        gpIntFIQIsrTbl[i] = NULL;
    
    pIntIrqMask = (volatile INT32U *)REG_INTIRQMASK;
    pIntFiqMask = (volatile INT32U *)REG_INTFIQMASK;
        
    *pIntIrqMask = 0xFFFFFFFF;
    *pIntFiqMask = 0xFFFFFFFF;
}

void marsIntIRQDefIsr(INT32U dintno, FP_VOID hdl)
{
    if(dintno >= INT_IRQID_MAX)
        return;    // error
    gpIntIRQIsrTbl[dintno] = hdl;
}

void marsIntFIQDefIsr(INT32U dintno, FP_VOID hdl)
{
    if(dintno >= INT_FIQID_MAX)
        return;    // error
    gpIntFIQIsrTbl[dintno] = hdl;
}

void marsIntIRQEnable(INT32U intno)
{
    volatile INT32U *pIntIrqMask;
    
    pIntIrqMask = (volatile INT32U *)REG_INTIRQMASK;
    *pIntIrqMask &= ~(intno);
}

void marsIntIRQDisable(INT32U intno)
{
    volatile INT32U *pIntIrqMask;
    
    pIntIrqMask = (volatile INT32U *)REG_INTIRQMASK;
    *pIntIrqMask |= (intno);
}

void marsIntFIQEnable(INT32U intno)
{
    volatile INT32U *pIntFiqMask;
    
    pIntFiqMask = (volatile INT32U *)REG_INTFIQMASK;
    *pIntFiqMask &= ~(intno);
}

void marsIntFIQDisable(INT32U intno)
{
    volatile INT32U *pIntFiqMask;
    
    pIntFiqMask = (volatile INT32U *)REG_INTFIQMASK;
    *pIntFiqMask |= (intno);
}

INT32U marsIntGetIRQStatus(void)
{
    volatile INT32U *pIntIrqSta;
    
    pIntIrqSta = (volatile INT32U *)REG_INTIRQINPUT;
    return (*pIntIrqSta);
}

INT32U marsIntGetFIQStatus(void)
{
    volatile INT32U *pIntFiqSta;
    
    pIntFiqSta = (volatile INT32U *)REG_INTFIQINPUT;
    return (*pIntFiqSta);
}

void marsIntIRQHandler(void)
{
    INT32U CurrIrqInput, i, CheckBit=0x01;
    //volatile INT32U *pIntIrqSta;

    //pIntIrqSta      = (volatile INT32U *)REG_INTIRQINPUT;    
    //CurrIrqInput    = *pIntIrqSta;
    CurrIrqInput    = IntIrqInput;
               
    for(i = 0; i < INT_IRQID_MAX; i++)
    {
        if( (CurrIrqInput & CheckBit) && (IntIrqInput & CheckBit) && (gpIntIRQIsrTbl[i] != NULL))
        {
            gpIntIRQIsrTbl[i]();      
            if(FiqError)
            {
            #if 1
               DEBUG_SYS("\n===FIQ ERROR1====\n");
            #else    
               printf("CurrIrqInput=0x%08x IntIrqInput=0x%08x IntFiqInput=0x%08x CheckBit=0x%08x gpIntIRQIsrTbl[%d]=0x%08x\n", CurrIrqInput, IntIrqInput, IntFiqInput, CheckBit, i, (u32)gpIntIRQIsrTbl[i]);
            #endif 
               FiqError=0;
            }
        }
        CheckBit <<= 1;
    }
    //*pIntIrqSta = (~CurrIrqInput);
    IntIrqInput     = ~CurrIrqInput;
}
void marsIntFIQHandler(void)
{
    INT32U fiqInput, i, j, CheckBit=0x01;
    //volatile INT32U *pIntFiqSta;
#if FIQ_DEBUG_ENA_LUCIAN
    gpioSetLevel(1, 6, 1);
#endif
    //pIntFiqSta = (volatile INT32U *)REG_INTFIQINPUT;
    //fiqInput = *pIntFiqSta;
    fiqInput    = IntFiqInput;

#if 1
    //Lucian: PA9002D 的CPU 速度較不足. 讓下列INTR先做. Optimized it by experience. 
    //        再者, 考慮到ISU,IDU,SIU同時發出時,其有優先順序 ISU > IDU > SIU. 
    //--ISU--//
    if (fiqInput & INT_FIQMASK_ISU)
    {
    	gpIntFIQIsrTbl[4]();
    	fiqInput &= (~INT_FIQMASK_ISU);
        //*pIntFiqSta = (~INT_FIQMASK_ISU);
        IntFiqInput = ~INT_FIQMASK_ISU;
    }
    
    //--IDU--//
    if (fiqInput & INT_FIQMASK_IDU)
    {
    	gpIntFIQIsrTbl[5]();
    	fiqInput &= (~INT_FIQMASK_IDU);
        //*pIntFiqSta = (~INT_FIQMASK_IDU);
        IntFiqInput = ~INT_FIQMASK_IDU;
    }
    
    //--SIU--//
    if (fiqInput & INT_FIQMASK_SIU)
    {
    	gpIntFIQIsrTbl[2]();
        fiqInput &= (~INT_FIQMASK_SIU);
        //*pIntFiqSta = (~INT_FIQMASK_SIU);
        IntFiqInput = ~INT_FIQMASK_SIU;
    }
#endif

    for(i = 0; i < INT_FIQID_MAX; i++)
    {
        if( (fiqInput & CheckBit) && (IntFiqInput & CheckBit) && (gpIntFIQIsrTbl[i] != NULL) )
        {
            gpIntFIQIsrTbl[i]();
            if(FiqError)
            {
            #if 1
               DEBUG_SYS("\n===FIQ ERROR2====\n");
            #else   
                printf("fiqInput=0x%08x IntFiqInput=0x%08x IntIrqInput=0x%08x CheckBit=0x%08x gpIntFIQIsrTbl[%d]=0x%08x\n", fiqInput, IntFiqInput, IntIrqInput, CheckBit, i, (u32)gpIntFIQIsrTbl[i]);
                for(j = Image$$RAM_STACK$$ZI$$Limit - 4; j >= Image$$RAM_STACK$$ZI$$Base; j -= 16)
                {
                    printf("0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x\n", *(u32*)j, *(u32*)(j - 4), *(u32*)(j - 8), *(u32*)(j - 12));
                }
            #endif
                FiqError=0;
            }
        }
        else if((fiqInput & CheckBit) && !(IntFiqInput & CheckBit))
        {
            #if 0
               DEBUG_SYS("\n===FIQ ERROR3====\n");
            #else 
            printf("2.fiqInput=0x%08x IntFiqInput=0x%08x IntIrqInput=0x%08x CheckBit=0x%08x\n", fiqInput, IntFiqInput, IntIrqInput, CheckBit);
            for(j = Image$$RAM_STACK$$ZI$$Limit - 4; j >= Image$$RAM_STACK$$ZI$$Base; j -= 16)
            {
                printf("0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x\n", *(u32*)j, *(u32*)(j - 4), *(u32*)(j - 8), *(u32*)(j - 12));
            }
            #endif
            FiqError=0;
        }
        CheckBit <<= 1;
    }
    //*pIntFiqSta = (~fiqInput);
    IntFiqInput = ~fiqInput;
#if FIQ_DEBUG_ENA_LUCIAN
    gpioSetLevel(1, 6, 0);
#endif
}

//----------Lucian: 為加強peformance ,將次數多的interrupt移到這裡---------------//

void marsUndefineHandler(void)
{
    printf("UDF ");
}

void marsSWIHandler(void)
{
    printf("SWI ");
}

void marsPrefetchHandler(void)
{
    printf("PFH ");
}

void marsAbortHandler(void)
{
    printf("ABT ");
}
/*

Routine Description:

	The handler of RF Interface Unit.

Arguments:

	None.

Return Value:

	None.

*/
extern unsigned int guiRFTimerID;
extern OS_FLAG_GRP  *gRfiuFlagGrp;
extern unsigned int gRfiuTimer[MAX_RFIU_UNIT];
extern unsigned int gRfiuTxSwCnt[MAX_RFIU_UNIT];

void rfiuIntHandler(void)
{
#if(RFIU_SUPPORT == 1)
   unsigned int intStat;
   unsigned int intStat1;
   unsigned char err;
   unsigned int RFTimer;
   int i;
   //============//
   
   intStat = RFIU_INT_STA;

   timerCountRead(guiRFTimerID, &RFTimer);
   OSFlagPost(gRfiuFlagGrp, intStat , OS_FLAG_SET, &err);

   #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL) 
       if(gRfiuTxSwCnt[0] != 0x0ff)
          gRfiuTimer[ gRfiuTxSwCnt[0] ]=RFTimer;  

   #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
       for(i=0;i<2;i++)
       {
           if(intStat & 0x03)
           {
              if(gRfiuTxSwCnt[i] != 0x0ff)
                 gRfiuTimer[i+ gRfiuTxSwCnt[i]*2 ]=RFTimer;
           }
           intStat=intStat >> 2;
       }
   #else
       for(i=0;i<MAX_RFIU_UNIT;i++)
       {
           if(intStat & 0x03)
              gRfiuTimer[i]=RFTimer;
           intStat=intStat >> 2;
       }
   #endif    
#endif  /*end of #if(RFIU_SUPPORT == 1)*/ 
}


