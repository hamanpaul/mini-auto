
#ifndef __MD_API_H__
#define __MD_API_H__


#define MD_SIU_ID    0
#define MD_CIU1_ID   1
#define MD_CIU2_ID   2
#define MD_CIU3_ID   3
#define MD_CIU4_ID   4
#define MC_CH_MAX    5


#if HW_MD_SUPPORT

  #define MD_DRAW_RECT_MAX               8
  
  #define MD_MODE_SHORT_DIFF             0
  #define MD_MODE_LONG_DIFF              1

  #if (NEW_VMDSW_TEST || NEW_VMDSW)
  #define MD_WIN_WIDTH_MAX               640
  #define MD_WIN_HEIGHT_MAX              480
  #define MD_BLOCK_WIDTH                 16
  #define MD_BLOCK_HEIGHT                16
  #else
  #define MD_WIN_WIDTH_MAX               704
  #define MD_WIN_HEIGHT_MAX              576
  #define MD_BLOCK_WIDTH                 32
  #define MD_BLOCK_HEIGHT                32
  #endif

  
  //------Motion Detect use--------//
  #define MOTION_DETECT_SPEED_HIGH    0
  #define MOTION_DETECT_SPEED_NORMAL  1
  #define MOTION_DETECT_SPEED_LOW     2

  typedef  struct
  {
      u16 X_Start;
      u16 X_End;
      u16 Y_Start;
      u16 Y_End;
  } DEF_MD_RECT_POS;

  #define MD_BLOCK_NUM_MAX              ((MD_WIN_WIDTH_MAX/MD_BLOCK_WIDTH) * (MD_WIN_HEIGHT_MAX/MD_BLOCK_HEIGHT))

extern void mduMotionDetect_Sensitivity_Config(u8);
extern void mduMotionDetect_NoiseMargin_Config(u8 );
extern void mduMotionDetect_Velocity_Config(u8);
extern void mduMotionDetect_Mask_Config(u8 *);
extern void mduMotionDetect_ONOFF(u8);
extern void MotionDetect_API(int);

extern void mdu_TestSensitivity(char *cmd);

extern  u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2];
extern  u8 MD_SensitivityConfTab_New[MD_SENSITIVITY_LEVEL][2];
    #if (HW_BOARD_OPTION == MR8200_RX_RDI_M706 && PROJ_OPT == 8)
    extern  u8 MD_SensitivityConfTab_CL692_150803[MD_SENSITIVITY_LEVEL][2];
    #endif
#else  
extern  u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2];
extern  u8 MD_SensitivityConfTab_New[MD_SENSITIVITY_LEVEL][2];
#endif

#endif
