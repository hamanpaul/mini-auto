#ifndef __IR_PPM_H__
#define __IR_PPM_H__
      #define ENABLE     1
      #define DISABLE   0
      #define FAIL          0

#define HIMAX_ISP_PLATFORM          1
#define IR_REPEATE_ENABLE              0
//#define IR_REMOTE_TYPE              5   /*1: 8 Keys, 2: 18 keys 3:21 keys, 4:eclipse 5: RDI*/
/**************************************************************************
 *  Resurace ussage notes:                                                *
 *    1. Requires 1 timer at 10us resolution (should < 200us)             *
 *    2. GPIO support both postive and negative edge intrrupt           *
 **************************************************************************/
      #define IR_TIMER_ID			1 /* irTimerID: 指定Timer-1 */
      //#define IR_REMOTE_TYPE		4 /* Lucian : 直接指定*/
      //Lucian: 設定IR要使用的GPIO0-X
      #define IR_GPIOGROUPSEL   0   //Use GPIO0




     typedef u32 UINT32;
      typedef void (*IR_ISR_FP)(void);
      
#if (IR_REMOTE_TYPE == 1)
      #define VK_POWER      0x12
      #define VK_MODE         0x1e
      #define VK_UP              0x04
      #define VK_DOWN        0x06
      #define VK_ENTER        0x05
      #define VK_MENU         0x1b
      #define VK_RIGHT        0x1f
      #define VK_LEFT           0x0a
#elif(IR_REMOTE_TYPE == 2)
    #define VK_UP       0x09
    #define VK_DOWN     0x06
    #define VK_RIGHT    0x50
    #define VK_LEFT     0x41
    #define VK_ENTER    0x42
    #define VK_MODE     0x1c
    #define VK_MENU     0x17
    #define VK_DELETE   0x1f
    #define VK_PLAY     0x49
    #define VK_STOP     0x05
#elif(IR_REMOTE_TYPE == 3)

	#if (HW_BOARD_OPTION == RDI_DOORPHONE_774)
    #define VK_UP			0x06
    #define VK_DOWN			0x44
    #define VK_RIGHT    	0x40
    #define VK_LEFT			0x47
    #define VK_OK_REC		0x07

    #define VK_REC_ALARM	0x5E
    #define VK_PLAY_PAUSE	0x05
    #define VK_STOP			0x03
    #define VK_MODE			0x02
    #define VK_DELETE		0x18
	#define VK_MENU			0x0F
	#define VK_ESC			0x51
	#define VK_SOURCE		0x17
	#else
    #define VK_UP       0x05
    #define VK_DOWN     0x1b
    #define VK_RIGHT    0x09
    #define VK_LEFT     0x07
    #define VK_ENTER    0x08
    #define VK_MODE     0x03
    #define VK_MENU     0x06
    #define VK_DELETE   0x01
    #define VK_PLAY     0x0d
    #define VK_STOP     0x0e
	#endif

#elif(IR_REMOTE_TYPE == 4)
    #define VK_UP       0x9c
    #define VK_DOWN     0xc0
    #define VK_RIGHT    0x92
    #define VK_LEFT     0x9a
    #define VK_ENTER    0x93
    #define VK_MODE     0x90
    #define VK_MENU     0x8f
    #define VK_DELETE   0x87
    #define VK_PLAY     0x85
    #define VK_STOP     0xc1
    #define VK_CAPIMG   0x86
#elif(IR_REMOTE_TYPE == 5)
    #define VK_UP       0x19
	#define VK_DOWN     0x11
	#define VK_RIGHT    0x16
	#define VK_LEFT     0x14
	#define VK_ENTER    0x15
	#define VK_MODE     0x18
	#define VK_MENU     0x12
	#define VK_DELETE   0x10
	#define VK_PLAY     0x05
    #define VK_STOP     0x17
	#define VK_CAPE     0x1A
	#define VK_REC      0x1B
#elif(IR_REMOTE_TYPE == 6)	
	#define VK_UP       0x19
    #define VK_DOWN     0x05
    #define VK_RIGHT    0x03
    #define VK_LEFT     0x01
    #define VK_ENTER    0x12
    #define VK_MODE     0x0E
    #define VK_MENU     0x1E
    #define VK_DELETE   0x1C
    #define VK_PLAY     0x02
    #define VK_STOP     0x0F
#elif(IR_REMOTE_TYPE == 7)
    #define VK_UP       0x0a
    #define VK_DOWN     0x0d
    #define VK_RIGHT    0x09
    #define VK_LEFT     0x0c
    #define VK_ENTER    0x15
    #define VK_MODE     0x00
    #define VK_MENU     0x0e
    #define VK_DELETE   0x01
    #define VK_PLAY     0x16
    #define VK_STOP     0x14
    #define VK_CAPIMG   0x06
#elif(IR_REMOTE_TYPE == 8)

#if 0
    #define VK_UP       0x9c
    #define VK_DOWN     0xc0
    #define VK_RIGHT    0x92
    #define VK_LEFT     0x9a
    #define VK_ENTER    0x93
    #define VK_MODE     0x90
    #define VK_MENU     0x8f
    #define VK_DELETE   0x87
    #define VK_PLAY     0x85
    #define VK_STOP     0xc1
    #define VK_CAPIMG   0x86
#else
    #define VK_UP       0x01
    #define VK_DOWN     0x09
    #define VK_RIGHT    0x06
    #define VK_LEFT     0x04
    #define VK_ENTER    0x05
	#define VK_ZOOMIN	0x10
    #define VK_ZOOMOUT  0x12
    #define VK_MENU     0x0D
    #define VK_REC   	0x0C
    #define VK_PLAY     0x11
    #define VK_STOP     0x0E
 #endif   
#elif(IR_REMOTE_TYPE == 9)
    #define VK_UP       0x03
    #define VK_DOWN     0x09
    #define VK_RIGHT    0x0d
    #define VK_LEFT     0x05
    #define VK_OK       0x01
    #define VK_MANUAL	0x06
    #define VK_DELETE   0x02
    #define VK_MENU     0x0f
    #define VK_POWER    0x0a
    #define VK_SCHEDULE 0x0e
    #define VK_MOTION   0x0b
#elif(IR_REMOTE_TYPE == 10) 
//for winnin 
    #define VK_UP       0x05
    #define VK_DOWN     0x0D
    #define VK_RIGHT    0x0A
    #define VK_LEFT     0x08
    #define VK_ENTER    0x09
    #define VK_MODE     0x02
    #define VK_MENU     0x06
    #define VK_DELETE   0x00
    #define VK_PLAY     0x11
    #define VK_STOP     0x12
/*   #define VK_CAPIMG   0x01 */
    #define VK_KEY_CHANNEL 0x01
#elif(IR_REMOTE_TYPE == 11) 
    #define VK_UP       0x07
    #define VK_DOWN     0x0f
    #define VK_RIGHT    0x0a
    #define VK_LEFT     0x08
    #define VK_ENTER    0x0b
    #define VK_MODE     0x00
    #define VK_MENU     0x02
    #define VK_DELETE   0x0e
    #define VK_PLAY     0x05
    #define VK_STOP     0x09
    #define VK_ESC      0x03
    #define VK_PAUSE    0x04
    #define VK_REC      0x06
    #define VK_REW      0x0d
    #define VK_FF       0x0c
    #define VK_CAPIMG   0x86    /*not support*/
#elif(IR_REMOTE_TYPE == 12) 
    #if(IR_REMOTE_CONTROL == 1)
        #define VK_UP       0x12
        #define VK_DOWN     0x1d
        #define VK_PLAY     0x14
        #define VK_REC      0x18
    #else
        #define VK_UP       0x41
        #define VK_DOWN     0x50
        #define VK_PLAY     0x09
        #define VK_REC      0x0a
    #endif
#elif(IR_REMOTE_TYPE == 13) 
    #define VK_DEL      0x01
    #define VK_CARD     0x03
    #define VK_PLAY     0x07        
    #define VK_RIGHT    0x08
    #define VK_LEFT     0x09
    #define VK_OK       0x0a
    #define VK_DOWN     0x0b
    #define VK_UP       0x0c
    #define VK_STOP     0x0d
    #define VK_MENU     0x0e
    #define VK_MUTE     0x0f
    #define VK_REC      0x11
    #define VK_FF       0x12
    #define VK_CH       0x14    
    #define VK_MODE     0x15
    #define VK_RF       0x18   
    #define VK_PRE      0x19 
    #define VK_TV_SET   0x02
#elif(IR_REMOTE_TYPE == 14)
	/* Skybest */
    #define VK_LCD		0x00
    #define VK_OK		0x01
    #define VK_UP		0x02
    #define VK_DOWN		0x03
    #define VK_LEFT		0x04
    #define VK_RIGHT	0x05
    #define VK_PLAY		0x08
    #define VK_REC		0x09
    #define VK_STOP		0x10 
    #define VK_PWR		0x14
    #define VK_DEL		0x15
    #define VK_DVR_TV	0x16 
    #define VK_MENU		0x17
#elif(IR_REMOTE_TYPE == 15)
    #define VK_UP       0x08
    #define VK_DOWN     0x0d
    #define VK_RIGHT    0x1f
    #define VK_LEFT     0x0a    
    #define VK_ENTER    0x1b
    #define VK_MENU     0x04
    
    #define VK_MODE     0x12 //rec
    #define VK_DELETE   0x01 
    #define VK_PLAY     0x03 //sch
    #define VK_STOP     0x06 //MD
    #define VK_REC      0x1e //REC
#elif(IR_REMOTE_TYPE == 16)   /*for 夏門奧亞 */
    #define VK_UP       0x05
    #define VK_DOWN     0x0d
    #define VK_RIGHT    0x0a
    #define VK_LEFT     0x08    
    #define VK_ENTER    0x09
    #define VK_MENU     0x06
    
    #define VK_MODE     0x02 
    #define VK_DELETE   0x00 
    #define VK_PLAY     0x11 
    #define VK_STOP     0x12 
    #define VK_POWER    0x6f	
#elif(IR_REMOTE_TYPE == 17)   /*OPCOM */
    #define VK_UP       0x15
    #define VK_DOWN     0x18
    #define VK_RIGHT    0x0d
    #define VK_LEFT     0x16    
    #define VK_ENTER    0x19
    #define VK_MENU     0x08
    #define VK_PLAY     0x44 
    #define VK_DEL      0x43 
    #define VK_ESC      0x5a 
    #define VK_POWER    0x47 
    #define VK_REC      0x1c 
#elif (IR_REMOTE_TYPE == 18)    /* for ACT611 */
    #define VK_UP       0x13
    #define VK_DOWN     0x0f
    #define VK_RIGHT    0x16
    #define VK_LEFT     0x17    
    #define VK_ENTER    0x10
    #define VK_MENU     0x00
    #define VK_PLAY     0x0e 
    #define VK_ESC      0x06 
    #define VK_STOP     0x12 
    #define VK_REC      0x04
#elif (IR_REMOTE_TYPE == 19)    /* for KD */
    #define VK_UP       0x0C
    #define VK_DOWN     0x0B
    #define VK_RIGHT    0x08
    #define VK_LEFT     0x09    
    #define VK_ENTER    0x0A
    #define VK_MENU     0x10
    #define VK_MODE     0x0D 
    #define VK_DELETE   0x03 
    #define VK_PLAY     0x01 
    #define VK_STOP     0x12 
#elif (IR_REMOTE_TYPE == 20)    /* for 鑫立捷 */
    #define VK_UP       0x21
    #define VK_DOWN     0x27
    #define VK_RIGHT    0x25
    #define VK_LEFT     0x23    
    #define VK_ENTER    0x24
    #define VK_MENU     0x40
#endif




#if (HW_BOARD_OPTION == MR6730_AFN)

typedef enum
{
	IrKeyId_Begin=0,
	//-----
	// 0
	IrKeyId_Up=IrKeyId_Begin,
	IrKeyId_Down,
	IrKeyId_Left,
	IrKeyId_Right,
	IrKeyId_Ok,
	IrKeyId_Menu,
	IrKeyId_Del,
	IrKeyId_Esc,
	IrKeyId_Play,
	IrKeyId_Stop,
	IrKeyId_Pwr,
	//-----
	IrKeyId_End,//Dummy
	IrKeyId_Num=IrKeyId_End,
}IrKeyId;

//extern u8 gIrKeyCodeTbl[];
extern u8* gIrKeyCodeTbl;


typedef enum
{
	IRKC_TYPE_BEGIN=0,
	//-----
	// 0
	IRKC_TYPE_A=IRKC_TYPE_BEGIN,
	#if (!EXT_IR_CODE)
	//#if(DERIVATIVE_MODEL==MODEL_TYPE_PUSH)	
	#if( (DERIVATIVE_MODEL==MODEL_TYPE_PUSH)||((DERIVATIVE_MODEL==MODEL_TYPE_YD)&&(IR_CODE_TBL_SEL==1))	)
	IRKC_TYPE_B,
	#endif
	#endif//#if (!EXT_IR_CODE)
	//-----
	IRKC_TYPE_END,//Dummy
	IRKC_TYPE_NUM=IRKC_TYPE_END,
}tIRKC_TYPE;

//extern const u8 IrKeyCode_Type_A[];
//extern const u8 IrKeyCode_Type_B[];
extern const u8 IrKeyCode_Types[IRKC_TYPE_NUM][IrKeyId_Num];




extern void IrKeyCodeTbl_Init(u8 TypeSel);






#if (EXT_IR_CODE)	
//load IR_CODE from RES	


#define IR_CODE_DAT_MAX						16 


#if (IrKeyId_Num>IR_CODE_DAT_MAX-2)	// 2bytes custom code
#error "IrKeyId_Num >(IR_CODE_DAT_MAX-2)"
#endif


typedef union _IrCodeDat{
	u8 _total[IR_CODE_DAT_MAX];
	//
	__packed struct {
		u16 CustomCode;
		u8 KeyCode[IrKeyId_Num];	
		//...
	}s;
}tIrCodeDat;

extern tIrCodeDat gIrCodeDat;

#define IrCodeData							gIrCodeDat.s

extern void hwIrInit_Ex(void* pParm);

#endif	//#if (EXT_IR_CODE)	
#endif//#if(HW_BOARD_OPTION == MR6730_AFN)

#endif

