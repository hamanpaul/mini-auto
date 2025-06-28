#include "general.h"
#include "sysapi.h"
#include "smcapi.h"
#include "isuapi.h"
#include "mmuapi.h"
#include "siuapi.h"
#include "./fs/fs_api.h"
#include "iduapi.h"
#include "iisapi.h"
#include "rfiuapi.h"
#include "spiapi.h"
#include "mpeg4api.h"
#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
#include "ima_adpcm_api.h"
#endif
#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
#if (HW_BOARD_OPTION == MR6730_AFN)
#include "..\ui\inc\MainFlow.h"
#endif

 u8 *CaptureStartAdr;

 u8 *siuRawBuf;

 u8 *PKBuf;
 u8 *PKBuf0;
 u8 *PKBuf1;
 u8 *PKBuf2;
#if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) || (SW_APPLICATION_OPTION  == DVP_RF_SELFTEST) )
  #if MENU_DONOT_SHARE_BUFFER
     u8 *uiMenuBuf1;
     u8 *uiMenuBuf2;
     u8 *uiMenuBuf3;
  #endif
#endif

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
 u8 *rfiuOperBuf[MAX_RFIU_UNIT];
 u8 *rfiuRxVideoBuf[MAX_RFIU_UNIT];
 u8 *rfiuRxVideoBufEnd[MAX_RFIU_UNIT];
 
 u8 *rfiuRxAudioBuf[MAX_RFIU_UNIT];
 u8 *rfiuRxAudioBufEnd[MAX_RFIU_UNIT];

#if RX_SNAPSHOT_SUPPORT
 u8 *rfiuRxDataBuf[MAX_RFIU_UNIT];
 u8 *rfiuRxDataBufEnd[MAX_RFIU_UNIT];
#endif 
 
 DEF_RF_MP4DEC_BUF rfiuRxDecBuf[MAX_RFIU_UNIT];
 DEF_RFIU_RXCMD_STATUS rfiuCmdRxStatus[16];

 u32 rfiuMainVideoPresentTime[DISPLAY_BUF_NUM];
 u32 rfiuSub1VideoPresentTime[DISPLAY_BUF_NUM];

 u32 rfiuVideoTimeBase[MAX_RFIU_UNIT];

 u32 rfiuMainAudioPresentTime[IISPLAY_BUF_NUM];

 u32 rfiuMainVideoTime;
 u32 rfiuMainVideoTime_frac;
 u32 rfiuRxMainVideoPlayStart;

 u32 rfiuSub1VideoTime;
 u32 rfiuSub1VideoTime_frac;
 u32 rfiuRxSub1VideoPlayStart;

 u32 rfiuMainAudioTime;
 u32 rfiuAudioTimeBase;
 u32 rfiuMainAudioTime_frac;
 u32 rfiuRxMainAudioPlayStart;

 int rfiuVideoInFrameRate;
 int rfiuTXBitRate[2];
 int rfiuTxBufFullness[2];

 u32 rfiuRX_OpMode;
 u32 rfiuRX_P2pVideoQuality;
 u32 rfiuRX_CamOnOff_Num=4;

 s32 rfiu_TX_P2pVideoQuality = 1; //RFWIFI_P2P_QUALITY_MEDI
 s32 rfiu_TX_WifiPower;
 s32 rfiu_TX_WifiCHNum;

 s32 M7688_WifiPower;
 s32 M7688_WifiCHNum;


 u32 rfiuRXWrapSyncErrCnt[MAX_RFIU_UNIT];


#if( RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2)
    u32 rfiuRX_CamOnOff_Sta=0x01;
#elif(RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1)
    u32 rfiuRX_CamOnOff_Sta=0x03;
#elif(RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_4x_RX_PROTOCOL_B1)
    u32 rfiuRX_CamOnOff_Sta=0x0f;
#else
  u32 rfiuRX_CamOnOff_Sta=0x0f;
#endif

 u32 rfiuRX_CamPerRF;
 u32 rfiuTxPIR_Trig=0;

#endif

 u32 rfiuVideoBufFill_idx[MAX_RFIU_UNIT];
 u32 rfiuVideoBufPlay_idx[MAX_RFIU_UNIT];

#if MULTI_CHANNEL_SUPPORT
 u8 *PNBuf_sub1[4];  //for CIU1
 u8 *PNBuf_sub2[4];  //For CIU2
 u8 *PNBuf_sub3[4];  //For CIU3
 u8 *PNBuf_sub4[4];  //For CIU4
 //#if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
 u8 *PNBuf_Quad;
 //#endif
#endif

 unsigned char *smcMBRCache;
 unsigned char *smcGeneralBuf;


 u8* smcGeneralBuf;
 u8 *ipuDstBuf0;
 u8 *ipuDstBuf1;
 u8 *ipuDstBuf2;

 u8 *MainVideodisplaybuf[DISPLAY_BUF_NUM];
 u32 MainVideodisplaybuf_idx;

 u8 *Sub1Videodisplaybuf[DISPLAY_BUF_NUM];
 u32 Sub1Videodisplaybuf_idx;


 u8 *Jpeg_displaybuf[3];
 u8 *mpeg4MVBuf;

 u8 *mpeg4PRefBuf_Y;
 u8 *mpeg4PRefBuf_Cb;
 u8 *mpeg4PRefBuf_Cr;
 u8 *mpeg4NRefBuf_Y;
 u8 *mpeg4NRefBuf_Cb;
 u8 *mpeg4NRefBuf_Cr;

 #if(VIDEO_CODEC_OPTION == H264_CODEC)
 u8 *H264MBPredBuf;
 u8 *H264ILFPredBuf;
 u8 *H264IntraPredBuf ;
 #endif

 u8 *P2PPBVideoBuf;
 u8 *P2PPBVideBufEnd; //Toby
 int P2PSentByteCnt;
 int P2PTxBitRate[1];
 int P2PTxBufFullness[1];

 u8 *PNBuf_Y0, *PNBuf_C0, *PNBuf_Y1, *PNBuf_C1, *PNBuf_Y2, *PNBuf_C2; /*Lucian 070531*/
 u8 *PNBuf_Y3, *PNBuf_C3;
 u8 *PNBuf_Y[4], *PNBuf_C[4];
 u8* mpeg4outputbuf[3];
 u8* VideoBuf;
 u8* mpeg4VideBufEnd;
 u8* mpeg4IndexBuf;
 #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
 u8 *Web_page;
 #endif
#if (CDVR_LOG || CDVR_TEST_LOG)
 u8* LogFileBuf;
 u8* LogFileBufEnd;
#endif

#if(USB2WIFI_SUPPORT || USB_DEVICE)
u8*  usb_device_buf;
#endif
#if USB2WIFI_SUPPORT
u8*  usbfwupgrade_buf;
u8*  usb_AV_buf;
u8*  usb_AV_buf_end;
#endif
#if (USB_HOST == 1)
u8*  usb_qh_buf_1;
u8*  usb_qh_buf_2;
u8*  usb_qh_buf_3;
u8*	 usb_qtd_buf;
u8*  usb_itd_buf_1;
u32* usb_Page_buf_0;

#endif

#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
 u8* ImaAdpcmBuf;
 u8* ImaAdpcmBufEnd;
#endif

#ifdef OPCOM_JPEG_GUI
 u8* gpGUIJPGFileBuf;
#endif
#if ( (HW_BOARD_OPTION==ROULE_SD8F)||(HW_BOARD_OPTION == ROULE_SD7N))
 u8* gpGUIJPGFileBuf;
#endif
 u8 *exifDecBuf;
 u8 *exifThumbnailBitstream;
 u8 *exifPrimaryBitstream;
#if ADDAPP3TOJPEG
 u8 *exifAPP3VGABitstream;
#endif
#if ADDAPP2TOJPEG
 DEF_APPENDIXINFO *exifApp2Data;
#endif

//added by Albert Lee on 20100119
#if(HW_BOARD_OPTION == ES_LIGHTING)
u8 *WaveCardFailBuf;			//size: 0x14000
u8 *WavekeySuccessBUF;			//size: 0x14000
u8 *WavekeyFailBUF;			    //size: 0x14000
u8 *WavekeyProcessBUF;			//size: 0x3B000
u8 *WavePIRBUF;			        //size: 0x14000
#endif


 u8 *sdcReadBuf;
 u8 *sdcWriteBuf;

 u8 *dcfBuf;

#if CDVR_iHome_LOG_SUPPORT
 u8 *dcfLogBuf_Wr;
 u8 *dcfLogBuf_Rd;
#endif

 
#if SDC_ECC_DETECT
 u8 *FatDummyBuf;
#endif
extern  char *cMulBlockBuffer;


u8* OSD_buf;
#if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
u8* OSD_buf1; // A1018B mouse use Amon (140718)
u32 OSD_buf1_Size;
#endif

u8* SPIConfigBuf;
u8* iduvideobuff;
u8* iduscalerbuff[NUMS_PHOTOS_PREVIEW];
 u8 *iisSounBuf[IIS_BUF_NUM];

u8 *iotcBuf; /*for ALL IOTCAPIs use*/
#if 1
u8 *icommbuf;
u8 *icommbuf2;
u8 *icommbuf3;
__align(4) u8 *icommbuf4;
#endif
u8 * iCommAudioRetBuf[RFI_AUDIO_RET_BUF_NUM];

#if IIS_TEST
  u8 *iisBuf_play;
#endif
 /*----------------------Declare Global Variable for Control---------------------*/
 #if GET_SIU_RAWDATA_PURE
   s16 siuAdjustSW;
   s16 siuAdjustAGC;
 #endif

 u8 isuStatus_OnRunning;

 s8 sysCameraMode=SYS_CAMERA_MODE_UNKNOWN;
 u8 sysInsertAPP3_ena; //Lucian: 決定是否加入APP3 to JPEG file
 u8 sysPowerOffFlag;
 u8 sysTVOutOnFlag;
 u8 sysVoiceRecStart;
 u8 sysVoiceRecStop;
 u8 sysVoicePlayStop;
 u8 sysVolumnControl;
 u8 sysIsAC97Playing;
 u32 sysPlayBeepFlag;
#if MULTI_CHANNEL_SUPPORT
 u8 sysVideoInCHsel;
 u8 sysDualModeDisp;
#endif
#if HWPIP_SUPPORT
u8 sysOsdInCHsel;
#endif

 u8 sysRFRxInMainCHsel;
 u8 sysRF_PTZ_CHsel;
 u8 sysRF_AudioChSw_DualMode;

 u8 sysPIPMain  = PIP_MAIN_NONE;

 OS_EVENT* isuSemEvt;
 u8	*pucClockImageBuf;
 RTC_DATE_TIME g_LocalTime;
 u32 g_LocalTimeInSec;

 //================== Scalar overlay image ================//
 u32 *ScalarOverlayImage;

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
 u32 *CiuOverlayImg1_Top;
 u32 *CiuOverlayImg1_Bot;

 u32 *CiuOverlayImg2_Top;
 u32 *CiuOverlayImg2_Bot;

 u32 *CiuOverlayImg3_Top;
 u32 *CiuOverlayImg3_Bot;

 u32 *CiuOverlayImg4_Top;
 u32 *CiuOverlayImg4_Bot;

 u32 *CiuOverlayImg1_SP_Top;
 u32 *CiuOverlayImg1_SP_Bot;
#endif

 u32 OS_tickcounter=0;

#ifdef MMU_SUPPORT
 unsigned int *mmu_tlb_base;
 unsigned int *mmu_L1_base;
 u8* FS_internal_mem;
 #endif

u16 nand_fat_start, nand_fat_end ,nand_fat_size,nand_data_start;

 #if TUTK_SUPPORT
u8* p2plocal_buffer;
//for event list use. by aher
u8* p2pEventList;
 #endif

#if defined(PIC_OP)
    u8* picTmpBuf;
#endif

#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
u32 gfuSwQueWrIdx;
u32 gfuSwQueRdIdx;
u8 *gfuSwQueAddr;
#endif

#if VMDSW
u8* VMDlog;
u8* VMDMeanBuf1;
u8* VMDMeanBuf2;
u8* VMDPositiveCnt_X;
u8* VMDNegativeCnt_X;
u8* VMDPositiveCnt_Y;
u8* VMDNegativeCnt_Y;

u8* VMDFilterPX;
u8* VMDFilterNX;
u8* VMDFilterPY;
u8* VMDFilterNY;

u8* VMDPositiveBuf_X[10];
u8* VMDNegativeBuf_X[10];
u8* VMDPositiveBuf_Y[10];
u8* VMDNegativeBuf_Y[10];

u8* VMDFlterMap;

// method 2
#if VMDSW
s8* VMDMotionPos_X;
s8* VMDMotionPos_Y;
#if 0
u8* VMDMotionBuf_X[10];
u8* VMDMotionBuf_Y[10];
#endif
#endif
#endif


#if TX_SNAPSHOT_SUPPORT
u8 *sysRFTXImgData;
u32 sysRFTXDataSize;
#endif

 /*====================
Routine Description:

    The initMemoryPool() function.

Arguments:

    addr- The memory pool start address.

Return Value:

    bottom address of memory pool.
=====================*/



u8 *initMemoryPool(u8 *addr)  //Lucian 070419
{
    int                 i, j;
    u8                  *Mpeg_addr;
#if MULTI_CHANNEL_VIDEO_REC
    VIDEO_CLIP_OPTION   *pVideoClipOption;
#endif
    u8                  *addr_temp;
    u8                  *Mpeg_addr_temp;

    //==========Global Variable Initialize==============//
    MainVideodisplaybuf_idx = 0;
    sysCaptureImageStart    = 0;
    sysCaptureVideoStart    = 0;
    sysCaptureVideoStop     = 1;
    sysPowerOffFlag         = 0;
#if (UI_BOOT_FROM_PANEL == 1)
    sysTVOutOnFlag          = 0;
#else
    sysTVOutOnFlag          = 1;
#endif
    sysUSBPlugInFlag        = 0;
    sysVoiceRecStart        = 0;
    sysVoiceRecStop         = 1;
    #if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
    sysVolumnControl        = VOL_CTRL_MAX;
	#else
    sysVolumnControl        = 10;
	#endif
    sysIsAC97Playing        = 0;
#if MULTI_CHANNEL_SUPPORT
    sysVideoInCHsel         = 0;
  #if MULTI_CHANNEL_SEL
    while( (MULTI_CHANNEL_SEL & (1<<sysVideoInCHsel))==0 )
    {
       sysVideoInCHsel = (sysVideoInCHsel+1) % MULTI_CHANNEL_MAX;
    }
  #endif
#endif
#if GET_SIU_RAWDATA_PURE
    siuAdjustSW             = 80;// 80:ligixbox,micro-lens@office
    siuAdjustAGC            = 8;
#endif

    //========================Memory Allocat Area=======================//
    //----MMU use----//
    addr_temp               = addr;
#ifdef MMU_SUPPORT
    mmu_tlb_base            = (u32*)addr;   // Translation table base is start from memorypool
    addr                   += SZ_16K;       // The MMU table need SZ_16K (Max)
  #ifdef COARSE_PAGE
    mmu_L1_base             = (u32*)addr;
    addr                   += SZ_2K;        // 2K for 2MB second descriptor
  #endif
    FS_internal_mem         = addr;
    addr                   += FS_MEMBLOCK_NUM*SMC_MAX_PAGE_SIZE;
#endif
    DEBUG_MAIN("addr:0x%08x~0x%08x: MMU %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
    addr_temp               = addr;

    //---NAND flash use---//
#if ( (FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    smcMBRCache             = addr; //SMC_MAX_FAT_PAGES
    addr                   += SMC_MAX_MBR_PAGES*SMC_MAX_PAGE_SIZE;

    smcGeneralBuf           = addr;
    addr                   += SMC_MAX_PAGE_SIZE * SMC_MAX_PAGE_PER_BLOCK;
#endif
    DEBUG_MAIN("addr:0x%08x~0x%08x: NAND flash %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
    addr_temp               = addr;


	//------TUTK buffer------//
	addr_temp				= addr;
	addr					= (u8 *)(((u32)addr + 63) & ~63);
	iotcBuf 				= (u8 *)addr;
	addr				   += (IOTC_BUF_SIZE);

	printf("addr:0x%08x~0x%08x: TUTK %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

#if 1
	//-------icomm buffer------------------//
		addr_temp				= addr;
		//addr					= (u8 *)(((u32)addr + 63) & ~63);
		icommbuf				= (u8 *)addr;
		//addr					+= (48*2560);
		addr				   	+= (8*2560); // Sean 20170426
		printf("addr:0x%08x~0x%08x: icomm buffer %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
	
	
	//-------icomm buffer2------------------//
		addr_temp				= addr;
		//addr					= (u8 *)(((u32)addr + 63) & ~63);
		icommbuf2				= (u8 *)addr;
		addr				   	+= (1024*256);
		printf("addr:0x%08x~0x%08x: icomm buffer2 %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
	
	//-------icomm buffer3------------------// for iperf3  20160824 by aher
		addr_temp				= addr;
		//addr					= (u8 *)(((u32)addr + 63) & ~63);
		icommbuf3				= (u8 *)addr;
		//addr					+= (10*4096*3);
		addr				   	+= (10*4096*6); // Sean 20170426
		printf("addr:0x%08x~0x%08x: icomm buffer3 %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
	//-------icomm buffer4------------------// for iperf3  20160824 by aher
		addr_temp				= addr;
		addr					= (u8 *)(((u32)addr + 63) & ~63);
		icommbuf4				= (u8 *)addr;
		addr				   	+= 4096;
		printf("addr:0x%08x~0x%08x: icomm buffer4 %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif	
		for(i = 0; i < RFI_AUDIO_RET_BUF_NUM; i++) 
		{
			iCommAudioRetBuf[i]=addr;
			addr +=1024;
		}
#if TUTK_SUPPORT
		p2plocal_buffer=addr;
		addr +=640*480*2;
		p2pEventList=addr;
		addr +=0;	//20180103 Sean: Not Use.
#endif	

    //----IPU block buffer, use in SIU/IPU/ISU capture mode----//
#if( (MULTI_CHANNEL_SUPPORT == 0) )
    ipuDstBuf0              = (u8 *)addr;
    addr                   += 0;


    ipuDstBuf1              = (u8 *)addr;
    addr                   += 0;


    ipuDstBuf2              = (u8 *)addr;
    addr                   += 0;
#elif(MULTI_CHANNEL_SUPPORT && (MULTI_CHANNEL_SEL & 0x01) )
    ipuDstBuf0              = (u8 *)addr;
    addr                   += (IPU_LINE_SIZE * 16 * 2);

    ipuDstBuf1              = (u8 *)addr;
    addr                   += (IPU_LINE_SIZE * 16 * 2);

    ipuDstBuf2              = (u8 *)addr;
    addr                   += (IPU_LINE_SIZE * 16 * 2);
#else
    ipuDstBuf0              = (u8 *)addr;
    addr                   += 0;

    ipuDstBuf1              = (u8 *)addr;
    addr                   += 0;

    ipuDstBuf2              = (u8 *)addr;
    addr                   += 0;
#endif

    //---2D Graphic command Que---//
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    addr  = (u8 *)(((u32)addr + 63) & ~63);
    gfuSwQueAddr            = addr;
    addr                   += GFU_SWQUE_CMDMAX * 16 * 4;
#endif

    DEBUG_MAIN("addr:0x%08x~0x%08x: IPU %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
    addr_temp               = addr;

    //---- IDU OSD use ----//
    addr  = (u8 *)(((u32)addr + 63) & ~63);
    OSD_buf  = addr;

    #if ((SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2)|| (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1))
        addr   += (VIDEODISPBUF_SIZE);
    #elif(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
        addr   +=0;
    #else
        if ((TV_MAXOSD_SizeX * TV_MAXOSD_SizeY)> (OSD_SizeX * OSD_SizeY))
            addr   += TVOSD_SizeX * TVOSD_SizeY;
        else
            addr   += OSD_SizeX * OSD_SizeY;
    #endif
    
    #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) )
        OSD_buf1 = addr;
        addr   += TVOSD_SizeX * TVOSD_SizeY;
        OSD_buf1_Size = TVOSD_SizeX * TVOSD_SizeY;
    #elif((CHIP_OPTION == CHIP_A1018B) )
        OSD_buf1 = addr;
        #if IDU_OSD_TEST
            addr   += TVOSD_SizeX * TVOSD_SizeY;
            OSD_buf1_Size = TVOSD_SizeX * TVOSD_SizeY;
        #else
            addr   += 80 * 80; // mouse use.FPGA limit 80x80 Amon (140718)
            OSD_buf1_Size = 80 * 80;
        #endif
    #endif
    DEBUG_MAIN("addr:0x%08x~0x%08x: IDU OSD %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

    //---------------SPI Config----------------//
    addr_temp               = addr;
    SPIConfigBuf = addr;
    addr   += SPI_BUF_SIZE;
    DEBUG_MAIN("addr:0x%08x~0x%08x:SPIConfigBuf %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

    //----------P2P used------------//
    addr_temp               = addr;
    DEBUG_MAIN("addr:0x%08x~0x%08x: P2P %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);


    //-----UI flow use for temp buffer-----//
    addr_temp               = addr;
 #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)   
        addr                    = (u8 *)(((u32)addr + 63) & ~63);
        iduvideobuff            = addr;
        addr                   += 0;
        for(i = 0; i < NUMS_PHOTOS_PREVIEW; i++) {
            addr                = (u8 *)(((u32)addr + 63) & ~63);
            iduscalerbuff[i]    = addr;
            addr               += 0;
        }
 #else
    #if ( (LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
        iduvideobuff            = addr;
        addr                   += PANNEL_X * PANNEL_Y;
        for(i = 0; i < NUMS_PHOTOS_PREVIEW; i++) {
            iduscalerbuff[i]    = addr;
            addr               += SCALER_X * SCALER_Y;
        }
    #elif ( (LCM_OPTION == VGA_640X480_60HZ) || (LCM_OPTION == VGA_800X600_60HZ) || (LCM_OPTION == VGA_1024X768_60HZ) || (LCM_OPTION == VGA_1280X800_60HZ) )
        addr                    = (u8 *)(((u32)addr + 63) & ~63);
        iduvideobuff            = addr;
        addr                   += 800* 600 * 2;
        for(i = 0; i < NUMS_PHOTOS_PREVIEW; i++) {
            addr                = (u8 *)(((u32)addr + 63) & ~63);
            iduscalerbuff[i]    = addr;
            addr               += SCALER_X * SCALER_Y * 2;
        }  
    #else
        addr                    = (u8 *)(((u32)addr + 63) & ~63);
        iduvideobuff            = addr;
        addr                   += PANNEL_X* PANNEL_Y * 2;
        for(i = 0; i < NUMS_PHOTOS_PREVIEW; i++) {
            addr                = (u8 *)(((u32)addr + 63) & ~63);
            iduscalerbuff[i]    = addr;
            addr               += SCALER_X * SCALER_Y * 2;
        }
    #endif
 #endif
 
#if ( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) || (SW_APPLICATION_OPTION  == DVP_RF_SELFTEST))
  #if MENU_DONOT_SHARE_BUFFER
    addr                    = (u8 *)(((u32)addr + 63) & ~63);
    uiMenuBuf1              = addr;
    addr                   += PANNEL_X* PANNEL_Y * 2;

    addr                    = (u8 *)(((u32)addr + 63) & ~63);
    uiMenuBuf2              = addr;
    addr                   += PANNEL_X* PANNEL_Y * 2;

   #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
    addr                    = (u8 *)(((u32)addr + 63) & ~63);
    uiMenuBuf3              = addr;
    addr                   += RF_RX_2DISP_WIDTH* RF_RX_2DISP_HEIGHT * 2;
   #else
    uiMenuBuf3=uiMenuBuf2;
   #endif
  #endif
#endif

    DEBUG_MAIN("addr:0x%08x~0x%08x: UI flow %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

    //-----Display buffer in preview mode for YUV422 mode-----//
    addr_temp               = addr;
    addr= (u8 *)(((u32)addr + 255) & 0xffffff00); //64 word alignment
#if(SW_APPLICATION_OPTION==MR8211_RFCAM_TX1)
    PKBuf0                  = (u8 *)addr;
    addr                   += 0;

    PKBuf1                  = (u8 *)addr;
    addr                   += 0;

    PKBuf2                  = (u8 *)addr;
    addr                   += 0;

    for(i = 0; i < DISPLAY_BUF_NUM; i++)
    {  //Lucian: 與preview buffer 共用
        if(i == 0)
            MainVideodisplaybuf[i]  = PKBuf0;
        else if(i == 1)
            MainVideodisplaybuf[i]  = PKBuf1;
        else if(i == 2)
            MainVideodisplaybuf[i]  = PKBuf2;
        else
        {
            MainVideodisplaybuf[i]  = addr;
            addr          += 0;
        }
    }
#else
    PKBuf0                  = (u8 *)addr;
    addr                   += (VIDEODISPBUF_SIZE);

    PKBuf1                  = (u8 *)addr;
    addr                   += (VIDEODISPBUF_SIZE);

    PKBuf2                  = (u8 *)addr;
    addr                   += (VIDEODISPBUF_SIZE);

    for(i = 0; i < DISPLAY_BUF_NUM; i++)
    {  //Lucian: 與preview buffer 共用
        if(i == 0)
            MainVideodisplaybuf[i]  = PKBuf0;
        else if(i == 1)
            MainVideodisplaybuf[i]  = PKBuf1;
        else if(i == 2)
            MainVideodisplaybuf[i]  = PKBuf2;
        else
        {
            MainVideodisplaybuf[i]  = addr;
            addr          += VIDEODISPBUF_SIZE;
        }
    }
#endif
    DEBUG_MAIN("addr:0x%08x~0x%08x: Display buffer %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

		


    //----------------- RFIU  buffer ------------------//
    addr_temp               = addr;
#if( (CHIP_OPTION >= CHIP_A1013A) )
    addr= (u8 *)(((u32)addr + 8191) & 0xffffe000); //must be 8KB alignment.

   #if (RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_SELF_TEST_TXRX_PROTOCOL || RFI_TEST_TXRX_COMMU || RFI_TEST_PERFORMANCE || RFI_TEST_PKTBURST || RFI_TEST_PKTMAP || RFI_MEASURE_RX1RX2_SENSITIVITY)
      rfiuOperBuf[0]            =(u8 *)addr;
      addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) ) * RFI_PAYLOAD_SIZE); // 1MB, command packet:64, dummy packet:64
      #if RFI_TEST_TX_PROTOCOL_B1
	    rfiuRxVideoBuf[0]         =(u8 *)addr; // 1MB
	    addr                     += 0;
	    rfiuRxVideoBufEnd[0]      =addr;

	    rfiuRxAudioBuf[0]         =(u8 *)addr;
	    addr                     += 0;
	    rfiuRxAudioBufEnd[0]      =addr;
	  #else
	    rfiuRxVideoBuf[0]         =(u8 *)addr; // 1MB
	    addr                     +=MPEG4_MAX_BUF_SIZE;
	    rfiuRxVideoBufEnd[0]      =addr;

	    rfiuRxAudioBuf[0]         =(u8 *)addr;
	    addr                     += 64*1024*4;
	    rfiuRxAudioBufEnd[0]      =addr;

        #if RX_SNAPSHOT_SUPPORT
	    rfiuRxDataBuf[0]         =(u8 *)addr;
	    addr                     += (1280 * 720 / 3); // 1280x720 picture
	    rfiuRxDataBufEnd[0]      =addr;
        #endif

	  #endif
   #else
      rfiuOperBuf[0]            = (u8 *)addr;
      addr                     += 0; // 1MB
   #endif

   #if (RFI_TEST_TX_PROTOCOL_B2 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_SELF_TEST_TXRX_PROTOCOL || RFI_TEST_TXRX_COMMU || RFI_TEST_PERFORMANCE || RFI_TEST_PKTBURST || RFI_TEST_PKTMAP || RFI_MEASURE_RX1RX2_SENSITIVITY)
      rfiuOperBuf[1]            = (u8 *)addr;
      addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE); // 1MB
      #if RFI_TEST_TX_PROTOCOL_B2
	    rfiuRxVideoBuf[1]         =(u8 *)addr; // 1MB
	    addr                     += 0;
	    rfiuRxVideoBufEnd[1]      =addr;

	    rfiuRxAudioBuf[1]         =(u8 *)addr;
	    addr                     += 0;
	    rfiuRxAudioBufEnd[1]      =addr;
	  #else
	    rfiuRxVideoBuf[1]         =(u8 *)addr; // 1MB
	    addr                     += MPEG4_MAX_BUF_SIZE;
	    rfiuRxVideoBufEnd[1]      =addr;

	    rfiuRxAudioBuf[1]         =(u8 *)addr;
	    addr                     += 64*1024*4;
	    rfiuRxAudioBufEnd[1]      =addr;

        #if RX_SNAPSHOT_SUPPORT
	    rfiuRxDataBuf[1]         =(u8 *)addr;
	    addr                     += (1280 * 720 / 3);
	    rfiuRxDataBufEnd[1]      =addr;
        #endif
	  #endif
   #else
    rfiuOperBuf[1]            = (u8 *)addr;
    addr                     += 0; // 1MB
   #endif

   #if (RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL)
    rfiuOperBuf[2]            = (u8 *)addr;
    addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE); // 1MB

    rfiuRxVideoBuf[2]         =(u8 *)addr; // 1MB
    addr                     += MPEG4_MAX_BUF_SIZE;
    rfiuRxVideoBufEnd[2]      =addr;

    rfiuRxAudioBuf[2]         =(u8 *)addr;
    addr                     += 64*1024*4;
    rfiuRxAudioBufEnd[2]      =addr;

    #if RX_SNAPSHOT_SUPPORT
    rfiuRxDataBuf[2]         =(u8 *)addr;
    addr                     += (1280 * 720 / 3);
    rfiuRxDataBufEnd[2]      =addr;
    #endif
   #else
    rfiuOperBuf[2]            = (u8 *)addr;
    addr                     += 0; // 1MB
   #endif

   #if (RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL)
    rfiuOperBuf[3]            = (u8 *)addr;
    addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE); // 1MB

    rfiuRxVideoBuf[3]         =(u8 *)addr; // 1MB
    addr                     += MPEG4_MAX_BUF_SIZE;
    rfiuRxVideoBufEnd[3]      =addr;

    rfiuRxAudioBuf[3]         =(u8 *)addr;
    addr                     += 64*1024*4;
    rfiuRxAudioBufEnd[3]      =addr;

    #if RX_SNAPSHOT_SUPPORT
    rfiuRxDataBuf[3]         =(u8 *)addr;
    addr                     += (1280 * 720 / 3);
    rfiuRxDataBufEnd[3]      =addr;
    #endif
   #else
    rfiuOperBuf[3]            = (u8 *)addr;
    addr                     += 0; // 1MB
   #endif



   //---------------------Mpeg decoding buffer for RF-RX -------------------------------//
   //Lucian: 共用Current reconstructed frame
   #if (RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
    addr                      = (u8 *)(((u32)addr + 63) & ~63);
    rfiuRxDecBuf[0].mpeg4MVBuf              = (u8 *)addr;
    addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

    addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    rfiuRxDecBuf[0].mpeg4PRefBuf_Y          = (u8 *)addr;
    addr                   += ((RF_RX_DEC_WIDTH_MAX + 32) * (RF_RX_DEC_HEIGHT_MAX + 32));

    rfiuRxDecBuf[0].mpeg4PRefBuf_Cb         = (u8 *)addr;
    addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

    rfiuRxDecBuf[0].mpeg4PRefBuf_Cr         = (u8 *)addr;
    addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

    addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    rfiuRxDecBuf[0].mpeg4NRefBuf_Y          = (u8 *)addr;
    addr                   += ((RF_RX_DEC_WIDTH_MAX + 32) * (RF_RX_DEC_HEIGHT_MAX + 32));

    rfiuRxDecBuf[0].mpeg4NRefBuf_Cb         = (u8 *)addr;
    addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

    rfiuRxDecBuf[0].mpeg4NRefBuf_Cr         = (u8 *)addr;
    addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));
   #endif

   #if (RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL)
    addr                      = (u8 *)(((u32)addr + 63) & ~63);
    rfiuRxDecBuf[1].mpeg4MVBuf              = (u8 *)addr;
    VideoBuf = (u8 *)addr;

    addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

    addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    rfiuRxDecBuf[1].mpeg4PRefBuf_Y          = (u8 *)addr;
    addr                   += ((RF_RX_DEC_WIDTH_MAX + 32) * (RF_RX_DEC_HEIGHT_MAX + 32));

    rfiuRxDecBuf[1].mpeg4PRefBuf_Cb         = (u8 *)addr;
    addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

    rfiuRxDecBuf[1].mpeg4PRefBuf_Cr         = (u8 *)addr;
    addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

    addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    mpeg4VideBufEnd = (u8 *)addr;

    rfiuRxDecBuf[1].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
    //addr                   += ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32));

    rfiuRxDecBuf[1].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
    //addr                   += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

    rfiuRxDecBuf[1].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
    //addr                   += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));
   #endif

   #if (RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL)
    addr                      = (u8 *)(((u32)addr + 63) & ~63);
    rfiuRxDecBuf[2].mpeg4MVBuf              = (u8 *)addr;
    addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

    addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    rfiuRxDecBuf[2].mpeg4PRefBuf_Y          = (u8 *)addr;
    addr                   += ((RF_RX_DEC_WIDTH_MAX + 32) * (RF_RX_DEC_HEIGHT_MAX + 32));

    rfiuRxDecBuf[2].mpeg4PRefBuf_Cb         = (u8 *)addr;
    addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

    rfiuRxDecBuf[2].mpeg4PRefBuf_Cr         = (u8 *)addr;
    addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

    addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    rfiuRxDecBuf[2].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
    //addr                   += ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32));

    rfiuRxDecBuf[2].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
    //addr                   += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

    rfiuRxDecBuf[2].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
    //addr                   += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));
    //----------//
	addr                      = (u8 *)(((u32)addr + 63) & ~63);
    rfiuRxDecBuf[3].mpeg4MVBuf              = (u8 *)addr;
    addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

    addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    rfiuRxDecBuf[3].mpeg4PRefBuf_Y          = (u8 *)addr;
    addr                   += ((RF_RX_DEC_WIDTH_MAX + 32) * (RF_RX_DEC_HEIGHT_MAX + 32));

    rfiuRxDecBuf[3].mpeg4PRefBuf_Cb         = (u8 *)addr;
    addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

    rfiuRxDecBuf[3].mpeg4PRefBuf_Cr         = (u8 *)addr;
    addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

    addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    mpeg4VideBufEnd = (u8 *)addr;


    rfiuRxDecBuf[3].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
    //addr                   += ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32));

    rfiuRxDecBuf[3].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
    //addr                   += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

    rfiuRxDecBuf[3].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
    //addr                   += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));
   #endif

   #if(  (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) )
    for(i = 0; i < DISPLAY_BUF_NUM; i++)
    {
	   Sub1Videodisplaybuf[i] = (u8 *)addr;
       addr                  += VIDEODISPBUF_SIZE;
    }
   #endif

#endif
    DEBUG_MAIN("addr:0x%08x~0x%08x: RFIU %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

    //----Jpeg thumbnail bitstream buffer----//
    addr_temp               = addr;
    exifThumbnailBitstream  = (u8 *)addr;
    addr                   += EXIF_THUMBNAIL_MAX;

#if (MULTI_CHANNEL_SUPPORT || TUTK_SUPPORT) //capture JPEG or P2P playback "BUSY" frame.
    exifPrimaryBitstream    = addr;
    addr              += IMG_MAX_WIDTH *IMG_MAX_HEIGHT;
#endif
    DEBUG_MAIN("addr:0x%08x~0x%08x: Jpeg thumbnail bitstream %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

    //-------File system use--------//
    addr_temp               = addr;
#if SDC_ECC_DETECT
    FatDummyBuf             = (u8 *)addr;
    addr                   += (DCF_BUF_SIZE);
#endif
    
    dcfBuf                  = (u8 *)addr;
    addr                   += (DCF_BUF_SIZE);

    cMulBlockBuffer         = (u8 *)addr;
    addr                   += (DCF_BUF_SIZE);

#if CDVR_iHome_LOG_SUPPORT
    dcfLogBuf_Wr            = (u8 *)addr;
    addr                   += (DCF_LOGBUF_SIZE+1024);

    dcfLogBuf_Rd             = (u8 *)addr;;
    addr                   += (DCF_LOGBUF_SIZE);
#endif


    DEBUG_MAIN("addr:0x%08x~0x%08x: File system %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

    //----IIS test used-----//
    addr_temp               = addr;
#if IIS_TEST
    addr                   += (44);//wave header
    iisBuf_play             = (u8 *)addr;
    addr                   += IIS_BUF_SIZ* IIS_REC_TIME;
#endif

    //-------- Scalar overlay image(Front end OSD) -------//
   addr= (u8 *)(((u32)addr + 255) & 0xffffff00); //64 word alignment
#if(MULTI_CHANNEL_SEL & 0x01)    
   ScalarOverlayImage      = (u32 *)addr;
   addr				   += 640 * 48 * 2;   
#else
   ScalarOverlayImage      = (u32 *)addr;
   addr				   += 0;   
#endif

#if(MULTI_CHANNEL_SEL & 0x02)    
   CiuOverlayImg1_Top = (u32 *)addr;
   addr += 640 * 80;
   CiuOverlayImg1_Bot = (u32 *)addr;
   addr += 640 * 80;
   #if( (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A) )
   CiuOverlayImg1_SP_Top = (u32 *)addr;
   addr += 640 * 80;
   CiuOverlayImg1_SP_Bot = (u32 *)addr;
   addr += 640 * 80;
   #endif
#else
   CiuOverlayImg1_Top = (u32 *)addr;
   addr += 0;
   CiuOverlayImg1_Bot = (u32 *)addr;
   addr += 0;
   #if( (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A) )
   CiuOverlayImg1_SP_Top = (u32 *)addr;
   addr += 0;
   CiuOverlayImg1_SP_Bot = (u32 *)addr;
   addr += 0;
   #endif
#endif

#if(MULTI_CHANNEL_SEL & 0x04)    
   CiuOverlayImg2_Top = (u32 *)addr;
   addr += 640 * 80;
   CiuOverlayImg2_Bot = (u32 *)addr;
   addr += 640 * 80;
#else
   CiuOverlayImg2_Top = (u32 *)addr;
   addr += 0;
   CiuOverlayImg2_Bot = (u32 *)addr;
   addr += 0;

#endif

#if(MULTI_CHANNEL_SEL & 0x08)    
   CiuOverlayImg3_Top = (u32 *)addr;
   addr += 640 * 80;
   CiuOverlayImg3_Bot = (u32 *)addr;
   addr += 640 * 80;
#else
   CiuOverlayImg3_Top = (u32 *)addr;
   addr += 0;
   CiuOverlayImg3_Bot = (u32 *)addr;
   addr += 0;

#endif

#if(MULTI_CHANNEL_SEL & 0x10)    
   CiuOverlayImg4_Top = (u32 *)addr;
   addr += 640 * 80;
   CiuOverlayImg4_Bot = (u32 *)addr;
   addr += 640 * 80;
#else
   CiuOverlayImg4_Top = (u32 *)addr;
   addr += 0;
   CiuOverlayImg4_Bot = (u32 *)addr;
   addr += 0;
#endif

    DEBUG_MAIN("addr:0x%08x~0x%08x: Scalar overlay image %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

    //------------iHome used--------------//
    addr_temp               = addr;
#if(HOME_RF_SUPPORT)
    gHomeRFSensorList=(HOMERF_SENSOR_LIST  *)addr;
    addr +=0x8000;  /* 32KB*/
    gHomeRFRoomList  =(HOMERF_ROOM_LIST    *)addr;
    addr +=0x2800;  /* 10KB*/
    gHomeRFSceneList =(HOMERF_SCENE_LIST   *)addr;
    addr +=0x2800;  /* 10KB*/
#endif
    DEBUG_MAIN("addr:0x%08x~0x%08x: iHome %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

    //==============================Common Data(JPEG and MPEG data)================================//

    PKBuf                   = (u8 *)addr;
    //===================Jpeg memory pools===================//
    addr_temp               = addr;
    addr                   += (IMG_MAX_WIDTH * (IMG_MAX_HEIGHT+32) );
    siuRawBuf               = (u8 *)addr;
    addr                   += 0;

    Jpeg_displaybuf[0]      = PKBuf0;
    Jpeg_displaybuf[1]      = PKBuf1;
    Jpeg_displaybuf[2]      = PKBuf2;

    //for Jpeg decoder
    //exifDecBuf              = (u8 *)PKBuf+IMG_MAX_WIDTH *IMG_MAX_HEIGHT * 2;

    //for Jpeg encoder
#if (MULTI_CHANNEL_SUPPORT == 0)
  #if (JPEG_ENC_OPMODE_OPTION == JPEG_OPMODE_FRAME)
    exifPrimaryBitstream    = addr;
    addr                   += IMG_MAX_WIDTH *IMG_MAX_HEIGHT;
  #elif (JPEG_ENC_OPMODE_OPTION == JPEG_OPMODE_SLICE)
    exifPrimaryBitstream    = (u8 *)PKBuf+ 4000*2*16;   //Lucian:for jpeg block mode buffer. support 12M
  #endif
#endif

#if ADDAPP2TOJPEG
    //Appendix information
    exifApp2Data            = (DEF_APPENDIXINFO *)addr;
    addr                   += (160*120+512);
#endif

#if ADDAPP3TOJPEG
    exifAPP3VGABitstream    = addr;  //放pannel 大小縮圖
    addr                   += JPGAPP3_WIDTH *JPGAPP3_HEIGHT/2;
#endif
    DEBUG_MAIN("addr:0x%08x~0x%08x: JPEG %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);

    //===================Mpeg memory pools===================//
    Mpeg_addr               = PKBuf;
    Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
    Mpeg_addr_temp          = Mpeg_addr;

    //Mpeg-input buffer in Encoding is  overlay with Mpeg-out buffer in decoding.
#if(MULTI_CHANNEL_SUPPORT)
    #if(MULTI_CHANNEL_SEL & 0x01)
        //Decording using 與 SIU/ISU preview buffer 公用//
        mpeg4outputbuf[0]       = Mpeg_addr;
        Mpeg_addr              += PNBUF_SIZE_Y * 2;
        mpeg4outputbuf[1]       = Mpeg_addr;
        Mpeg_addr              += PNBUF_SIZE_Y * 2;
        mpeg4outputbuf[2]       = Mpeg_addr;
        Mpeg_addr              += PNBUF_SIZE_Y * 2;

        PNBuf_Y[0]  = PNBuf_Y0 = PKBuf;
        PNBuf_C[0]  = PNBuf_C0 = PNBuf_Y0 + PNBUF_SIZE_Y;
        PNBuf_Y[1]  = PNBuf_Y1 = PNBuf_C0 + PNBUF_SIZE_C;
        PNBuf_C[1]  = PNBuf_C1 = PNBuf_Y1 + PNBUF_SIZE_Y;
        PNBuf_Y[2]  = PNBuf_Y2 = PNBuf_C1 + PNBUF_SIZE_C;
        PNBuf_C[2]  = PNBuf_C2 = PNBuf_Y2 + PNBUF_SIZE_Y;
        PNBuf_Y[3]  = PNBuf_Y3 = PNBuf_C2 + PNBUF_SIZE_C;
        PNBuf_C[3]  = PNBuf_C3 = PNBuf_Y3 + PNBUF_SIZE_Y;
    #else
        PNBuf_Y[0]  = PNBuf_Y0 = PKBuf;
        PNBuf_C[0]  = PNBuf_C0 = PNBuf_Y0 + 0;
        PNBuf_Y[1]  = PNBuf_Y1 = PNBuf_C0 + 0;
        PNBuf_C[1]  = PNBuf_C1 = PNBuf_Y1 + 0;
        PNBuf_Y[2]  = PNBuf_Y2 = PNBuf_C1 + 0;
        PNBuf_C[2]  = PNBuf_C2 = PNBuf_Y2 + 0;
        PNBuf_Y[3]  = PNBuf_Y3 = PNBuf_C2 + 0;
        PNBuf_C[3]  = PNBuf_C3 = PNBuf_Y3 + 0;
    #endif

    #if ( (MULTI_CHANNEL_SEL & 0x01) == 0)
      #if(HW_BOARD_OPTION == MR8211_ZINWELL)
        Mpeg_addr   = addr;
      #else
        Mpeg_addr   = PKBuf;
      #endif
	    Mpeg_addr   = (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
    #endif
    DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x: Ch0 mpeg4outputbuf %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);

    //--------------------CIU1 -------------------//
    Mpeg_addr_temp              = Mpeg_addr;
    #if(MULTI_CHANNEL_SEL & 0x02)
      #if DUAL_MODE_DISP_SUPPORT
       #define PNBUF_SUB1_SIZE  ((PNBUF_SIZE_Y + PNBUF_SIZE_C) * 2)
      #else
       #define PNBUF_SUB1_SIZE  (PNBUF_SIZE_Y + PNBUF_SIZE_C)
      #endif      
       #if IS_COMMAX_DOORPHONE
           #undef PNBUF_SUB1_SIZE
           #define PNBUF_SUB1_SIZE  (PNBUF_SIZE_Y*2)
       #endif

       PNBuf_sub1[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB1_SIZE;

       PNBuf_sub1[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB1_SIZE;

       PNBuf_sub1[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB1_SIZE;

       PNBuf_sub1[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB1_SIZE;

       #undef PNBUF_SUB1_SIZE
    #else
       PNBuf_sub1[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub1[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub1[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub1[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;
    #endif

    #if ( (MULTI_CHANNEL_SEL & 0x03) == 0)
       Mpeg_addr               = PKBuf;
	   Mpeg_addr= (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
    #endif

       DEBUG_MAIN("PNBuf_sub1:0x%08x~0x%08x: Ch1 PNBuf_sub1 %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
    
       //-----------------CIU2--------------------//
       Mpeg_addr_temp          = Mpeg_addr;

    #if(MULTI_CHANNEL_SEL & 0x04)
     #if (HW_BOARD_OPTION == MR8211_ZINWELL)
       //#define PNBUF_SUB2_SIZE  (320 * 240 * 3 / 2)
       #define PNBUF_SUB2_SIZE  (PNBUF_SIZE_Y + PNBUF_SIZE_C)

       PNBuf_sub2[0]           = (u8 *)PNBuf_sub1[3] + PNBUF_SUB1_SIZE;
       PNBuf_sub2[1]           = (u8 *)PNBuf_sub2[0] + PNBUF_SUB2_SIZE;
       PNBuf_sub2[2]           = (u8 *)PNBuf_sub2[0] + PNBUF_SUB2_SIZE * 2;
       PNBuf_sub2[3]           = (u8 *)PNBuf_sub2[0] + PNBUF_SUB2_SIZE * 3;
       DEBUG_MAIN("PNBuf_sub2:0x%08x~0x%08x: Ch2 PNBuf_sub2 %d bytes\n", (u32)PNBuf_sub2[0], (u32)PNBuf_sub2[0] + PNBUF_SUB2_SIZE * 4, PNBUF_SUB2_SIZE * 4);
       Mpeg_addr_temp              = Mpeg_addr;
     #else
      #if DUAL_MODE_DISP_SUPPORT
       PNBuf_sub2[0]           = PNBuf_sub1[0] + 640;
       PNBuf_sub2[1]           = PNBuf_sub1[1] + 640;
       PNBuf_sub2[2]           = PNBuf_sub1[2] + 640;
       PNBuf_sub2[3]           = PNBuf_sub1[3] + 640;
      #else
       #define PNBUF_SUB2_SIZE  (PNBUF_SIZE_Y + PNBUF_SIZE_C)

       PNBuf_sub2[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB2_SIZE;

       PNBuf_sub2[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB2_SIZE;

       PNBuf_sub2[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB2_SIZE;

       PNBuf_sub2[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB2_SIZE;
      #endif
     #endif
    #else
       PNBuf_sub2[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub2[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub2[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub2[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;
    #endif

    #if ( (MULTI_CHANNEL_SEL & 0x07) == 0)
       Mpeg_addr               = PKBuf;
	   Mpeg_addr= (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
    #endif

       DEBUG_MAIN("PNBuf_sub2:0x%08x~0x%08x: Ch3 PNBuf_sub2 %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
    
       //-----------------CIU3------------------//
       Mpeg_addr_temp          = Mpeg_addr;
    #if(MULTI_CHANNEL_SEL & 0x08)
       PNBuf_sub3[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

       PNBuf_sub3[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

       PNBuf_sub3[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

       PNBuf_sub3[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);
    #else
       PNBuf_sub3[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub3[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub3[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub3[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;
    #endif

    #if ( (MULTI_CHANNEL_SEL & 0x0f) == 0)
       Mpeg_addr               = PKBuf;
	   Mpeg_addr= (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
    #endif
       DEBUG_MAIN("PNBuf_sub3:0x%08x~0x%08x: Ch3 PNBuf_sub3 %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);

       //--------------CIU4---------------//
       Mpeg_addr_temp          = Mpeg_addr;
    #if(MULTI_CHANNEL_SEL & 0x10)
       PNBuf_sub4[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

       PNBuf_sub4[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

       PNBuf_sub4[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

       PNBuf_sub4[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);
    #else
       PNBuf_sub4[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub4[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub4[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

       PNBuf_sub4[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;
    #endif

    #if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
       PNBuf_Quad              = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);
    #endif
        DEBUG_MAIN("PNBuf_sub4:0x%08x~0x%08x: Ch4 PNBuf_sub4 %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);

#else   //====For MR6270 use only===//
        //Decording using 與 SIU/ISU preview buffer 公用//
        mpeg4outputbuf[0]       = Mpeg_addr;
        Mpeg_addr              += PNBUF_SIZE_Y * 2;
        mpeg4outputbuf[1]       = Mpeg_addr;
        Mpeg_addr              += PNBUF_SIZE_Y * 2;
        mpeg4outputbuf[2]       = Mpeg_addr;
        Mpeg_addr              += PNBUF_SIZE_Y * 2;

        PNBuf_Y[0]  = PNBuf_Y0 = PKBuf;
        PNBuf_C[0]  = PNBuf_C0 = PNBuf_Y0 + PNBUF_SIZE_Y;
        PNBuf_Y[1]  = PNBuf_Y1 = PNBuf_C0 + PNBUF_SIZE_C;
        PNBuf_C[1]  = PNBuf_C1 = PNBuf_Y1 + PNBUF_SIZE_Y;
        PNBuf_Y[2]  = PNBuf_Y2 = PNBuf_C1 + PNBUF_SIZE_C;
        PNBuf_C[2]  = PNBuf_C2 = PNBuf_Y2 + PNBUF_SIZE_Y;
        PNBuf_Y[3]  = PNBuf_Y3 = PNBuf_C2 + PNBUF_SIZE_C;
        PNBuf_C[3]  = PNBuf_C3 = PNBuf_Y3 + PNBUF_SIZE_Y;
#endif


//----------------------------------------------------------------------//
    Mpeg_addr_temp          = Mpeg_addr;

#if MULTI_CHANNEL_VIDEO_REC

    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        if(MULTI_CHANNEL_SEL & (1 << i) || (i >= MULTI_CHANNEL_LOCAL_MAX))
        {
            u32 mpeg4_max_width, mpeg4_max_height;

        #if (HW_BOARD_OPTION == MR8211_ZINWELL)
            if(i == 2)
            {
                mpeg4_max_width     = MPEG4_MAX_WIDTH;
                mpeg4_max_height    = MPEG4_MAX_HEIGHT;
            } else {
                mpeg4_max_width     = 640;
                mpeg4_max_height    = 480;
            }
        #else
            mpeg4_max_width     = MPEG4_MAX_WIDTH;
            mpeg4_max_height    = MPEG4_MAX_HEIGHT;
        #endif
            pVideoClipOption    = &VideoClipOption[i];
        #if ( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) ||\
               (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) ||\
               (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) ||(SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
            #if(VIDEO_CODEC_OPTION == H264_CODEC)
            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr+0x07) & 0xfffffff8); //double word aligned

            pVideoClipOption->H264MBPredBuf     = (u8 *)Mpeg_addr;                              //double word aligned
            Mpeg_addr                          += 0; //include upper 4 YUV line, FRAME_WIDTH*2*4

            pVideoClipOption->H264ILFPredBuf    = (u8 *)Mpeg_addr;                             //double word aligned
            Mpeg_addr                          += 0; //include upper 4 YUV line, FRAME_WIDTH*2*4

            pVideoClipOption->H264IntraPredBuf  = (u8 *)Mpeg_addr;                            //double word aligned
            Mpeg_addr                          += 0;  //include upper YUV line, FRAME_WIDTH*2
            #endif

            pVideoClipOption->mpeg4MVBuf        = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

            pVideoClipOption->mpeg4PRefBuf_Y    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

            pVideoClipOption->mpeg4PRefBuf_Cb   = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

            pVideoClipOption->mpeg4PRefBuf_Cr   = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

            pVideoClipOption->mpeg4NRefBuf_Y    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

            pVideoClipOption->mpeg4NRefBuf_Cb   = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

            pVideoClipOption->mpeg4NRefBuf_Cr    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

            for(j = 0; j < IIS_BUF_NUM; j++)
            {
                pVideoClipOption->iisSounBuf[j] = Mpeg_addr;
                Mpeg_addr                      += 0;
            }
        #else
            #if(VIDEO_CODEC_OPTION == H264_CODEC)
            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr+0x07) & 0xfffffff8); //double word aligned

            pVideoClipOption->H264MBPredBuf     = (u8 *)Mpeg_addr;                              //double word aligned
            Mpeg_addr                          += (((mpeg4_max_width*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

            pVideoClipOption->H264ILFPredBuf    = (u8 *)Mpeg_addr;                             //double word aligned
            Mpeg_addr                          += (((mpeg4_max_width*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

            pVideoClipOption->H264IntraPredBuf  = (u8 *)Mpeg_addr;                            //double word aligned
            Mpeg_addr                          += (((mpeg4_max_width*2)+0x07) & 0xfffffff8);  //include upper YUV line, FRAME_WIDTH*2
            #endif

            pVideoClipOption->mpeg4MVBuf        = (u8 *)Mpeg_addr;
            Mpeg_addr                          += ((MPEG4_MVBUF + 63) & ~63);

            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

            pVideoClipOption->mpeg4PRefBuf_Y    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += ((mpeg4_max_width + 32) * (mpeg4_max_height + 32));

            pVideoClipOption->mpeg4PRefBuf_Cb   = (u8 *)Mpeg_addr;
            Mpeg_addr                          += (((mpeg4_max_width >> 1) + 16) * ((mpeg4_max_height >> 1) + 16));

            pVideoClipOption->mpeg4PRefBuf_Cr         = (u8 *)Mpeg_addr;
            Mpeg_addr                          += (((mpeg4_max_width >> 1) + 16) * ((mpeg4_max_height >> 1) + 16));

            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

            pVideoClipOption->mpeg4NRefBuf_Y    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += ((mpeg4_max_width + 32) * (mpeg4_max_height + 32));

            pVideoClipOption->mpeg4NRefBuf_Cb   = (u8 *)Mpeg_addr;
            Mpeg_addr                          += (((mpeg4_max_width >> 1) + 16) * ((mpeg4_max_height >> 1) + 16));

            pVideoClipOption->mpeg4NRefBuf_Cr    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += (((mpeg4_max_width >> 1) + 16) * ((mpeg4_max_height >> 1) + 16));

            for(j = 0; j < IIS_BUF_NUM; j++)
            {
                pVideoClipOption->iisSounBuf[j] = Mpeg_addr;
                Mpeg_addr                      += IIS_CHUNK_SIZE;
            }
        #endif

        #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
            pVideoClipOption->ImaAdpcmBuf       = Mpeg_addr;
            Mpeg_addr                          += IMA_ADPCM_BLOCK_SIZE;
            pVideoClipOption->ImaAdpcmBufEnd    = Mpeg_addr;
        #endif

        #if ( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) ||  (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
            // Mpeg Video encoding/decoding bitstream buffer
            pVideoClipOption->VideoBuf          = Mpeg_addr;
            Mpeg_addr                          += 0;
            pVideoClipOption->mpeg4VideBufEnd   = Mpeg_addr;
        #else
            // Mpeg Video encoding/decoding bitstream buffer
            pVideoClipOption->VideoBuf          = Mpeg_addr;
          #if (HW_BOARD_OPTION == MR8211_ZINWELL)
            if(i == 2)
                Mpeg_addr                          += MPEG4_MAX_BUF_SIZE;
            else    // CIU1是給P2p用的,可以小一點
                Mpeg_addr                          += PNBUF_SIZE_Y;
          #else
            Mpeg_addr                          += MPEG4_MAX_BUF_SIZE;
          #endif
            pVideoClipOption->mpeg4VideBufEnd   = Mpeg_addr;
        #endif

        #if CDVR_LOG
            Mpeg_addr                           = (u8*)(((u32)Mpeg_addr + 15) & ~15);
            pVideoClipOption->LogFileBuf        = Mpeg_addr;
            Mpeg_addr                          += LOG_FILE_MAX_SIZE;
            pVideoClipOption->LogFileBufEnd     = Mpeg_addr;
        #endif

        #if (CDVR_TEST_LOG)
            Mpeg_addr                           = (u8*)(((u32)Mpeg_addr + 15) & ~15);
            pVideoClipOption->LogFileBuf        = Mpeg_addr;
            Mpeg_addr                          += LOG_FILE_MAX_SIZE;
            pVideoClipOption->LogFileBufEnd     = Mpeg_addr;
        #endif

            pVideoClipOption->mpeg4IndexBuf     = Mpeg_addr;
            Mpeg_addr                          += MPEG4_INDEX_BUF_SIZE;

            DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x: Ch%d MPEG4 %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, i, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
            Mpeg_addr_temp              = Mpeg_addr;
        }
    }

  //======================= for playback =========================//
   Mpeg_addr_temp          = Mpeg_addr;
  #if (RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL)
     //與 RF RX Decoding buffer 共用
    mpeg4MVBuf              = rfiuRxDecBuf[0].mpeg4MVBuf;

    mpeg4PRefBuf_Y          = rfiuRxDecBuf[0].mpeg4PRefBuf_Y;

    mpeg4PRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4PRefBuf_Cb;

    mpeg4PRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4PRefBuf_Cr;

    mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

    mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

    mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;

    for(i = 0; i < IIS_BUF_NUM; i++)
    {
        iisSounBuf[i]       = Mpeg_addr;
        Mpeg_addr          += IIS_CHUNK_SIZE;
    }

   #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    ImaAdpcmBuf             = Mpeg_addr;
    Mpeg_addr              += IMA_ADPCM_BLOCK_SIZE;
    ImaAdpcmBufEnd          = Mpeg_addr;
   #endif

    // Mpeg Video encoding/decoding bitstream buffer
   #if 1
     //與 RF RX Decoding buffer 共用
   #else
     VideoBuf                = Mpeg_addr;
     Mpeg_addr              += MPEG4_MAX_BUF_SIZE;
     mpeg4VideBufEnd         = Mpeg_addr;
   #endif

   #if CDVR_LOG
    Mpeg_addr               = (u8*)(((u32)Mpeg_addr + 15) & ~15);
    LogFileBuf              = Mpeg_addr;
    Mpeg_addr              += LOG_FILE_MAX_SIZE;
    LogFileBufEnd           = Mpeg_addr;
   #endif

   #if (CDVR_TEST_LOG)
    Mpeg_addr               = (u8*)(((u32)Mpeg_addr + 15) & ~15);
    LogFileBuf              = Mpeg_addr;
    Mpeg_addr              += LOG_FILE_MAX_SIZE;
    LogFileBufEnd           = Mpeg_addr;
   #endif

    mpeg4IndexBuf           = Mpeg_addr;
    Mpeg_addr              += MPEG4_INDEX_BUF_SIZE;

  #else  // for no RF project, doorphone  etc.

   #if (HW_BOARD_OPTION == MR8211_ZINWELL)
    pVideoClipOption        = &VideoClipOption[2];  // QVGA記憶體太小,要使用720P的才能撥放720P影片
   #endif

	mpeg4MVBuf              = pVideoClipOption->mpeg4MVBuf;

    mpeg4PRefBuf_Y          = pVideoClipOption->mpeg4PRefBuf_Y;

    mpeg4PRefBuf_Cb         = pVideoClipOption->mpeg4PRefBuf_Cb;

    mpeg4PRefBuf_Cr         = pVideoClipOption->mpeg4PRefBuf_Cr;

    mpeg4NRefBuf_Y          = pVideoClipOption->mpeg4NRefBuf_Y;

    mpeg4NRefBuf_Cb         = pVideoClipOption->mpeg4NRefBuf_Cb;

    mpeg4NRefBuf_Cr         = pVideoClipOption->mpeg4NRefBuf_Cr;

#if REMOTE_TALK_BACK    // Peter: APP talk back 會有邊錄邊放的可能,所以要有獨立的記憶體.
    for(i = 0; i < IIS_BUF_NUM; i++)
    {
        iisSounBuf[i]       = Mpeg_addr;
        Mpeg_addr          += IIS_CHUNK_SIZE;
    }
#else
    for(i = 0; i < IIS_BUF_NUM; i++)
    {
        iisSounBuf[i]       = pVideoClipOption->iisSounBuf[i];
    }
#endif

   #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    ImaAdpcmBuf             = pVideoClipOption->ImaAdpcmBuf;
    ImaAdpcmBufEnd          = pVideoClipOption->ImaAdpcmBufEnd;
   #endif

    // Mpeg Video encoding/decoding bitstream buffer
    VideoBuf                = pVideoClipOption->VideoBuf;
    mpeg4VideBufEnd         = pVideoClipOption->mpeg4VideBufEnd;

   #if CDVR_LOG
    LogFileBuf              = pVideoClipOption->LogFileBuf;
    LogFileBufEnd           = pVideoClipOption->LogFileBufEnd;
   #endif

   #if (CDVR_TEST_LOG)
    LogFileBuf              = pVideoClipOption->LogFileBuf;
    LogFileBufEnd           = pVideoClipOption->LogFileBufEnd;
   #endif

    mpeg4IndexBuf           = pVideoClipOption->mpeg4IndexBuf;
  #endif

    DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x: Audio+Index+Playback %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);

#else//--------------------------------(MULTI_CHANNEL_VIDEO_REC == 0) ----------------------------------

    #if(VIDEO_CODEC_OPTION == H264_CODEC)
        Mpeg_addr              = (u8 *)(((u32)Mpeg_addr+0x07) & 0xfffffff8); //double word aligned

        H264MBPredBuf          = (u8 *)Mpeg_addr;                              //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

        H264ILFPredBuf          = (u8 *)Mpeg_addr;                             //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

        H264IntraPredBuf        = (u8 *)Mpeg_addr;                            //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2)+0x07) & 0xfffffff8);  //include upper YUV line, FRAME_WIDTH*2
    #endif

    #if ( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
        mpeg4MVBuf              = rfiuRxDecBuf[0].mpeg4MVBuf;

        mpeg4PRefBuf_Y          = rfiuRxDecBuf[0].mpeg4PRefBuf_Y;

        mpeg4PRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4PRefBuf_Cb;

        mpeg4PRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4PRefBuf_Cr;

        mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

        mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

        mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;

    #elif ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
        //For TX
        mpeg4MVBuf              = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

        Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

        mpeg4PRefBuf_Y          = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((RF_RX_DEC_WIDTH_MAX + 32) * (RF_RX_DEC_HEIGHT_MAX + 32));

        mpeg4PRefBuf_Cb         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

        mpeg4PRefBuf_Cr         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

        Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        mpeg4NRefBuf_Y          = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((RF_RX_DEC_WIDTH_MAX + 32) * (RF_RX_DEC_HEIGHT_MAX + 32));

        mpeg4NRefBuf_Cb         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

        mpeg4NRefBuf_Cr         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((RF_RX_DEC_WIDTH_MAX >> 1) + 16) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 16));

    #else   // for playback
    	mpeg4MVBuf              = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((MPEG4_MVBUF + 63) & ~63);

        Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

        mpeg4PRefBuf_Y          = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32));

        mpeg4PRefBuf_Cb         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

        mpeg4PRefBuf_Cr         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

        Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        mpeg4NRefBuf_Y          = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32));

        mpeg4NRefBuf_Cb         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

        mpeg4NRefBuf_Cr         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));
   #endif
        for(i = 0; i < IIS_BUF_NUM; i++)
        {
            iisSounBuf[i]       = Mpeg_addr;
            Mpeg_addr          += IIS_CHUNK_SIZE;
        }

   #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    ImaAdpcmBuf             = Mpeg_addr;
    Mpeg_addr              += IMA_ADPCM_BLOCK_SIZE;
    ImaAdpcmBufEnd          = Mpeg_addr;
   #endif

    // Mpeg Video encoding/decoding bitstream buffer
   #if (RFI_TEST_RXRX_PROTOCOL_B1B2 ||  RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
    //與 RF RX Decoding buffer 共用
   #else
    VideoBuf                = Mpeg_addr;
    Mpeg_addr              += MPEG4_MAX_BUF_SIZE;
    mpeg4VideBufEnd         = Mpeg_addr;
   #endif

   #if CDVR_LOG
    Mpeg_addr               = (u8*)(((u32)Mpeg_addr + 15) & ~15);
    LogFileBuf              = Mpeg_addr;
    Mpeg_addr              += LOG_FILE_MAX_SIZE;
    LogFileBufEnd           = Mpeg_addr;
   #endif

   #if (CDVR_TEST_LOG)
    Mpeg_addr               = (u8*)(((u32)Mpeg_addr + 15) & ~15);
    LogFileBuf              = Mpeg_addr;
    Mpeg_addr              += LOG_FILE_MAX_SIZE;
    LogFileBufEnd           = Mpeg_addr;
   #endif

    mpeg4IndexBuf           = Mpeg_addr;
    Mpeg_addr              += MPEG4_INDEX_BUF_SIZE;


#endif

    //-----------------------P2P playback Video buffer-----------------------------------------//
    Mpeg_addr_temp          = Mpeg_addr;
#if NIC_SUPPORT
    P2PPBVideoBuf           = Mpeg_addr;
    Mpeg_addr              += PNBUF_SIZE_Y;
    P2PPBVideBufEnd         = Mpeg_addr;
    DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x: P2PPBVideoBuf %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
#endif

    //===========JPEG Decoding buffer========//
    Mpeg_addr_temp          = Mpeg_addr;
    exifDecBuf              = Mpeg_addr;
    Mpeg_addr              +=256*1024; //reverved 256KB
    DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x: exifDecBuf %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
    //===========USB used=============//
    Mpeg_addr_temp          = Mpeg_addr;
#if(USB2WIFI_SUPPORT || USB_DEVICE)
    usb_device_buf          = (u8 *)Mpeg_addr;
    Mpeg_addr                   += 1024 *50;
#endif
#if USB2WIFI_SUPPORT
    Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & ~255);
    usbfwupgrade_buf        = Mpeg_addr;
    Mpeg_addr              += 1024 *1024;
    
    usb_AV_buf              = Mpeg_addr;
    Mpeg_addr              += 2048 *512;
    usb_AV_buf_end          = Mpeg_addr;
#endif

#if (USB_HOST == 1)
	//QH1
	Mpeg_addr                   += 0x1f ;
	Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
    usb_qh_buf_1            = (u8 *)Mpeg_addr ;
    Mpeg_addr                   += (4*12) ;
	//QH2
	Mpeg_addr                   += 0x1f ;
	Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	usb_qh_buf_2            = (u8 *)Mpeg_addr ;
    Mpeg_addr                   += (4*12) ;
    //QH3
	Mpeg_addr                   += 0x1f ;
	Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	usb_qh_buf_3            = (u8 *)Mpeg_addr ;
    Mpeg_addr                   += (4*12) ;
	//QTD
	Mpeg_addr                   += 0x1f ;
	Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	usb_qtd_buf           	= (u8 *)Mpeg_addr ;
    Mpeg_addr                   += (4*8)*30 ;
	//iTD1
	Mpeg_addr                   += 0x1f ;
	Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	usb_itd_buf_1           = (u8 *)Mpeg_addr ;
    Mpeg_addr                   += (4*16) ;
	//Page0
    Mpeg_addr                   += 0xfff ;
    Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xfffff000);
    usb_Page_buf_0          = (u32 *)Mpeg_addr ;
    Mpeg_addr                   +=  1024*4;
#endif
    DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x: USB %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);

#if defined(PIC_OP)
    Mpeg_addr_temp          = Mpeg_addr;
    Mpeg_addr = (u8 *)(((u32)Mpeg_addr + 31) & 0xffffffe0); // 32 bytes alignment
    picTmpBuf = (u8 *)Mpeg_addr;
    Mpeg_addr += (VIDEODISPBUF_SIZE);
    DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x: PIC_OP %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
#endif

    
#if VMDSW
    //------VMDlog buffer------//
    //------VMD mean buffer------//
    Mpeg_addr_temp               = Mpeg_addr;
    Mpeg_addr              = (u8 *)(((u32)Mpeg_addr + 63) & ~63);
  #if INTERPOLATION
    VMDMeanBuf1            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE_INTER); 
    VMDMeanBuf2            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE_INTER); 
  #else
    VMDMeanBuf1            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE); 
    VMDMeanBuf2            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE); 
  #endif

    VMDPositiveCnt_X       = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);
    VMDPositiveCnt_Y       = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE); 
    VMDNegativeCnt_X       = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE); 
    VMDNegativeCnt_Y       = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

    VMDFilterPX            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);
    VMDFilterNX            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);
    VMDFilterPY            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);
    VMDFilterNY            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

    VMDFlterMap            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);  

    for(i = 0; i < PeriodTime; i++) 
    {
        VMDPositiveBuf_X[i]       = (u8 *)Mpeg_addr;
        Mpeg_addr                += (VMD_BITBUF_SIZE);
        VMDPositiveBuf_Y[i]       = (u8 *)Mpeg_addr;
        Mpeg_addr                += (VMD_BITBUF_SIZE); 
        VMDNegativeBuf_X[i]       = (u8 *)Mpeg_addr;
        Mpeg_addr                += (VMD_BITBUF_SIZE); 
        VMDNegativeBuf_Y[i]       = (u8 *)Mpeg_addr;
        Mpeg_addr                += (VMD_BITBUF_SIZE);
    }

   //method 2
  #if VMDSW
    VMDMotionPos_X       = (u8 *)Mpeg_addr;
    Mpeg_addr                += (VMD_BUF_SIZE);
    VMDMotionPos_Y       = (u8 *)Mpeg_addr;
    Mpeg_addr                += (VMD_BUF_SIZE);
  #endif
    DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x: VMDBuf %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
#endif

    //========JPEG bitstream buffer for TX snapshot========//
#if TX_SNAPSHOT_SUPPORT
    Mpeg_addr_temp          = Mpeg_addr;
	Mpeg_addr = (u8 *)(((u32)Mpeg_addr + 0x0FFF) & 0xFFFFF000);
    sysRFTXImgData=Mpeg_addr;
    Mpeg_addr +=MPEG4_MAX_WIDTH*MPEG4_MAX_HEIGHT/3;
    DEBUG_MAIN("addr:0x%08x~0x%08x: JPG-TX Snapshot %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
#endif	

    //========================================================//
    Mpeg_addr_temp               = Mpeg_addr;
    DEBUG_MAIN("Mpeg_addr   = 0x%08x\n", Mpeg_addr);
    DEBUG_MAIN("addr        = 0x%08x\n", addr);
#if (CDVR_TEST_LOG)
    DEBUG_MAIN("LogFileBuf  = 0x%08x\n", LogFileBuf);
    DEBUG_MAIN("LogFileBufEnd  = 0x%08x\n", LogFileBufEnd);
#endif

    if(Mpeg_addr>addr)
        return Mpeg_addr;
    else
        return addr;
}

