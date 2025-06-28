#ifndef __GFU_API_H__
   #define __GFU_API_H__

   #define GFU_ALPHA_100P        0
   #define GFU_ALPHA_75P         1
   #define GFU_ALPHA_50P         2
   #define GFU_ALPHA_25P         3
   #define GFU_ALPHA_00P         4

   //============================================================//
   extern int gfuInit(void);
   extern int gfuReset(void);

   extern int gfuTest_BCP_Cmd(  int Src1_X,int Src1_Y,
                                         int Src1_W,int Src1_H,
                                         int Dst_X,int Dst_Y);

   extern int gfuTest_Rect_Cmd(  int Dst_X,int Dst_Y,int Dst_W,int Dst_H,unsigned int color);
   extern int gfuTest_O2V_Cmd(  int Src1_X,int Src1_Y,char ch,u32 IndexColor);

   extern int gfuTest_V2V_Cmd(  int Src1_X,int Src1_Y,int Src1_W,int Src1_H,
                                        int Src2_X,int Src2_Y,
                                        int Dst_X,int Dst_Y,
                                        int Alpha
                                      );

   extern int gfuTest_O2O_Cmd(  int Src1_X,int Src1_Y,char ch,u32 OldColor, u32 NewColor);

   extern int gfuTest_ALL_Cmd(void);



#endif
