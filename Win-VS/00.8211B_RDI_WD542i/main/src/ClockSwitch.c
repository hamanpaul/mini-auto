#include "general.h"
#include "board.h"


#define SYSTEMCLOCK_27MHZ   0
#define SYSTEMCLOCK_48MHZ   1
#define SYSTEMCLOCK_64MHZ   2

static u32 Prev_IDUdiv;
extern u8 sysTVOutOnFlag;

void EnterClock64_8mFrom48m(void)
{
     s32 i;
     u32 temp;
     u32 IDU_div;
     
     //SYS clock switch to XIN(27M)
     SYS_CPU_PLLCTL &=  (~SYS_CLK_SEL_MASK);
     SYS_CPU_PLLCTL |=  SYS_CLK_SEL_XIN;
     
     //PLL clock switch to 64.8M
     SYS_CPU_PLLCTL &= (~SYS_PLL_SEL_MASK);
     SYS_CPU_PLLCTL |= SYS_PLL_SEL_64M;
     for(i=0;i<0xffff;i++); //delay
     
     //SYS clock switch to PLL
     SYS_CPU_PLLCTL |= SYS_CLK_SEL_PLL;
     
     //IDU clock=36M
    if(!sysTVOutOnFlag)
    {
     temp  =IduDispCfg;
     Prev_IDUdiv= (temp & IDU_CLK_DIV_MASK);
     IDU_div= (temp & IDU_CLK_DIV_MASK) >>24;
     IDU_div +=1;
     IDU_div=IDU_div*36/48;     
     IDU_div -=1;
     IDU_div <<=24;
     
     temp &= (~IDU_CLK_DIV_MASK);
     temp |= IDU_div;
     IduDispCfg=temp;
    }
     
}

void LeaveClokc64_8mTo48m(void)
{
     s32 i;
     u32 temp;
     //u32 IDU_div;
     
     //SYS clock switch to XIN(27M)
     SYS_CPU_PLLCTL &=  (~SYS_CLK_SEL_MASK);
     SYS_CPU_PLLCTL |=  SYS_CLK_SEL_XIN;
     
     //PLL clock switch to 48M
     SYS_CPU_PLLCTL &= (~SYS_PLL_SEL_MASK);
     SYS_CPU_PLLCTL |= SYS_PLL_SEL_48M;
     for(i=0;i<0xffff;i++); //delay
     
     //SYS clock switch to PLL
     SYS_CPU_PLLCTL |= SYS_CLK_SEL_PLL;
     
     //IDU clock=48M
    if(!sysTVOutOnFlag)
    {
     temp  =IduDispCfg;
     temp &= (~IDU_CLK_DIV_MASK);
     temp |= Prev_IDUdiv;
     IduDispCfg=temp;
    }
    
}

void ChangeSystemClock(u8 freq)
{
    s32 i;
    u32 temp;
    u32 IDU_div;
    u32 IDU_EN;
    switch(freq)
    {    
    case SYSTEMCLOCK_27MHZ:
        //SYS clock switch to XIN(27M)
        SYS_CPU_PLLCTL &=  (~SYS_CLK_SEL_MASK);
        SYS_CPU_PLLCTL |=  SYS_CLK_SEL_XIN;
        //PLL clock switch to 48M
        SYS_CPU_PLLCTL &= (~SYS_PLL_SEL_MASK);
        SYS_CPU_PLLCTL |= SYS_PLL_SEL_48M;
        for(i=0;i<0xffff;i++); //delay
     
        temp  =IduDispCfg;
        temp &= (~IDU_CLK_DIV_MASK);
        temp |= IDU_div;
        IduDispCfg=temp;
        break;
    case SYSTEMCLOCK_48MHZ:
        //SYS clock switch to XIN(27M)
        SYS_CPU_PLLCTL &=  (~SYS_CLK_SEL_MASK);
        SYS_CPU_PLLCTL |=  SYS_CLK_SEL_XIN;
         
        //PLL clock switch to 48M
        SYS_CPU_PLLCTL &= (~SYS_PLL_SEL_MASK);
        SYS_CPU_PLLCTL |= SYS_PLL_SEL_48M;
        for(i=0;i<0xffff;i++); //delay
     
        //SYS clock switch to PLL
        SYS_CPU_PLLCTL |= SYS_CLK_SEL_PLL;
     
        //IDU clock=48M
        temp  =IduDispCfg;
        temp &= (~IDU_CLK_DIV_MASK);
        temp |= Prev_IDUdiv;
        IduDispCfg=temp;
        IDU_EN = IduEna;
        IduEna = 0x00;
        for(i=0;i<0xffff;i++);
        IduEna = 0x01;
        for(i=0;i<0xffff;i++);
        IduEna = IDU_EN;
        break;
    case SYSTEMCLOCK_64MHZ:
        //SYS clock switch to XIN(27M)
        SYS_CPU_PLLCTL &=  (~SYS_CLK_SEL_MASK);
        SYS_CPU_PLLCTL |=  SYS_CLK_SEL_XIN;
     
        //PLL clock switch to 64.8M
        SYS_CPU_PLLCTL &= (~SYS_PLL_SEL_MASK);
        SYS_CPU_PLLCTL |= SYS_PLL_SEL_64M;
        for(i=0;i<0xffff;i++); //delay
     
        //SYS clock switch to PLL
        SYS_CPU_PLLCTL |= SYS_CLK_SEL_PLL;
     
        //IDU clock=36M
        temp  =IduDispCfg;
        Prev_IDUdiv= (temp & IDU_CLK_DIV_MASK);
        IDU_div= (temp & IDU_CLK_DIV_MASK) >>24;
        IDU_div +=1;
        IDU_div=IDU_div*36/48;     
        IDU_div -=1;
        IDU_div <<=24;
     
        temp &= (~IDU_CLK_DIV_MASK);
        temp |= IDU_div;
        IduDispCfg=temp;
        break;
    }
}
