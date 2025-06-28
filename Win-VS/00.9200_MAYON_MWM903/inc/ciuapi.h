

#ifndef __CIU_API_H__
#define __CIU_API_H__

/*
 *********************************************************************************************************
 *  SYS Constant
 *********************************************************************************************************
 */

 #define CIU_OSD_OFF  0
 #define CIU_OSD_ON   1

 #define CIU_MODE_YUV             0x00000000
 #define CIU_MODE_656             0x00000010
 #define CIU_MODE_601             0x00000020
 #define CIU_MODE_656_BOB         0x00000030

 #define CIU_SCA_SHAREBUF_DIS     0x00000000
 #define CIU_SCA_SHAREBUF_EN      0x00200000


 #define CIU_EVET_FREAME_END      0x00000001
 #define CIU_EVET_TOPFILD_END     0x00000002
/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
extern OS_FLAG_GRP  *ciuFlagGrp_CH1;
extern OS_FLAG_GRP  *ciuFlagGrp_CH2;
extern OS_FLAG_GRP  *ciuFlagGrp_CH3;
extern OS_FLAG_GRP  *ciuFlagGrp_CH4;
extern OS_FLAG_GRP  *ciuFlagGrp_CH5;



extern OS_EVENT* ciuCapSemEvt_CH1;     // for Video capture
extern OS_EVENT* ciuCapSemEvt_CH2;     // for Video capture
extern OS_EVENT* ciuCapSemEvt_CH3;     // for Video capture
extern OS_EVENT* ciuCapSemEvt_CH4;     // for Video capture
extern OS_EVENT* ciuCapSemEvt_CH5;     // for Video capture


extern OS_STK ciuTaskStack_CH1[CIU_TASK_STACK_SIZE_CH1]; /* Stack of task ciuTask() */
extern OS_STK ciuTaskStack_CH2[CIU_TASK_STACK_SIZE_CH2]; /* Stack of task ciuTask() */
extern OS_STK ciuTaskStack_CH3[CIU_TASK_STACK_SIZE_CH3]; /* Stack of task ciuTask() */
extern OS_STK ciuTaskStack_CH4[CIU_TASK_STACK_SIZE_CH4]; /* Stack of task ciuTask() */
extern OS_STK ciuTaskStack_CH5[CIU_TASK_STACK_SIZE_CH5]; /* Stack of task ciuTask() */

extern u32 ciu_idufrmcnt_ch1;
extern u32 ciu_idufrmcnt_ch2;
extern u32 ciu_idufrmcnt_ch3;
extern u32 ciu_idufrmcnt_ch4;
extern u32 ciu_idufrmcnt_ch5;


extern u8 ciu_1_OpMode;
extern u8 ciu_2_OpMode;
extern u8 ciu_3_OpMode;
extern u8 ciu_4_OpMode;
extern u8 ciu_5_OpMode;


extern u32 ciu_1_FrameTime;
extern u32 ciu_2_FrameTime;
extern u32 ciu_3_FrameTime;
extern u32 ciu_4_FrameTime;
extern u32 ciu_5_FrameTime;


extern u32 ciu_1_OutX;
extern u32 ciu_2_OutX;
extern u32 ciu_3_OutX;
extern u32 ciu_4_OutX;
extern u32 ciu_5_OutX;


extern u32 ciu_1_OutY;
extern u32 ciu_2_OutY;
extern u32 ciu_3_OutY;
extern u32 ciu_4_OutY;
extern u32 ciu_5_OutY;


extern u32 ciu_1_line_stride;
extern u32 ciu_2_line_stride;
extern u32 ciu_3_line_stride;
extern u32 ciu_4_line_stride;
extern u32 ciu_5_line_stride;


extern u32 ciu_1_OutWidth;
extern u32 ciu_2_OutWidth;
extern u32 ciu_3_OutWidth;
extern u32 ciu_4_OutWidth;
extern u32 ciu_5_OutWidth;


extern u32 ciu_1_OutHeight;
extern u32 ciu_2_OutHeight;
extern u32 ciu_3_OutHeight;
extern u32 ciu_4_OutHeight;
extern u32 ciu_5_OutHeight;


extern u32 ciu_1_pnbuf_size_y;
extern u32 ciu_2_pnbuf_size_y;
extern u32 ciu_3_pnbuf_size_y;
extern u32 ciu_4_pnbuf_size_y;

extern u32 ciu_1_PIP_OutWidth;
extern u32 ciu_2_PIP_OutWidth;
extern u32 ciu_3_PIP_OutWidth;
extern u32 ciu_4_PIP_OutWidth;
extern u32 ciu_5_PIP_OutWidth;


extern u32 ciu_1_PIP_OutHeight;
extern u32 ciu_2_PIP_OutHeight;
extern u32 ciu_3_PIP_OutHeight;
extern u32 ciu_4_PIP_OutHeight;
extern u32 ciu_5_PIP_OutHeight;


extern u32 ciu_1_PIP_OutX;
extern u32 ciu_2_PIP_OutX;
extern u32 ciu_3_PIP_OutX;
extern u32 ciu_4_PIP_OutX;
extern u32 ciu_5_PIP_OutX;


extern u32 ciu_1_PIP_OutY;
extern u32 ciu_2_PIP_OutY;
extern u32 ciu_3_PIP_OutY;
extern u32 ciu_4_PIP_OutY;
extern u32 ciu_5_PIP_OutY;


extern u32 ciu_1_FPS_Count;



/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */ 

extern s32 CiuInit(void);

extern void ciu_1_Stop(void);
extern void ciu_2_Stop(void);
extern void ciu_3_Stop(void);
extern void ciu_4_Stop(void);
extern void ciu_5_Stop(void);


extern s32 ciuPreviewInit_CH1(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
extern s32 ciuPreviewInit_CH2(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
extern s32 ciuPreviewInit_CH3(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
extern s32 ciuPreviewInit_CH4(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
extern s32 ciuPreviewInit_CH5(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);


extern void ciuPreviewInMenu_CH1(u32 InWidth, u32 InHeight, u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride);

extern s32 ciuCaptureVideo_CH1(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride);
extern s32 ciuCaptureVideo_CH2(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride);
extern s32 ciuCaptureVideo_CH3(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride);
extern s32 ciuCaptureVideo_CH4(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride);


extern void ciuIntHandler_CH1(void);
extern void ciuIntHandler_CH2(void);
extern void ciuIntHandler_CH3(void);
extern void ciuIntHandler_CH4(void);
extern void ciuIntHandler_CH5(void);


extern void mdIntHandler(void);

extern int ciu1_ChangeInputSize(int InWidth,int InHeight);
extern int ciu2_ChangeInputSize(int InWidth,int InHeight);
extern int ciu3_ChangeInputSize(int InWidth,int InHeight);
extern int ciu4_ChangeInputSize(int InWidth,int InHeight);


extern s32 GenerateCIU1_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot, 
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH, 
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);

extern s32 GenerateCIU2_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot, 
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH, 
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);

extern s32 GenerateCIU3_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot, 
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH, 
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);

extern s32 GenerateCIU4_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot, 
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH, 
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);

extern s32 GenerateCIU1_SP_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot, 
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH, 
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);

extern s32 ciu1ScUpZoom(s32 zoomFactor);
extern s32 ciu2ScUpZoom(s32 zoomFactor);
extern s32 ciu3ScUpZoom(s32 zoomFactor);
extern s32 ciu4ScUpZoom(s32 zoomFactor);
extern s32 ciu5ScUpZoom(s32 zoomFactor);

extern s32 rfiuciu1ZoomInx2(int OnOff,int Xpos,int Ypos);
extern s32 rfiuciu2ZoomInx2(int OnOff,int Xpos,int Ypos);

#if (HW_DEINTERLACE_CIU1_ENA || HW_DEINTERLACE_CIU2_ENA || HW_DEINTERLACE_CIU3_ENA || HW_DEINTERLACE_CIU4_ENA || HW_DEINTERLACE_CIU5_ENA)
extern int diuRegConfig(int ID, u32 Width,u32 Height,u32 Stride);
extern void diuIntHandler(void);
#endif

extern u32 ciuCheckFrameRate(void);


// for timestamp position in the first frame is correct, especially battery cam 
#if (HW_BOARD_OPTION== MR9160_TX_DB_BATCAM)
 #define ciu_loc_x_vga 2
 #define ciu_loc_y_vga 20
 #define ciu_loc_x_hd  2
 #define ciu_loc_y_hd  28
 #define ciu_loc_x_fhd 2
 #define ciu_loc_y_fhd 27
 #define ciu_line_num  2
 #define ciu_line1_str_num 16

#elif (HW_BOARD_OPTION == MR9160_TX_OPCOM_BATCAM) //this set for timestamp on top-right always ENABLE
 #define ciu_loc_x_vga 1
 #define ciu_loc_y_vga 1
 #define ciu_loc_x_hd  1
 #define ciu_loc_y_hd  1
 #define ciu_loc_x_fhd 1
 #define ciu_loc_y_fhd 1
 #define ciu_line_num  1
 #define ciu_line1_str_num 32
#elif ((HW_BOARD_OPTION == MR9160_TX_MAYON_MWL613))
 #define ciu_loc_x_vga 30
 #define ciu_loc_y_vga 20
 #define ciu_loc_x_hd 20 // 20x24
 #define ciu_loc_y_hd 28 // 20x24
//    u8  loc_x_hd=17; // 24x28
//    u8  loc_y_hd=24; // 24x28
//    u8  loc_x_hd=13; // 28x32
//    u8  loc_y_hd=21; // 28x32
 #define ciu_loc_x_fhd 19
 #define ciu_loc_y_fhd 28
 #define ciu_line_num 1
 #define ciu_line1_str_num 24
#elif ((HW_BOARD_OPTION == MR9160_TX_ROULE_BATCAM))
#define ciu_loc_x_vga 2
#define ciu_loc_y_vga 27
#define ciu_loc_x_hd  2
#define ciu_loc_y_hd  3
#define ciu_loc_x_fhd 2
#define ciu_loc_y_fhd 4
#define ciu_line_num  1
#define ciu_line1_str_num 24  //第一行字串個數
#else
 #define ciu_loc_x_vga 2
 #define ciu_loc_y_vga 27
 #define ciu_loc_x_hd  2
 #define ciu_loc_y_hd  28
 #define ciu_loc_x_fhd 2
 #define ciu_loc_y_fhd 2
 #define ciu_line_num  1
 #define ciu_line1_str_num 24  //第一行字串個數
#endif

#endif
