/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    gfu.c

Abstract:

    The routines of 2D Graphic Unit.

Environment:

        ARM RealView Developer Suite

Revision History:

    2014/03/26      Lucian Yuan     create

*/

/*========= Programer guide=======//
1. src1 and src2 address must be word alignment
2. 

*/

#include "general.h"
#include "board.h"
#include "iduapi.h"
#include "idureg.h"
#include "gpioapi.h"
#include "../board/inc/intreg.h"
#include "Timerapi.h"
#include "sysapi.h"
#include "iisapi.h"
#include "rtcapi.h"
#include "uiapi.h"
#include "ciuapi.h"
#include "isuapi.h"
#include "../isu/inc/isu.h"
#include "gfuapi.h"


#define GFU_TEST_BLOCKCOPY   0
#define GFU_TEST_LINEDRAW    1
#define GFU_TEST_V2V         0
#define GFU_TEST_O2V         0
#define GFU_TEST_O2O         1

#if GFU_SUPPORT

  

/*********************
*	Constant
*********************/
#define GFU_OSD2OSD_Cmd       0
#define GFU_VDO2VDO_Cmd       1
#define GFU_OSD2VDO_Cmd       2
#define GFU_BLKCPY_Cmd        3
#define GFU_RECTDRAW_Cmd      4




typedef struct _GFU_CMDDATA
{
   unsigned int Command;         
   unsigned int Src1_Addr_Y;       
   unsigned int Src1_Addr_C;    
   unsigned int Src1_WidthHieght;   
   
   unsigned int Src2_Addr_Y;       
   unsigned int Src2_Addr_C;  //or ColorIdx_1    
   unsigned int Src2_WidthHieght;  
   unsigned int Src1Src2_Stride;
   
   unsigned int StartPosXY;
   unsigned int EndPosXY;
   unsigned int Dest_Addr_Y;
   unsigned int Dest_Addr_C;

   unsigned int Dest_Stride;
   unsigned int Dest_WidthHeight;
   unsigned int ColorIdx_2;
   unsigned int ColorIdx_3;
}DEF_GFU_CMDDATA;

/*********************
*	Variable
*********************/
OS_EVENT    *gfuQueReadySemEvt;


/*********************
* Exteral	Variable
*********************/

extern u32 ciu_idufrmcnt_ch1;
extern u32 ciu_idufrmcnt_ch2;
extern u32 ciu_idufrmcnt_ch3;
extern u32 ciu_idufrmcnt_ch4;

extern u32 ciu_1_pnbuf_size_y;
extern u32 ciu_2_pnbuf_size_y;
extern u32 ciu_3_pnbuf_size_y;
extern u32 ciu_4_pnbuf_size_y;

extern  u32 *CiuOverlayImg1_Top;
extern  u8 OSD_ASCII_20x28[96][20*28];
extern  u8 *PNBuf_Quad;


/*********************
*	Function Prototype
*********************/
int gfuSetCommad(DEF_GFU_CMDDATA *pCmd);
int gfuCheckQueStatus(void);


/*********************
*	Function Body
*********************/

int gfuInit(void)
{
    gfuSwQueWrIdx=0;
    gfuSwQueRdIdx=0;
    gfuQueReadySemEvt=OSSemCreate(1);

    memset(gfuSwQueAddr,0,GFU_SWQUE_CMDMAX * 16 * 4);

    GFU_QUE_WP  = 0;
    GFU_QUE_SIZE= GFU_SWQUE_CMDMAX;
    GFU_QUE_ADDR= (u32)gfuSwQueAddr;
    GFU_CTL     = GFU_ENA;

    return 1;
}

int gfuReset(void)
{
  
     GFU_CTL |= GFU_RESET;

     return 1;
}

int gfuSetCommad(DEF_GFU_CMDDATA *pCmd)
{
    int QueRemainSize;
    u8 err;

    OSSemPend(gfuQueReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    QueRemainSize=gfuCheckQueStatus();
    
    while(QueRemainSize<3)
    {
       DEBUG_IDU("GFX Que Full:%d,%d,%d\n",QueRemainSize,GFU_QUE_WP,GFU_QUE_RP);
       OSTimeDly(1);
       QueRemainSize=gfuCheckQueStatus();
    }
    
    memcpy(gfuSwQueAddr+16*4*gfuSwQueWrIdx,pCmd,16*4);
    gfuSwQueWrIdx = (gfuSwQueWrIdx+1) % GFU_SWQUE_CMDMAX;
    GFU_QUE_WP=gfuSwQueWrIdx;

    OSSemPost(gfuQueReadySemEvt);
    return 1;
}


int gfuCheckQueStatus(void)
{
    int QueRemainSize;
 
    gfuSwQueRdIdx=GFU_QUE_RP;
    if(gfuSwQueWrIdx==gfuSwQueRdIdx)
    {
        QueRemainSize=GFU_SWQUE_CMDMAX;
    }
    else if(gfuSwQueWrIdx>gfuSwQueRdIdx)
    {
        QueRemainSize=GFU_SWQUE_CMDMAX-(gfuSwQueWrIdx-gfuSwQueRdIdx);
    }
    else
    {
        QueRemainSize=gfuSwQueRdIdx-gfuSwQueWrIdx;
    }

    return QueRemainSize;
}

int gfuTest_V2V_Cmd(  int Src1_X,int Src1_Y,
                              int Src1_W,int Src1_H,
                              int Src2_X,int Src2_Y,
                              int Dst_X,int Dst_Y,
                              int Alpha
                            )
{
    DEF_GFU_CMDDATA GfuCmd;
    int Src1Stride,Src2Stride,DesStride;
    //--------//
    Src1_X= Src1_X & (~0x03);
    Src1_W= Src1_W & (~0x03);

    Dst_X=Dst_X & (~0x03);

    Src1Stride=640;
    Src2Stride=640;
    DesStride =640;

    GfuCmd.Command         =(Alpha<<8) | GFU_VDO2VDO_Cmd;
    GfuCmd.Src1_Addr_Y     =(unsigned int)PNBuf_sub1[(ciu_idufrmcnt_ch1-1) & 0x03]+Src1_X+Src1_Y*Src1Stride;
    GfuCmd.Src1_Addr_C     =(unsigned int)PNBuf_sub1[(ciu_idufrmcnt_ch1-1) & 0x03]+ciu_1_pnbuf_size_y+Src1_X+Src1_Y/2*Src1Stride;
    GfuCmd.Src1_WidthHieght=(Src1_W<<16) | (Src1_H);

    GfuCmd.Src2_Addr_Y     =(unsigned int)PNBuf_sub2[(ciu_idufrmcnt_ch2-1) & 0x03]+Src2_X+Src2_Y*Src2Stride;       
    GfuCmd.Src2_Addr_C     =(unsigned int)PNBuf_sub2[(ciu_idufrmcnt_ch2-1) & 0x03]+ciu_2_pnbuf_size_y+Src2_X+Src2_Y/2*Src2Stride;
    GfuCmd.Src2_WidthHieght=(Src1_W<<16) | (Src1_H);  
    GfuCmd.Src1Src2_Stride =(Src1Stride<<16) | Src2Stride; 
   
    GfuCmd.StartPosXY      =0;
    GfuCmd.EndPosXY        =(Src1_W<<16) | (Src1_H);
    GfuCmd.Dest_Addr_Y     =(unsigned int)PNBuf_Quad+Dst_X+Dst_Y*DesStride;
    GfuCmd.Dest_Addr_C     =(unsigned int)PNBuf_Quad+PNBUF_SIZE_Y+Dst_X+Dst_Y/2*DesStride;

    GfuCmd.Dest_Stride     =DesStride;
    GfuCmd.Dest_WidthHeight=(Src1_W<<16) | Src1_H;
    GfuCmd.ColorIdx_2      =0;
    GfuCmd.ColorIdx_3      =0;  

    gfuSetCommad(&GfuCmd);


    return 1;
}


int gfuTest_BCP_Cmd(  int Src1_X,int Src1_Y,
                               int Src1_W,int Src1_H,
                               int Dst_X,int Dst_Y
                            )
{
    DEF_GFU_CMDDATA GfuCmd;
    int SrcStride,DesStride;
    //--------//
    Src1_X= Src1_X & (~0x03);
    Src1_W= Src1_W & (~0x03);

    Dst_X=Dst_X & (~0x03);

    SrcStride=640;
    DesStride=640;

    GfuCmd.Command         =GFU_BLKCPY_Cmd;
    GfuCmd.Src1_Addr_Y     =(unsigned int)PNBuf_sub1[(ciu_idufrmcnt_ch1-1) & 0x03]+Src1_X+Src1_Y*SrcStride;
    GfuCmd.Src1_Addr_C     =(unsigned int)PNBuf_sub1[(ciu_idufrmcnt_ch1-1) & 0x03]+ciu_1_pnbuf_size_y+Src1_X+Src1_Y/2*SrcStride;
    GfuCmd.Src1_WidthHieght=(Src1_W<<16) | (Src1_H);

    GfuCmd.Src2_Addr_Y     =0;       
    GfuCmd.Src2_Addr_C     =0;
    GfuCmd.Src2_WidthHieght=0;  
    GfuCmd.Src1Src2_Stride =SrcStride<<16; 
   
    GfuCmd.StartPosXY      =0;
    GfuCmd.EndPosXY        =0;
    GfuCmd.Dest_Addr_Y     =(unsigned int)PNBuf_Quad+Dst_X+Dst_Y*DesStride;
    GfuCmd.Dest_Addr_C     =(unsigned int)PNBuf_Quad+PNBUF_SIZE_Y+Dst_X+Dst_Y/2*DesStride;

    GfuCmd.Dest_Stride     =DesStride;
    GfuCmd.Dest_WidthHeight=(Src1_W<<16) | Src1_H;
    GfuCmd.ColorIdx_2      =0;
    GfuCmd.ColorIdx_3      =0;  

    gfuSetCommad(&GfuCmd);

    return 1;
}

int gfuTest_Rect_Cmd(  int Dst_X,int Dst_Y,int Dst_W,int Dst_H,unsigned int color)
{
    DEF_GFU_CMDDATA GfuCmd;
    int DesStride;
    //--------//

    Dst_X=Dst_X & (~0x01);
    Dst_W=Dst_W & (~0x01);

    DesStride=640;

    GfuCmd.Command         =GFU_RECTDRAW_Cmd;
    GfuCmd.Src1_Addr_Y     =0;
    GfuCmd.Src1_Addr_C     =0;
    GfuCmd.Src1_WidthHieght=0;

    GfuCmd.Src2_Addr_Y     =0;       
    GfuCmd.Src2_Addr_C     =0;
    GfuCmd.Src2_WidthHieght=0;  
    GfuCmd.Src1Src2_Stride =0; 
   
    GfuCmd.StartPosXY      =0;
    GfuCmd.EndPosXY        =0;
    GfuCmd.Dest_Addr_Y     =(unsigned int)PNBuf_Quad+Dst_X+Dst_Y*DesStride;
    GfuCmd.Dest_Addr_C     =(unsigned int)PNBuf_Quad+PNBUF_SIZE_Y+Dst_X+Dst_Y/2*DesStride;

    GfuCmd.Dest_Stride     =DesStride;
    GfuCmd.Dest_WidthHeight=(Dst_W<<16) | Dst_H;
    GfuCmd.ColorIdx_2      =color;
    GfuCmd.ColorIdx_3      =0;  

    gfuSetCommad(&GfuCmd);
    return 1;
}


int gfuTest_O2V_Cmd(  int Src1_X,int Src1_Y,char ch, u32 IndexColor)
{
    DEF_GFU_CMDDATA GfuCmd;
    int SrcStride,DesStride;
    //--------//
    Src1_X= Src1_X & (~0x03);

    SrcStride=640;
    DesStride=640;

    GfuCmd.Command         =GFU_OSD2VDO_Cmd;
    GfuCmd.Src1_Addr_Y     =(unsigned int)PNBuf_Quad+Src1_X+Src1_Y*SrcStride;
    GfuCmd.Src1_Addr_C     =(unsigned int)PNBuf_Quad+PNBUF_SIZE_Y+Src1_X+Src1_Y/2*SrcStride;
    GfuCmd.Src1_WidthHieght=((ASCII_LARGE_FONT_WIDTH)<<16) | (ASCII_LARGE_FONT_HEIGHT);
 #if ISU_OVERLAY_ENABLE
    GfuCmd.Src2_Addr_Y     =(unsigned int)ASCII_Font[ch-32];     
 #else
    DEBUG_IDU("Warning!! Not include ISU_OVERLAY_ENABLE\n");
 #endif
    GfuCmd.Src2_Addr_C     =IndexColor;
    GfuCmd.Src2_WidthHieght=((ASCII_LARGE_FONT_WIDTH*ASCII_LARGE_FONT_HEIGHT*2/8)<<16) | (1);  
    GfuCmd.Src1Src2_Stride =(SrcStride<<16) | (ASCII_LARGE_FONT_WIDTH*ASCII_LARGE_FONT_HEIGHT*2/8); 
   
    GfuCmd.StartPosXY      =0;
    GfuCmd.EndPosXY        =((ASCII_LARGE_FONT_WIDTH)<<16) | (ASCII_LARGE_FONT_HEIGHT);
    GfuCmd.Dest_Addr_Y     =GfuCmd.Src1_Addr_Y;
    GfuCmd.Dest_Addr_C     =GfuCmd.Src1_Addr_C;

    GfuCmd.Dest_Stride     =DesStride;
    GfuCmd.Dest_WidthHeight=((ASCII_LARGE_FONT_WIDTH)<<16) | (ASCII_LARGE_FONT_HEIGHT);
    GfuCmd.ColorIdx_2      =IndexColor;
    GfuCmd.ColorIdx_3      =IndexColor;  

    gfuSetCommad(&GfuCmd);

    return 1;
}


int gfuTest_O2O_Cmd(int Src1_X,int Src1_Y,char ch,
                             u32 OldColor, u32 NewColor)
{
    DEF_GFU_CMDDATA GfuCmd;
    int SrcStride,DesStride;
    int Ch_W,Ch_H;
    int Remain_X,Carry;
    //--------//
    Remain_X=Src1_X & 0x03;
    Src1_X = Src1_X & (~0x03);

    if(Remain_X)
       Carry=4-Remain_X;
    else
       Carry=0;
    
#if OSD_SIZE_X2_DISABLE
    SrcStride=640;
    DesStride=640;
#else
    SrcStride=320;
    DesStride=320;
#endif
    
    Ch_W=20;
    Ch_H=28;
    

    GfuCmd.Command         =GFU_OSD2OSD_Cmd;
    GfuCmd.Src1_Addr_Y     =(unsigned int)OSD_buf+Src1_X+Src1_Y*SrcStride;
    GfuCmd.Src1_Addr_C     =0;
    GfuCmd.Src1_WidthHieght=((Ch_W+Remain_X+Carry)<<16) | (Ch_H);

#if ((HW_BOARD_OPTION != MR9200_RX_RDI) && (HW_BOARD_OPTION != MR9200_RX_TRANWO)&&\
    (HW_BOARD_OPTION != MR9200_RX_TRANWO_D8795) && (UI_VERSION != UI_VERSION_TRANWO) &&\
    (HW_BOARD_OPTION != MR9200_RX_TRANWO_D8795R2) && (HW_BOARD_OPTION  != MR9200_RX_TRANWO_D8797R) &&\
    (HW_BOARD_OPTION != MR9200_RX_ROULE)  &&\
    (HW_BOARD_OPTION != MR9200_RX_TRANWO_D8710R) && (HW_BOARD_OPTION != MR9200_RX_TRANWO_D8796P) &&\
    (HW_BOARD_OPTION != MR9200_RX_TRANWO_D8897H) && (HW_BOARD_OPTION != MR9200_RX_TRANWO_SH8710R) &&\
    (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
    GfuCmd.Src2_Addr_Y     =(unsigned int)OSD_ASCII_20x28[ch-32];
#endif
    GfuCmd.Src2_Addr_C     =0;
    GfuCmd.Src2_WidthHieght=((Ch_W*Ch_H)<<16) | (1);  
    GfuCmd.Src1Src2_Stride =(SrcStride<<16) | (Ch_W*Ch_H); 
   
    GfuCmd.StartPosXY      = (Remain_X<<16) | 0;
    GfuCmd.EndPosXY        =((Ch_W+Remain_X)<<16) | (Ch_H);
    GfuCmd.Dest_Addr_Y     =GfuCmd.Src1_Addr_Y;
    GfuCmd.Dest_Addr_C     =0;

    GfuCmd.Dest_Stride     =DesStride;
    GfuCmd.Dest_WidthHeight=((Ch_W+Remain_X+Carry)<<16) | (Ch_H);
    GfuCmd.ColorIdx_2      =OldColor;
    GfuCmd.ColorIdx_3      =NewColor;  

    gfuSetCommad(&GfuCmd);

    return 1;
}


int gfuTest_ALL_Cmd(void)
{
     static u32 testcnt=0;
     static u32 BCP_Xpos=0;
     static u32 BCP_Ypos=0;
     
     static u32 Rect_Xpos=0;
     static u32 Rect_Ypos=240;

     static u32 O2V_Xpos=0;

     static s32 V2V_Xpos=0;
     static s32 V2V_Ypos=0;
     static s32 prevV2V_Xpos=0;
     static s32 prevV2V_Ypos=0;
     static int V2Vreturn_flag=0;


     static s32 O2O_Xpos=0;
     static s32 O2O_Ypos=0;
     static int O2Oreturn_flag=0;
     static s32 prevO2O_Xpos=0;
     static s32 prevO2O_Ypos=0;
     //----------Block Copy-----------//  
 #if GFU_TEST_BLOCKCOPY    
     gfuTest_Rect_Cmd(BCP_Xpos,0,4,240,0x00008080);

     BCP_Xpos += 4;
     BCP_Ypos += 1;     

     if(BCP_Xpos > 320 )
     {
        BCP_Xpos=0;
     }
     if(BCP_Ypos > 240)
        BCP_Ypos=0;
   
     gfuTest_BCP_Cmd(  BCP_Xpos,BCP_Ypos,
                       320,240,
                       BCP_Xpos,0
                    );

     if(BCP_Xpos+320+4<640)
       gfuTest_Rect_Cmd(BCP_Xpos+320,0,4,240,0x00008080);
  #endif
     //-------Line Draw---------//

#if GFU_TEST_LINEDRAW         
     //if( (testcnt & 0x3)== 0x00)
     {
        gfuTest_Rect_Cmd(Rect_Xpos,Rect_Ypos,200,2,0x00008080);
        gfuTest_Rect_Cmd(Rect_Xpos,Rect_Ypos+100,200,2,0x00008080);
        gfuTest_Rect_Cmd(Rect_Xpos,Rect_Ypos,2,100,0x00008080);
        gfuTest_Rect_Cmd(Rect_Xpos+200,Rect_Ypos,2,100+2,0x00008080);

        Rect_Xpos +=2;
        Rect_Ypos +=1;
        if(Rect_Xpos>320-200)
        {
           Rect_Xpos=0;
        }
        if(Rect_Ypos>470-100)
           Rect_Ypos=240; 
 
        gfuTest_Rect_Cmd(Rect_Xpos,Rect_Ypos,200,2,0xffff8080);
        gfuTest_Rect_Cmd(Rect_Xpos,Rect_Ypos+100,200,2,0xffff8080);
        gfuTest_Rect_Cmd(Rect_Xpos,Rect_Ypos,2,100,0xffff8080);
        gfuTest_Rect_Cmd(Rect_Xpos+200,Rect_Ypos,2,100+2,0xffff8080);
     }
 #endif
     //------------- V2V--------------//
 #if GFU_TEST_V2V             
     V2V_Ypos=32;
     if(V2Vreturn_flag)
        V2V_Xpos -= 4;
     else
        V2V_Xpos += 4;

     if(V2V_Xpos > 160 )
     {
        V2Vreturn_flag=1;
        V2V_Xpos=160;
     }
     else if(V2V_Xpos < 0)
     {
        V2Vreturn_flag=0;
        V2V_Xpos=0;
     }

     if(V2Vreturn_flag)
        gfuTest_Rect_Cmd(320+prevV2V_Xpos+160,240+prevV2V_Ypos,4,128,0x00008080);
     else
        gfuTest_Rect_Cmd(320+prevV2V_Xpos,240+prevV2V_Ypos,4,128,0x00008080);
     
     gfuTest_V2V_Cmd(
                       320+V2V_Xpos,240+V2V_Ypos,
                       160,128,
                       320+V2V_Xpos,240+V2V_Ypos,
                       320+V2V_Xpos,240+V2V_Ypos,
                       GFU_ALPHA_50P
                    );
     
     prevV2V_Xpos=V2V_Xpos;
     prevV2V_Ypos=V2V_Ypos;
  #endif
     //--------- O2V -----------//
  #if GFU_TEST_O2V                
     gfuTest_O2V_Cmd(O2V_Xpos,350,'A',0x00008080);
     gfuTest_O2V_Cmd(O2V_Xpos+32,350,'B',0x00008080);
     gfuTest_O2V_Cmd(O2V_Xpos+32+32,350,'C',0x00008080);
     gfuTest_O2V_Cmd(O2V_Xpos+32+32+32,350,'D',0x00008080);
     O2V_Xpos +=4;
     if(O2V_Xpos > 320-32*4 )
     {
        O2V_Xpos=0;
     }
     gfuTest_O2V_Cmd(O2V_Xpos,350,'A',0xffff8080);
     gfuTest_O2V_Cmd(O2V_Xpos+32,350,'B',0xffff8080);
     gfuTest_O2V_Cmd(O2V_Xpos+32+32,350,'C',0xffff8080);
     gfuTest_O2V_Cmd(O2V_Xpos+32+32+32,350,'D',0xffff8080);
  #endif
     //------------- O2O--------------//
  #if GFU_TEST_O2O    
     O2O_Ypos +=1;
     if(O2O_Ypos>200) 
        O2O_Ypos=0;
     
     if(O2Oreturn_flag)
        O2O_Xpos -= 1;
     else
        O2O_Xpos += 1;

     if(O2O_Xpos > 240 )
     {
        O2Oreturn_flag=1;
        O2O_Xpos=240;
     }
     else if(O2O_Xpos < 0)
     {
        O2Oreturn_flag=0;
        O2O_Xpos=0;
     }

     gfuTest_O2O_Cmd(prevO2O_Xpos,prevO2O_Ypos,'W',0xc0,0x00);
     gfuTest_O2O_Cmd(prevO2O_Xpos+20,prevO2O_Ypos,'X',0xc0,0x02);
     gfuTest_O2O_Cmd(prevO2O_Xpos+20+20,prevO2O_Ypos,'Y',0xc0,0x03);
     gfuTest_O2O_Cmd(prevO2O_Xpos+20+20+20,prevO2O_Ypos,'Z',0xc0,0x04);
    
     gfuTest_O2O_Cmd(O2O_Xpos,O2O_Ypos,'W',0xc0,0xc0);
     gfuTest_O2O_Cmd(O2O_Xpos+20,O2O_Ypos,'X',0xc0,0xc2);
     gfuTest_O2O_Cmd(O2O_Xpos+20+20,O2O_Ypos,'Y',0xc0,0xc3);
     gfuTest_O2O_Cmd(O2O_Xpos+20+20+20,O2O_Ypos,'Z',0xc0,0xc4);

     prevO2O_Xpos=O2O_Xpos;
     prevO2O_Ypos=O2O_Ypos;
  #endif
     //-------------------------------//
      
     testcnt ++;
     return 1;     
}
#endif
