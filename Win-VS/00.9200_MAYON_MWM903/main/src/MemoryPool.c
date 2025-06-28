#include "general.h"
#include "sysapi.h"
#include "fsapi.h"
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


//#define DRAW_MEMORY_POOL


 u8 *CaptureStartAdr;

 u8 *siuRawBuf;

 u8 *PKBuf;
 u8 *PKBuf0;
 u8 *PKBuf1;
 u8 *PKBuf2;

 u8 *rfiuOperBuf[MAX_RFIU_UNIT];
 u8 *rfiuRxVideoBuf[MAX_RFIU_UNIT];
 u8 *rfiuRxVideoBufEnd[MAX_RFIU_UNIT];
 
 u8 *rfiuRxAudioBuf[MAX_RFIU_UNIT];
 u8 *rfiuRxAudioBufEnd[MAX_RFIU_UNIT];
 
#if RX_SNAPSHOT_SUPPORT
 u8 *rfiuRxDataBuf[MAX_RFIU_UNIT];
 u8 *rfiuRxDataBufEnd[MAX_RFIU_UNIT];
#endif 
#if TX_FW_UPDATE_SUPPORT
 u8 *rfiuTXFwUpdBuf;
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
 u32 rfiuRX_OpModeCmdRetry[MAX_RFIU_UNIT];
 u32 rfiuRX_P2pVideoQuality;

#if ICOMMWIFI_SUPPORT
 s32 rfiu_TX_P2pVideoQuality = 1; //RFWIFI_P2P_QUALITY_MEDI
 s32 rfiu_TX_WifiPower;
 s32 rfiu_TX_WifiCHNum;

 s32 Icomm_WifiPower;
 s32 Icomm_WifiCHNum;
#endif
 
 u32 rfiu_TX8BitPCMFmt;

 u32 rfiuRXWrapSyncErrCnt[MAX_RFIU_UNIT];
 u8 *MaskAreaBuf;

#if( RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2)
    u32 rfiuRX_CamOnOff_Sta=0x01;
    u32 rfiuRX_CamOnOff_Num=1;

#elif(RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1)
    u32 rfiuRX_CamOnOff_Sta=0x03;
    u32 rfiuRX_CamOnOff_Num=2;
#elif(RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
    u32 rfiuRX_CamOnOff_Sta=0x03;
    u32 rfiuRX_CamOnOff_Num=2;
#elif(RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
    u32 rfiuRX_CamOnOff_Sta=0x03;
    u32 rfiuRX_CamOnOff_Num=2;
#else
    u32 rfiuRX_CamOnOff_Sta=0x0f;
    u32 rfiuRX_CamOnOff_Num=4;
#endif
 u32 rfiuRX_CamPair_Sta;
 u32 rfiuRX_CamPerRF;
 u32 rfiuTxPIR_Trig=0;
 u32 rfiuRX_CamSleep_Sta=0xff;



 u32 rfiuVideoBufFill_idx[MAX_RFIU_UNIT];
 u32 rfiuVideoBufPlay_idx[MAX_RFIU_UNIT];

 u8 *PNBuf_sub1[4];  //for CIU1
 u8 *PNBuf_sub2[4];  //For CIU2
 u8 *PNBuf_sub3[4];  //For CIU3
 u8 *PNBuf_sub4[4];  //For CIU4
 u8 *PNBuf_sub5[4];  //For CIU4

 u8 *PNBuf_Quad;
 u8 *PKBuf_PIP0Y;
 u8 *PKBuf_PIP1Y;
 u8 *PKBuf_PIP2Y;
 u8 *PKBuf_PIP0CbCr;
 u8 *PKBuf_PIP1CbCr;
 u8 *PKBuf_PIP2CbCr;
 
 unsigned char *smcMBRCache;
 unsigned char *smcGeneralBuf;


 u8* smcGeneralBuf;
 u8 *ipuDstBuf0;
 u8 *ipuDstBuf1;
 u8 *ipuDstBuf2;

#if ( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
u8 *MainVideodisplaybuf[DISPLAY_BUF_NUM+2]; //Lsk: 6, 7 for Panel, Single mode, RecFrame/RefFrame
#else
 u8 *MainVideodisplaybuf[DISPLAY_BUF_NUM];
#endif
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
 int P2PTxBitRate[1];
 int P2PTxBufFullness[1];

 u8 *PNBuf_Y0, *PNBuf_C0, *PNBuf_Y1, *PNBuf_C1, *PNBuf_Y2, *PNBuf_C2; /*Lucian 070531*/
 u8 *PNBuf_Y3, *PNBuf_C3;
 u8 *PNBuf_Y[4], *PNBuf_C[4];
 u8* mpeg4outputbuf[3];
 u8* VideoBuf;
 u8* mpeg4VideBufEnd;
 u8* mpeg4IndexBuf;
 #if (FPGA_BOARD_A1018_SERIES)
 u8 *Web_page;
 #endif
#if (CDVR_LOG || CDVR_TEST_LOG)
 u8* LogFileBuf;
 u8* LogFileBufEnd;
#endif

//#if (USB_DEVICE == 1)
u8*  usb_device_buf;
//#endif

u8*  usb_AV_buf;
u8*  usb_AV_buf_end;

#if (USB_HOST == 1)
//u8* usb_qh_buf_1;
//u8* usb_qh_buf_2;
//u8* usb_qh_buf_3;
//u8* usb_qtd_buf;
//u8* usb_itd_buf_1;
//u32* usb_Page_buf_0;

/* Queue Head */
u8* qh_list_addr;
u8* qh_addr;
u8* hub_qh_addr;
u8* mouse_qh_addr;
u8* keyboard_qh_addr;

/* Queue Element Transfer Descriptor */
u8* qtd_addr[5];
u8* hub_qtd;
u8* mouse_qtd;
u8* keyboard_qtd;
u32* periodic_base;
//u8* test_buf;
#endif

#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
 u8* ImaAdpcmBuf;
 u8* ImaAdpcmBufEnd;
#endif

#ifdef OPCOM_JPEG_GUI
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

 u8 *sdcReadBuf;
 u8 *sdcWriteBuf;

 u8 *dcfBuf;
#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
 u8 *dcfLogBuf_Wr;
 u8 *dcfLogBuf_Rd;
#endif

#if SDC_ECC_DETECT
 u8 *FatDummyBuf;
#endif
u8 *sizeTblBase;
u8 *H1test1;
u8 *H1test2;

u8* OSD_buf;
u8* OSD_buf1; // A1018B mouse use Amon (140718)
u32 OSD_buf1_Size;

u8* SPIConfigBuf;
u8* iduvideobuff;
 u8 *iisSounBuf[IIS_BUF_NUM];

u8 *iotcBuf; /*for ALL IOTCAPIs use*/
// for new tutk lib, must allocate this
#if (LWIP2_SUPPORT == 1)
void SetLwipRamHeap(u8 *);
unsigned int ram_heap_size(void);    
u8 *lwipbuf;
u8 *lwipbuf2;
u8 *lwipbuf3;
#elif (ICOMMWIFI_SUPPORT == 1)
u8 *icommbuf;
u8 *icommbuf2;
u8 *icommbuf3;
__align(4) u8 *icommbuf4;
#endif

#if IIS_TEST
  u8 *iisBuf_play;
#elif IIS_4CH_REC_TEST
  u8 *iisBuf_play;
  u8 *iisBuf_play1;
  u8 *iisBuf_play2;
  u8 *iisBuf_play3;
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
 u8 sysVideoInCHsel;
 u8 sysDualModeDisp;
#if HWPIP_SUPPORT
u8 sysOsdInCHsel;
#endif

 u8 sysRFRxInMainCHsel;
 u8 sysRFRxInPIPCHsel;  //for PIP sub-channel
 u8 sysRFRXPIP_en;

 u8 sysRF_PTZ_CHsel;
 u8 sysRF_AudioChSw_DualMode;


 u8 sysPIPMain  = PIP_MAIN_NONE;

 OS_EVENT* isuSemEvt;
 u8	*pucClockImageBuf;
 RTC_DATE_TIME g_LocalTime;
 u32 g_LocalTimeInSec;
#if UART_GPS_COMMAND
 u8 g_updatetimeFlag=0;
#endif
 //================== Scalar overlay image ================//
#if ISU_OVERLAY_ENABLE
 u32 *ScalarOverlayImage;
#endif

 u32 *CiuOverlayImg1_Top;
 u32 *CiuOverlayImg1_Bot;

 u32 *CiuOverlayImg2_Top;
 u32 *CiuOverlayImg2_Bot;

 u32 *CiuOverlayImg3_Top;
 u32 *CiuOverlayImg3_Bot;

 u32 *CiuOverlayImg4_Top;
 u32 *CiuOverlayImg4_Bot;

 u32 *CiuOverlayImg5_Top;
 u32 *CiuOverlayImg5_Bot;

 u32 *CiuOverlayImg1_SP_Top;
 u32 *CiuOverlayImg1_SP_Bot;

 u32 *CiuOverlayImg2_SP_Top;
 u32 *CiuOverlayImg2_SP_Bot;

 u32 *CiuOverlayImg5_SP_Top;
 u32 *CiuOverlayImg5_SP_Bot;

 u32 OS_tickcounter=0;

#ifdef MMU_SUPPORT
 unsigned int *mmu_tlb_base;
 unsigned int *mmu_L1_base;
 u8* FS_internal_mem;
 #endif

u16 nand_fat_start, nand_fat_end ,nand_fat_size,nand_data_start;

 #if (TUTK_SUPPORT)
u8* p2plocal_buffer;
u8* p2pbusylocal_buffer;
//for event list use. by aher
u8* p2pEventList;
 #endif

#if USB_DONGLE_SUPPORT
u8* usbfwupgrade_buf;
#endif

#if PIC_OP
    u8* picTmpBuf;
#endif

u32 gfuSwQueWrIdx;
u32 gfuSwQueRdIdx;
u8 *gfuSwQueAddr;

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
s8* VMDMotionPos_X;
s8* VMDMotionPos_Y;
#if 0
u8* VMDMotionBuf_X[10];
u8* VMDMotionBuf_Y[10];
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
#ifdef DRAW_MEMORY_POOL
#define MAX_MEMORY_BLOCK_ENTRY   128
#define MAX_MEMORY_OVERLAP_ENTRY MAX_MEMORY_BLOCK_ENTRY
#define MAX_MEMORY_UNKNOWN_ENTRY MAX_MEMORY_BLOCK_ENTRY
#define MAX_MEMORY_ZERO_ENTRY    MAX_MEMORY_BLOCK_ENTRY
#define DRAW_MEMORY_BLOCK_UNIT   256*1024

typedef struct
{
   char name[32];
   u8 *start_addr;
   u8 *end_addr;
} MEMORY_BLOCK;

MEMORY_BLOCK memory_block[MAX_MEMORY_BLOCK_ENTRY];
MEMORY_BLOCK memory_overlap[MAX_MEMORY_OVERLAP_ENTRY];  //Lsk: overlap
MEMORY_BLOCK memory_unknown[MAX_MEMORY_UNKNOWN_ENTRY];  //Lsk: undefine block size
MEMORY_BLOCK memory_zero[MAX_MEMORY_ZERO_ENTRY];        //Lsk: define block size 0


int memory_block_idx=0;
int memory_overlap_idx=0;
int memory_unknown_idx=0;
int memory_zero_idx=0;
u32 memory_pool_bottom_addr = MEMORY_POOL_START_ADDR;

u8 DumpMemoryOverlapInfo(void)
{
	int i,j ;
	DEBUG_MAIN("\n\n\n@@@@@@@@@@@@@@@ memory block, overlap info @@@@@@@@@@@@@@@\n");	
	for(i=0; i<memory_overlap_idx; i++)
	{
		DEBUG_MAIN("%-36s, 0x%08x~0x%08x\n", memory_overlap[i].name, memory_overlap[i].start_addr, memory_overlap[i].end_addr); //Cross-block
		for(j=0;j<memory_block_idx;j++)
		{
			if(((memory_block[j].start_addr >= memory_overlap[i].start_addr) && (memory_block[j].start_addr <= memory_overlap[i].end_addr))
			 ||((memory_block[j].end_addr >= memory_overlap[i].start_addr) && (memory_block[j].end_addr <= memory_overlap[i].end_addr))
				)
				DEBUG_MAIN("                          overlap =>, 0x%08x~0x%08x, %s, \n", memory_block[j].start_addr, memory_block[j].end_addr, memory_block[j].name);			

			if((memory_block[j].start_addr <= memory_overlap[i].start_addr) && (memory_block[j].end_addr >= memory_overlap[i].end_addr))							
				DEBUG_MAIN("                          overlap =>, 0x%08x~0x%08x, %s, \n", memory_block[j].start_addr, memory_block[j].end_addr, memory_block[j].name);			
		}
	}
	return 0;			
}

u8 DumpMemoryUnknownInfo(void)
{
	int i;
	DEBUG_MAIN("\n\n\n@@@@@@@@@@@@@@@ memory block, size unknown info @@@@@@@@@@@@@@@\n");	
	for(i=0; i<memory_unknown_idx; i++)
	{
		DEBUG_MAIN("%-36s, 0x%08x~~~~~~~~\n", memory_unknown[i].name, memory_unknown[i].start_addr);
	}
	return 0;		
}

u8 DumpMemoryZeroInfo(void)
{
	int i;
	DEBUG_MAIN("\n\n\n@@@@@@@@@@@@@@@ memory block, size zero info @@@@@@@@@@@@@@@\n");	
	for(i=0; i<memory_zero_idx; i++)
	{
		DEBUG_MAIN("%-36s, 0x%08x~~~~~~~~\n", memory_zero[i].name, memory_zero[i].start_addr);
	}
	return 0;		
}

u8 DrawMemoryBlock(u8 *start_addr, u32 size, char* name, u8 unknown)
{
	int count=0;
	int i=0;

	if(unknown == 1)   //Lsk: undefine block size
	{
		memory_unknown[memory_unknown_idx].start_addr = start_addr;
		strcpy(memory_unknown[memory_unknown_idx].name, name);
		memory_unknown_idx++;
		//DEBUG_MAIN("#################### undefine\n");
	}	
	else if(size == 0) //Lsk: define block size 0
	{
		memory_zero[memory_zero_idx].start_addr = start_addr;
		strcpy(memory_zero[memory_zero_idx].name, name);
		memory_zero_idx++;
		//DEBUG_MAIN("#################### zero\n");		
	}
	else
	{
		if((u32)start_addr < memory_pool_bottom_addr) //Lsk: overlap
		{
			memory_overlap[memory_overlap_idx].start_addr = start_addr;
			memory_overlap[memory_overlap_idx].end_addr   = (u8*)((u32)start_addr+size);
			strcpy(memory_overlap[memory_overlap_idx].name, name);
			memory_overlap_idx++;
			//DEBUG_MAIN("#################### overlap\n");					
		}
		else
		{	
			if((u32)start_addr > memory_pool_bottom_addr) //Lsk: memory leak
			{
				count = (((u32)(start_addr-memory_pool_bottom_addr) + (DRAW_MEMORY_BLOCK_UNIT-1)) & ~(DRAW_MEMORY_BLOCK_UNIT-1))/(DRAW_MEMORY_BLOCK_UNIT);				
				DEBUG_MAIN("--------------------------------------\n");
				DEBUG_MAIN("|%-36s| -> Begin:0x%08x\n", "Memory leak", memory_pool_bottom_addr);
				for(i=1; i<count-1; i++)
				{
					DEBUG_MAIN("|                                    |\n");				
				}
				DEBUG_MAIN("|                                    | -> End  :0x%08x, size=%08d\n", (u32)start_addr, (u32)start_addr-memory_pool_bottom_addr);

				memory_block[memory_block_idx].start_addr = (u8*)(memory_pool_bottom_addr);
				memory_block[memory_block_idx].end_addr   = start_addr;
				strcpy(memory_block[memory_block_idx].name, "Memory leak");
				memory_block_idx++;
				memory_pool_bottom_addr = (u32)start_addr;
				//DEBUG_MAIN("#################### leak\n");						
			}
			
			
			count = (((u32)size + (DRAW_MEMORY_BLOCK_UNIT-1)) & ~(DRAW_MEMORY_BLOCK_UNIT-1))/(DRAW_MEMORY_BLOCK_UNIT);
			
			DEBUG_MAIN("--------------------------------------\n");
			DEBUG_MAIN("|%-36s| -> Begin:0x%08x\n", name, start_addr);
			for(i=1; i<count-1; i++)
			{
				DEBUG_MAIN("|                                    |\n");
			}
			DEBUG_MAIN("|                                    | -> End  :0x%08x, size=%08d\n", (u32)start_addr+size, size);

			memory_block[memory_block_idx].start_addr = start_addr;
			memory_block[memory_block_idx].end_addr   = (u8*)((u32)start_addr+size);
			strcpy(memory_block[memory_block_idx].name, name);
			memory_block_idx++;

			memory_pool_bottom_addr += size;
			//DEBUG_MAIN("#################### block\n");		
		}
	}
	return 0;		
}

u8 DrawMemoryPoolEnd(void)
{
	int i=0, count=0, size=0;

	if (memory_pool_bottom_addr > DRAM_MEMORY_END)
		DEBUG_MAIN("-------------------------------------- -> Over My Body\n");
	else
	{
		size = 4 + DRAM_MEMORY_END - memory_pool_bottom_addr;
		count = (((u32)size + (DRAW_MEMORY_BLOCK_UNIT-1)) & ~(DRAW_MEMORY_BLOCK_UNIT-1))/(DRAW_MEMORY_BLOCK_UNIT);
		DEBUG_MAIN("--------------------------------------\n");
		DEBUG_MAIN("|%-36s| -> Begin:0x%08x\n", "Memory leak", memory_pool_bottom_addr);
		for(i=1; i<count-1; i++)
		{
			DEBUG_MAIN("|                                    |\n");
		}
		DEBUG_MAIN("-------------------------------------- -> End  :0x%08x, size=%08d\n", DRAM_MEMORY_END+4, size);
	}
}
#else
u8 DumpMemoryOverlapInfo(void)
{
	return 0;
}

u8 DumpMemoryUnknownInfo(void)
{
	return 0;
}

u8 DumpMemoryZeroInfo(void)
{
	return 0;
}

u8 DrawMemoryBlock(u8 *start_addr, u32 size, char* name, u8 unknown)
{
	return 0;
}

u8 DrawMemoryPoolEnd(void)
{
	return 0;
}
#endif
u8 *initMemoryPool(u8 *addr)  //Lucian 070419
{
    int                 i, j;
    u8                  *Mpeg_addr;
#if MULTI_CHANNEL_VIDEO_REC
    VIDEO_CLIP_OPTION   *pVideoClipOption;
#endif
    u8                  *addr_temp;
    u8                  *addr_pip;
    u8                  *Mpeg_addr_temp;
#if LWIP2_SUPPORT    
    u8 *__lwipbuf = 0, *__lwipbuf2 = 0,	*__lwipbuf3 = 0;
#endif    

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
    sysVolumnControl        = 10;
    sysIsAC97Playing        = 0;
    sysVideoInCHsel         = 0;
  #if MULTI_CHANNEL_SEL
    i=0;
    while( (MULTI_CHANNEL_SEL & (1<<sysVideoInCHsel))==0 )
    {
       sysVideoInCHsel = (sysVideoInCHsel+1) % MULTI_CHANNEL_MAX;
       if(i>32)
       {
            DEBUG_WARERR("####################################################\r\n");
            DEBUG_WARERR("##  MULTI_CHANNEL_MAX less then MULTI_CHANNEL_SEL ##\r\n");
            DEBUG_WARERR("####################################################\r\n");
            while(1);
       }
       i++;
    }
  #endif
#if GET_SIU_RAWDATA_PURE
    siuAdjustSW             = 80;// 80:ligixbox,micro-lens@office
    siuAdjustAGC            = 8;
#endif

    //========================Memory Allocat Area=======================//
    //----MMU use----//
    addr_temp               = addr;
#ifdef MMU_SUPPORT
	DrawMemoryBlock(addr, SZ_16K, "mmu_tlb_base", 0);
    mmu_tlb_base            = (u32*)addr;   // Translation table base is start from memorypool
    addr                   += SZ_16K;       // The MMU table need SZ_16K (Max)    
	#ifdef COARSE_PAGE
	DrawMemoryBlock(addr, SZ_2K, "mmu_L1_base", 0);
    mmu_L1_base             = (u32*)addr;
    addr                   += SZ_2K;        // 2K for 2MB second descriptor    
  	#endif
	DrawMemoryBlock(addr, FS_MEMBLOCK_NUM*SMC_MAX_PAGE_SIZE, "FS_internal_mem", 0);
    FS_internal_mem         = addr;
    addr                   += FS_MEMBLOCK_NUM*SMC_MAX_PAGE_SIZE;
#endif

 #ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: MMU %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
 #endif


    //---NAND flash use---//
    addr_temp               = addr;
#if ( (FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
	DrawMemoryBlock(addr, SMC_MAX_MBR_PAGES*SMC_MAX_PAGE_SIZE, "smcMBRCache", 0);
    smcMBRCache             = addr; //SMC_MAX_FAT_PAGES
    addr                   += SMC_MAX_MBR_PAGES*SMC_MAX_PAGE_SIZE;

	DrawMemoryBlock(addr, SMC_MAX_PAGE_SIZE * SMC_MAX_PAGE_PER_BLOCK, "smcGeneralBuf", 0);
    smcGeneralBuf           = addr;
    addr                   += SMC_MAX_PAGE_SIZE * SMC_MAX_PAGE_PER_BLOCK;
#endif
   #ifndef DRAW_MEMORY_POOL
     DEBUG_MAIN("addr:0x%08x~0x%08x: NAND flash %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
   #endif
   //------TUTK buffer------//
   addr_temp			   = addr;
#if (TUTK_SUPPORT)    
    addr 				   = (u8 *)(((u32)addr + 63) & ~63);
    DrawMemoryBlock(addr, (IOTC_BUF_SIZE), "iotcBuf", 0);
    iotcBuf				   = (u8 *)addr;
    addr 				  += (IOTC_BUF_SIZE);
#endif
#if (LWIP2_SUPPORT == 1)// for new tutk lib, must allocate this
	//-------icomm buffer------------------//
		addr_temp				= addr;
		//addr					= (u8 *)(((u32)addr + 63) & ~63);
		__lwipbuf				= (u8 *)addr;
		addr					+= LWIP_ALL_BLK_SIZE; // Sean 20170426
		printf("addr:0x%08x~0x%08x: lwip buffer %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
	
	//-------icomm buffer2------------------//
		addr_temp				= addr;
		//addr					= (u8 *)(((u32)addr + 63) & ~63);
		__lwipbuf2				= (u8 *)addr;
		addr					+= LWIP2_ALL_BLK_SIZE;
		printf("addr:0x%08x~0x%08x: lwip buffer2 %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
	
	//-------icomm buffer3------------------// for iperf3  20160824 by aher
		addr_temp				= addr;
		//addr					= (u8 *)(((u32)addr + 63) & ~63);
		__lwipbuf3				= (u8 *)addr;
		addr					+= LWIP3_ALL_BLK_SIZE; // Sean 20170426
        SetLwipBuf(__lwipbuf, __lwipbuf2, __lwipbuf3);
		printf("addr:0x%08x~0x%08x: lwip buffer3 %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);        
        if(ram_heap_size() > 0) //move PBUF_RAM to MemoryPool?
        {
            addr_temp = addr;
            memset(addr, 0, ram_heap_size());
            SetLwipRamHeap(addr);        
            addr += ram_heap_size();
            printf("lwip ram heap 0x%x size = %d\n", addr_temp, ram_heap_size());
        }
#elif (ICOMMWIFI_SUPPORT == 1)
	//-------icomm buffer------------------//
		addr_temp				= addr;
		//addr					= (u8 *)(((u32)addr + 63) & ~63);
		icommbuf				= (u8 *)addr;
		//addr					+= (48*2560);
		addr					+= (8*2560); // Sean 20170426
		printf("addr:0x%08x~0x%08x: icomm buffer %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
	
	
	//-------icomm buffer2------------------//
		addr_temp				= addr;
		//addr					= (u8 *)(((u32)addr + 63) & ~63);
		icommbuf2				= (u8 *)addr;
		addr					+= (1024*256);
		printf("addr:0x%08x~0x%08x: icomm buffer2 %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
	
	//-------icomm buffer3------------------// for iperf3  20160824 by aher
		addr_temp				= addr;
		//addr					= (u8 *)(((u32)addr + 63) & ~63);
		icommbuf3				= (u8 *)addr;
		//addr					+= (10*4096*3);
		addr					+= (10*4096*6); // Sean 20170426
		printf("addr:0x%08x~0x%08x: icomm buffer3 %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
	//-------icomm buffer4------------------// for iperf3  20160824 by aher
		addr_temp				= addr;
		addr					= (u8 *)(((u32)addr + 63) & ~63);
		icommbuf4				= (u8 *)addr;
		addr					+= 4096;
		printf("addr:0x%08x~0x%08x: icomm buffer4 %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif	

    //----IPU block buffer, use in SIU/IPU/ISU capture mode----//
    addr_temp               = addr;
#if((MULTI_CHANNEL_SEL & 0x01) )
	DrawMemoryBlock(addr, (IPU_LINE_SIZE * 16 * 2), "ipuDstBuf0", 0);
    ipuDstBuf0              = (u8 *)addr;
    addr                   += (IPU_LINE_SIZE * 16 * 2);

	DrawMemoryBlock(addr, (IPU_LINE_SIZE * 16 * 2), "ipuDstBuf1", 0);
    ipuDstBuf1              = (u8 *)addr;
    addr                   += (IPU_LINE_SIZE * 16 * 2);

	DrawMemoryBlock(addr, (IPU_LINE_SIZE * 16 * 2), "ipuDstBuf2", 0);
    ipuDstBuf2              = (u8 *)addr;
    addr                   += (IPU_LINE_SIZE * 16 * 2);
#else
	DrawMemoryBlock(addr, 0, "ipuDstBuf0", 0);
    ipuDstBuf0              = (u8 *)addr;
    addr                   += 0;

	DrawMemoryBlock(addr, 0, "ipuDstBuf1", 0);
    ipuDstBuf1              = (u8 *)addr;
    addr                   += 0;

	DrawMemoryBlock(addr, 0, "ipuDstBuf2", 0);
    ipuDstBuf2              = (u8 *)addr;
    addr                   += 0;
#endif

 #ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: IPU %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
 #endif
    //---2D Graphic command Que---//       
    addr  = (u8 *)(((u32)addr + 63) & ~63);
    addr_temp               = addr;

	DrawMemoryBlock(addr, GFU_SWQUE_CMDMAX * 16 * 4, "gfuSwQueAddr", 0);
    gfuSwQueAddr            = addr;
    addr                   += GFU_SWQUE_CMDMAX * 16 * 4;
 #ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: 2D-GFU %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
 #endif

#if USB_DONGLE_SUPPORT
    addr                    = (u8 *)(((u32)addr + 63) & ~63);
	DrawMemoryBlock(addr, 1024*1024*1, "usbfwupgrade_buf", 0);
    usbfwupgrade_buf        =  addr;
	addr                    += 1024*1024;
#endif


    //---- IDU OSD use ----//
    addr  = (u8 *)(((u32)addr + 63) & ~63);
    addr_temp               = addr;
 #if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
       (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
       (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD)  || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) ||\
       (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) ||\
       (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_CIU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_SIU) || (SW_APPLICATION_OPTION  == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) ||\
       (SW_APPLICATION_OPTION == Standalone_Test) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W) ||\
       (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)) //no playback fun.
    //=== TX ===//
    DrawMemoryBlock(addr, 0, "OSD_buf", 0);
    OSD_buf  = addr;

    if ((TV_MAXOSD_SizeX * TV_MAXOSD_SizeY)> (OSD_SizeX * OSD_SizeY))
        addr   += 0;
    else
        addr   += 0;

 	DrawMemoryBlock(addr, 0, "OSD_buf1", 0);
    OSD_buf1 = addr;
    #if IDU_OSD_TEST
        addr   += 0;
        OSD_buf1_Size = 0;
    #else
        addr   += 0; // mouse use.FPGA limit 80x80 Amon (140718)
        OSD_buf1_Size = 0;
    #endif
#else
    OSD_buf  = addr;

  #if(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
        addr   += 0;
  #else
    if ((TV_MAXOSD_SizeX * TV_MAXOSD_SizeY)> (OSD_SizeX * OSD_SizeY))
        addr   += TVOSD_SizeX * TVOSD_SizeY;
    else
        addr   += OSD_SizeX * OSD_SizeY;
  #endif
	DrawMemoryBlock(OSD_buf, (u32)addr-(u32)OSD_buf, "OSD_buf", 0);

    OSD_buf1 = addr;
    #if IDU_OSD_TEST
        addr   += TVOSD_SizeX * TVOSD_SizeY;
        OSD_buf1_Size = TVOSD_SizeX * TVOSD_SizeY;
    #else
        addr   += 80 * 80; // mouse use.FPGA limit 80x80 Amon (140718)
        OSD_buf1_Size = 80 * 80;
    #endif
	DrawMemoryBlock(OSD_buf1, (u32)addr-(u32)OSD_buf1, "OSD_buf1", 0);
#endif
	DrawMemoryBlock(addr, SPI_BUF_SIZE, "SPIConfigBuf", 0);
    SPIConfigBuf = addr;
    addr   += SPI_BUF_SIZE;

#ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: IDU OSD %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif

//==============iHome ===========//
#if(HOME_RF_SUPPORT)
	DrawMemoryBlock(addr, 0x8000, "gHomeRFSensorList", 0);    	   	      					                        	   			   						        									
    gHomeRFSensorList=(HOMERF_SENSOR_LIST  *)addr;
    addr +=0x8000;  /* 32KB*/

	DrawMemoryBlock(addr, 0x2800, "gHomeRFRoomList", 0);    	   	      					                        	   			   						        									
    gHomeRFRoomList  =(HOMERF_ROOM_LIST    *)addr;
    addr +=0x2800;  /* 10KB*/

	DrawMemoryBlock(addr, 0x2800, "gHomeRFSceneList", 0);    	   	      					                        	   			   						        										
    gHomeRFSceneList =(HOMERF_SCENE_LIST   *)addr;
    addr +=0x2800;  /* 10KB*/
#endif

#ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: TUTK %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif

    //==================P2P Buffer=====================//
    // adjust the 640*480*2 to 1920*1080*2 buffer size to adapte 9200 FHD raw data
    addr_temp               = addr;
#if (TUTK_SUPPORT)
   #if( (SW_APPLICATION_OPTION  == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
	DrawMemoryBlock(addr, 1920*1080/2, "p2plocal_buffer", 0);
    p2plocal_buffer=addr;
	addr +=1920*1080/2;
   #elif( (SW_APPLICATION_OPTION  == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
	DrawMemoryBlock(addr, 1920*1080/2, "p2plocal_buffer", 0);
    p2plocal_buffer=addr;
	addr +=1920*1080/2;
	DrawMemoryBlock(addr, 320*240*2, "p2pbusylocal_buffer", 0);
    p2pbusylocal_buffer=addr;
	addr +=320*240*2;  
   #elif(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
	DrawMemoryBlock(addr, 1920*1080/2, "p2plocal_buffer", 0);
    p2plocal_buffer=addr;
	addr +=1920*1080/2;
	DrawMemoryBlock(addr, 320*240*2, "p2pbusylocal_buffer", 0);
    p2pbusylocal_buffer=addr;
	addr +=320*240*2;  
   #else
	DrawMemoryBlock(addr, 1920*1080*2, "p2plocal_buffer", 0);
    p2plocal_buffer=addr;
	addr +=1920*1080*2;
	DrawMemoryBlock(addr, 320*240*2, "p2pbusylocal_buffer", 0);
    p2pbusylocal_buffer=addr;
	addr +=320*240*2;
   #endif
	DrawMemoryBlock(addr, 1024, "p2pEventList", 0);
    p2pEventList=addr;
	addr +=1024;
#endif


#ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: P2P %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif

    //===================Mask area buff===================//
    addr_temp               = addr;
#if CIU_MASKAREA_TEST	
    addr= (u8 *)(((u32)addr + 255) & 0xffffff00); //64 word alignment
    DrawMemoryBlock(addr, (640 *480)/8/16, "MaskAreaBuf", 0);
    MaskAreaBuf    = addr;
    addr                   += (640 *480)/8/16;
#endif

#ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: Mask area %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif

    //===================PIC_OP buff===================//
    addr_temp               = addr;
#if PIC_OP
  #if(SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)
    addr = (u8 *)(((u32)addr + 31) & 0xffffffe0); // 32 bytes alignment
    DrawMemoryBlock(addr, 0, "picTmpBuf", 0);
    picTmpBuf = (u8 *)addr;
    addr += 0;
  #else
    addr = (u8 *)(((u32)addr + 31) & 0xffffffe0); // 32 bytes alignment
    DrawMemoryBlock(addr, 1280*720*3/2, "picTmpBuf", 0);
    picTmpBuf = (u8 *)addr;
    addr += 1280*720*3/2;
  #endif  
#endif

#ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: PIC_OP %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif

    //-----Display buffer in preview mode for YUV420 mode-----//
    addr= (u8 *)(((u32)addr + 255) & 0xffffff00); //64 word alignment
    addr_temp               = addr;
 #if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
       (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) ||\
       (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
       (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_CIU) ||\
       (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_SIU) || (SW_APPLICATION_OPTION  == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION  == MR9100_WIFI_DONGLE_AVSED_RX1) ||\
 	(SW_APPLICATION_OPTION == MR8202_AN_KLF08W) ||  (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)) //no playback fun.

 	DrawMemoryBlock(addr, 0, "PKBuf0", 0);
    PKBuf0                  = (u8 *)addr;
    addr                   += 0;

	DrawMemoryBlock(addr, 0, "PKBuf1", 0);
    PKBuf1                  = (u8 *)addr;
    addr                   += 0;

	DrawMemoryBlock(addr, 0, "PKBuf2", 0);
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
		{
			char tmp[64]={0};
			sprintf(tmp, "MainVideodisplaybuf%d", i);
			DrawMemoryBlock(MainVideodisplaybuf[i], 0, tmp, 0);
        }
    }
    #if TX_FW_UPDATE_SUPPORT
      #if( (SW_APPLICATION_OPTION  == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) )
      rfiuTXFwUpdBuf= addr;
      addr += 1024*1024;
      #else
      rfiuTXFwUpdBuf=MainVideodisplaybuf[DISPLAY_BUF_NUM-1];  //for TX fw update use
      #endif
    #endif
 #elif( (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) ||\
 		(SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
 	DrawMemoryBlock(addr, 0, "PKBuf0", 0);
    PKBuf0                  = (u8 *)addr;
    addr                   += VIDEODISPBUF_SIZE;

	DrawMemoryBlock(addr, 0, "PKBuf1", 0);
    PKBuf1                  = (u8 *)PKBuf0;
    addr                   += 0;

	DrawMemoryBlock(addr, 0, "PKBuf2", 0);
    PKBuf2                  = (u8 *)PKBuf0;
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
            MainVideodisplaybuf[i]  = PKBuf0;
            addr          += 0;
        }
		{
			char tmp[64]={0};
			sprintf(tmp, "MainVideodisplaybuf%d", i);
			DrawMemoryBlock(MainVideodisplaybuf[i], 0, tmp, 0);
        }
    }
    #if TX_FW_UPDATE_SUPPORT
      rfiuTXFwUpdBuf= addr;
      addr += 1024*1024;
    #endif
 #else
    //----//
    DrawMemoryBlock(addr, VIDEODISPBUF_SIZE, "PKBuf0", 0);
    PKBuf0                  = (u8 *)addr;
    addr                   += (VIDEODISPBUF_SIZE);

	DrawMemoryBlock(addr, VIDEODISPBUF_SIZE, "PKBuf1", 0);
    PKBuf1                  = (u8 *)addr;
    addr                   += (VIDEODISPBUF_SIZE);

	DrawMemoryBlock(addr, VIDEODISPBUF_SIZE, "PKBuf2", 0);
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

		{
			char tmp[64]={0};
			sprintf(tmp, "MainVideodisplaybuf%d", i);
			DrawMemoryBlock(MainVideodisplaybuf[i], VIDEODISPBUF_SIZE, tmp, 0);
        }
    }
	#if ( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
	MainVideodisplaybuf[DISPLAY_BUF_NUM]  = addr;
    addr          += VIDEODISPBUF_SIZE;
	{
		char tmp[64]={0};
		sprintf(tmp, "MainVideodisplaybuf%d", DISPLAY_BUF_NUM);
		DrawMemoryBlock(MainVideodisplaybuf[DISPLAY_BUF_NUM], VIDEODISPBUF_SIZE, tmp, 0);
    }

	MainVideodisplaybuf[DISPLAY_BUF_NUM+1]  = addr;
    addr          += VIDEODISPBUF_SIZE;
	{
		char tmp[64]={0};
		sprintf(tmp, "MainVideodisplaybuf%d", DISPLAY_BUF_NUM+1);
		DrawMemoryBlock(MainVideodisplaybuf[DISPLAY_BUF_NUM+1], VIDEODISPBUF_SIZE, tmp, 0);
    }
	#endif
	#if( ( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) ) && (USE_NEW_MEMORY_MAP)) 
	//Playback bitstream buffer
	addr                      = (u8 *)(((u32)addr + 63) & ~63);
	DrawMemoryBlock(addr, MPEG4_MAX_BUF_SIZE, "Playback VideoBuf", 0);           	    				   		
    VideoBuf         =(u8 *)addr; // 1MB
    addr                     +=MPEG4_MAX_BUF_SIZE;
	
	DrawMemoryBlock(addr, 0, "Playback mpeg4VideBufEnd", 1);           	    				   		
    mpeg4VideBufEnd      = addr;
	#endif
    
    #if TX_FW_UPDATE_SUPPORT
      rfiuTXFwUpdBuf=MainVideodisplaybuf[DISPLAY_BUF_NUM-1];  //for TX fw update use
    #endif
    
#endif


  #ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: Display buffer %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
  #endif
  
#if H1_264TEST_ENC
  //-------H1 H264 use--------//
    addr= (u8 *)(((u32)addr + 255) & 0xffffff00); //64 word alignment
    addr_temp               = addr;

	DrawMemoryBlock(addr, VIDEODISPBUF_SIZE, "sizeTblBase", 0);    
    sizeTblBase                  = (u8 *)addr;
    addr                   += (VIDEODISPBUF_SIZE);

    DrawMemoryBlock(addr, VIDEODISPBUF_SIZE, "sizeTblBase", 0);    
    H1test1                  = (u8 *)addr;
    addr                   += (VIDEODISPBUF_SIZE);

    DrawMemoryBlock(addr, VIDEODISPBUF_SIZE, "sizeTblBase", 0);    
    H1test2                  = (u8 *)addr;
    addr                   += (VIDEODISPBUF_SIZE);

 #ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: H1 H264 %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
 #endif
#endif
 

  //-------File system use--------//
#if(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) ||\
   (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
   (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION  == MR9100_WIFI_DONGLE_AVSED_RX1) ||\
   (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
   (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

    addr_temp               = addr;
   #if SDC_ECC_DETECT
	DrawMemoryBlock(addr, DCF_BUF_SIZE, "FatDummyBuf", 0);    
    FatDummyBuf             = (u8 *)addr;
    addr                   += 0;
   #endif

   #if SD_CARD_DISABLE

   #else
	DrawMemoryBlock(addr, DCF_BUF_SIZE, "FSBuf", 0);    
	FSBuf         			= (u8 *)addr;
    addr                   += 0;    
   #endif
   
	DrawMemoryBlock(addr, DCF_BUF_SIZE, "dcfBuf", 0);    
    dcfBuf                  = (u8 *)addr;
    addr                   += 0;


  #if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
	DrawMemoryBlock(addr, (DCF_LOGBUF_SIZE+1024), "dcfLogBuf_Wr", 0);    
    dcfLogBuf_Wr            = (u8 *)addr;
    addr                   += 0;

	DrawMemoryBlock(addr, (DCF_LOGBUF_SIZE), "dcfLogBuf_Rd", 0);    	
    dcfLogBuf_Rd             = (u8 *)addr;;
    addr                   += 0;
  #endif  
#else
    addr_temp               = addr;
   #if SDC_ECC_DETECT
	DrawMemoryBlock(addr, DCF_BUF_SIZE, "FatDummyBuf", 0);    
    FatDummyBuf             = (u8 *)addr;
    addr                   += (DCF_BUF_SIZE);
   #endif

   #if SD_CARD_DISABLE

   #else
	DrawMemoryBlock(addr, DCF_BUF_SIZE, "FSBuf", 0);    
	FSBuf         			= (u8 *)addr;
    addr                   += (FS_V_MEMBUF_SIZE * FS_V_TOTAL_CACHE_BLOCK);    
   #endif
   
	DrawMemoryBlock(addr, DCF_BUF_SIZE, "dcfBuf", 0);    
    dcfBuf                  = (u8 *)addr;
    addr                   += (DCF_BUF_SIZE);

  #if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
	DrawMemoryBlock(addr, (DCF_LOGBUF_SIZE+1024), "dcfLogBuf_Wr", 0);    
    dcfLogBuf_Wr            = (u8 *)addr;
    addr                   += (DCF_LOGBUF_SIZE+1024);

	DrawMemoryBlock(addr, (DCF_LOGBUF_SIZE), "dcfLogBuf_Rd", 0);    	
    dcfLogBuf_Rd             = (u8 *)addr;;
    addr                   += (DCF_LOGBUF_SIZE);
  #endif
#endif

 #ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: File system %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
 #endif

    //----------------- ----------------------------RFIU  buffer ----------------------------------------//
    addr= (u8 *)(((u32)addr + 8191) & 0xffffe000); //must be 8KB alignment.
    addr_temp               = addr;
#if RFIU_SUPPORT
   //---------------------Mpeg decoding buffer for RF-RX -------------------------------//
   //Lucian: 共用Current reconstructed frame
   //====RF-0====//
   #if ( (SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHDIN_AVSED_RX1) ||\
         (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) ||\
         (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) ||\
         (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W)) // 與DISPBUF共用 reconstructed frame
        addr                      = (u8 *)(((u32)addr + 63) & ~63);
   		DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[0].mpeg4MVBuf", 0);
        rfiuRxDecBuf[0].mpeg4MVBuf              = (u8 *)addr;
        addr                   += 0;
        
        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[0].mpeg4PRefBuf_Y", 0);
        rfiuRxDecBuf[0].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += 0;

		DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[0].mpeg4PRefBuf_Cb", 0);
        rfiuRxDecBuf[0].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += 0;

		DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[0].mpeg4PRefBuf_Cr", 0);
        rfiuRxDecBuf[0].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

		DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[0].mpeg4NRefBuf_Y", 0);
        rfiuRxDecBuf[0].mpeg4NRefBuf_Y          = (u8 *)addr;
        addr                   += 0;

		DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[0].mpeg4NRefBuf_Cb", 0);
        rfiuRxDecBuf[0].mpeg4NRefBuf_Cb         = (u8 *)addr;
        addr                   += 0;

		DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[0].mpeg4NRefBuf_Cr", 0);
        rfiuRxDecBuf[0].mpeg4NRefBuf_Cr         = (u8 *)addr;
        addr                   += 0;
   #else
       #if (RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
        addr                      = (u8 *)(((u32)addr + 63) & ~63);
	    DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 63) & ~63), "rfiuRxDecBuf[0].mpeg4MVBuf", 0);
        rfiuRxDecBuf[0].mpeg4MVBuf              = (u8 *)addr;	//Lsk: remove 
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

		#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && (USE_NEW_MEMORY_MAP))
		rfiuRxDecBuf[0].mpeg4PRefBuf_Y         = MainVideodisplaybuf[1];
		rfiuRxDecBuf[0].mpeg4PRefBuf_Cb		   = MainVideodisplaybuf[1]+(ULTRA_FHD_SIZE_Y);
        rfiuRxDecBuf[0].mpeg4PRefBuf_Cr		   = MainVideodisplaybuf[1]+(ULTRA_FHD_SIZE_Y+ULTRA_FHD_SIZE_C/2);
		#elif(((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
		rfiuRxDecBuf[0].mpeg4PRefBuf_Y         = MainVideodisplaybuf[1];
		rfiuRxDecBuf[0].mpeg4PRefBuf_Cb		   = MainVideodisplaybuf[1]+((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));
        rfiuRxDecBuf[0].mpeg4PRefBuf_Cr		   = MainVideodisplaybuf[1]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0))+(((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0))/2);
		#else
	    DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)), "rfiuRxDecBuf[0].mpeg4PRefBuf_Y", 0);        
        rfiuRxDecBuf[0].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));

	    DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[0].mpeg4PRefBuf_Cb", 0);        
        rfiuRxDecBuf[0].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

		DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[0].mpeg4PRefBuf_Cr", 0);        
        rfiuRxDecBuf[0].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        #endif

		
		#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && (USE_NEW_MEMORY_MAP))
		rfiuRxDecBuf[0].mpeg4NRefBuf_Y          = MainVideodisplaybuf[5];
		rfiuRxDecBuf[0].mpeg4NRefBuf_Cb         = MainVideodisplaybuf[5]+(ULTRA_FHD_SIZE_Y);
        rfiuRxDecBuf[0].mpeg4NRefBuf_Cr         = MainVideodisplaybuf[5]+(ULTRA_FHD_SIZE_Y+ULTRA_FHD_SIZE_C/2);
		#elif(((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
		rfiuRxDecBuf[0].mpeg4NRefBuf_Y          = MainVideodisplaybuf[5];
		rfiuRxDecBuf[0].mpeg4NRefBuf_Cb         = MainVideodisplaybuf[5]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)));
        rfiuRxDecBuf[0].mpeg4NRefBuf_Cr         = MainVideodisplaybuf[5]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0))+(((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0))/2);
		#else
		DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)), "rfiuRxDecBuf[0].mpeg4NRefBuf_Y", 0);        
        rfiuRxDecBuf[0].mpeg4NRefBuf_Y          = (u8 *)addr;		
		addr                   += ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));
		
		DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[0].mpeg4NRefBuf_Cb", 0);        		
        rfiuRxDecBuf[0].mpeg4NRefBuf_Cb         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));
		
		DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[0].mpeg4NRefBuf_Cr", 0);        		
        rfiuRxDecBuf[0].mpeg4NRefBuf_Cr         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));
		#endif
        
       #endif
   #endif
   //===RF-1===//
   #if( (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test)||\
   		(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) )
       #if (RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)
        addr                      = (u8 *)(((u32)addr + 63) & ~63);
    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[1].mpeg4MVBuf", 0);           
    	DrawMemoryBlock(addr, 0, "VideoBuf", 0);           
        rfiuRxDecBuf[1].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        VideoBuf = (u8 *)addr;
    	
        addr                   += 0;
        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[1].mpeg4PRefBuf_Y", 0);           
        rfiuRxDecBuf[1].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += 0;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[1].mpeg4PRefBuf_Cb", 0);           
        rfiuRxDecBuf[1].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += 0;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[1].mpeg4PRefBuf_Cr", 0);           
        rfiuRxDecBuf[1].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    	
    	DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	
        mpeg4VideBufEnd = (u8 *)addr;
    	

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[1].mpeg4NRefBuf_Y", 1);           	
        rfiuRxDecBuf[1].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[1].mpeg4NRefBuf_Cb", 1);           	
        rfiuRxDecBuf[1].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[1].mpeg4NRefBuf_Cr", 1);           	
        rfiuRxDecBuf[1].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
       #endif

   #else
       #if (RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
        addr                      = (u8 *)(((u32)addr + 63) & ~63);
    	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 63) & ~63), "rfiuRxDecBuf[1].mpeg4MVBuf", 0);           
    	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 63) & ~63), "VideoBuf", 0);           
        rfiuRxDecBuf[1].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
    	#if(USE_NEW_MEMORY_MAP)
    	#else
    	VideoBuf = (u8 *)addr;
    	#endif
    	#elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
    	#if(USE_NEW_MEMORY_MAP)
    	#else
    	VideoBuf = (u8 *)addr;
    	#endif
    	#else
        VideoBuf = (u8 *)addr;
    	#endif
    	
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    	#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[1].mpeg4PRefBuf_Y         = MainVideodisplaybuf[2];
    	rfiuRxDecBuf[1].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[2]+(ULTRA_FHD_SIZE_Y);
        rfiuRxDecBuf[1].mpeg4PRefBuf_Cr        = MainVideodisplaybuf[2]+(ULTRA_FHD_SIZE_Y+ULTRA_FHD_SIZE_C/2);
    	#elif( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[1].mpeg4PRefBuf_Y         = MainVideodisplaybuf[2];
    	rfiuRxDecBuf[1].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[2]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)));
        rfiuRxDecBuf[1].mpeg4PRefBuf_Cr        = MainVideodisplaybuf[2]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0))+(((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0))/2);
    	#else
        DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)), "rfiuRxDecBuf[1].mpeg4PRefBuf_Y", 0);           
        rfiuRxDecBuf[1].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));

        DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[1].mpeg4PRefBuf_Cb", 0);           
        rfiuRxDecBuf[1].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

        DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[1].mpeg4PRefBuf_Cr", 0);           
        rfiuRxDecBuf[1].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    	#endif
    	
    	#if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
    	#if(USE_NEW_MEMORY_MAP)
    	#else
    	DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	
        mpeg4VideBufEnd = (u8 *)addr;
    	#endif
    	#elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
    	#if(USE_NEW_MEMORY_MAP)
    	#else
    	DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	
        mpeg4VideBufEnd = (u8 *)addr;
    	#endif
    	#else	
    	DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	
        mpeg4VideBufEnd = (u8 *)addr;
    	#endif
    	

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[1].mpeg4NRefBuf_Y", 1);           	
        rfiuRxDecBuf[1].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[1].mpeg4NRefBuf_Cb", 1);           	
        rfiuRxDecBuf[1].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[1].mpeg4NRefBuf_Cr", 1);           	
        rfiuRxDecBuf[1].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
       #endif
   #endif
   
   //===RF-2,RF-3===//
   #if( (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test)||\
   		(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) )
       #if (RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)
       
        addr                      = (u8 *)(((u32)addr + 63) & ~63);
    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[2].mpeg4MVBuf", 0);           	
        rfiuRxDecBuf[2].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[2].mpeg4PRefBuf_Y", 0);           	    
        rfiuRxDecBuf[2].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += 0;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[2].mpeg4PRefBuf_Cb", 0);           	    	
        rfiuRxDecBuf[2].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += 0;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[2].mpeg4PRefBuf_Cr", 0);           	    		
        rfiuRxDecBuf[2].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[2].mpeg4NRefBuf_Y", 1);           	
        rfiuRxDecBuf[2].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[2].mpeg4NRefBuf_Cb", 1);           	
        rfiuRxDecBuf[2].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[2].mpeg4NRefBuf_Cr", 1);           	
        rfiuRxDecBuf[2].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
        
        //===for PIP:目前沒有用到===//
    	addr                      = (u8 *)(((u32)addr + 63) & ~63);
        addr_pip=addr;
      	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP0Y", 0);           	    		
        PKBuf_PIP0Y             = (u8 *)addr_pip;
        addr_pip                   += 0;

    	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP0CbCr", 0);           	    		
        PKBuf_PIP0CbCr          = (u8 *)addr_pip;
        addr_pip                   += 0;

    	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP1Y", 0);           	    		
        PKBuf_PIP1Y             = (u8 *)addr_pip;
        addr_pip                   += 0;
    	
    	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP1CbCr", 0);           	    		
        PKBuf_PIP1CbCr          = (u8 *)addr_pip;
        addr_pip                   += 0;

    	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP2Y", 0);           	    		
        PKBuf_PIP2Y             = (u8 *)addr_pip;
        addr_pip                   += 0;

    	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP2CbCr", 0);           	    		
        PKBuf_PIP2CbCr          = (u8 *)addr_pip;
        addr_pip                   += 0;
        //-------------//
        
      	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[3].mpeg4MVBuf", 0);           	    		
        rfiuRxDecBuf[3].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

       	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[3].mpeg4PRefBuf_Y", 0);           	    		
        rfiuRxDecBuf[3].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += 0;

       	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[3].mpeg4PRefBuf_Cb", 0);           	    		
        rfiuRxDecBuf[3].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += 0;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[3].mpeg4PRefBuf_Cr", 0);           	    				
        rfiuRxDecBuf[3].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        
    	DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	    				
        mpeg4VideBufEnd = (u8 *)addr;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[3].mpeg4NRefBuf_Y", 1);
    	rfiuRxDecBuf[3].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
        //addr                   += ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32));

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[3].mpeg4NRefBuf_Cb", 1);
        rfiuRxDecBuf[3].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
        //addr                   += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[3].mpeg4NRefBuf_Cr", 1);
        rfiuRxDecBuf[3].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
        //addr                   += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));
       #endif

   #else
       #if (RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)
        addr                      = (u8 *)(((u32)addr + 63) & ~63);
    	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 63) & ~63), "rfiuRxDecBuf[2].mpeg4MVBuf", 0);           	
        rfiuRxDecBuf[2].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    	#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[2].mpeg4PRefBuf_Y         = MainVideodisplaybuf[3];
    	rfiuRxDecBuf[2].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[3]+(ULTRA_FHD_SIZE_Y);
        rfiuRxDecBuf[2].mpeg4PRefBuf_Cr		   = MainVideodisplaybuf[3]+(ULTRA_FHD_SIZE_Y+ULTRA_FHD_SIZE_C/2);
    	#elif( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[2].mpeg4PRefBuf_Y         = MainVideodisplaybuf[3];
    	rfiuRxDecBuf[2].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[3]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)));
        rfiuRxDecBuf[2].mpeg4PRefBuf_Cr		   = MainVideodisplaybuf[3]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0))+(((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0))/2);	
    	#else
    	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)), "rfiuRxDecBuf[2].mpeg4PRefBuf_Y", 0);           	    
        rfiuRxDecBuf[2].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));

    	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[2].mpeg4PRefBuf_Cb", 0);           	    	
        rfiuRxDecBuf[2].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

    	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[2].mpeg4PRefBuf_Cr", 0);           	    		
        rfiuRxDecBuf[2].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    	#endif
    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[2].mpeg4NRefBuf_Y", 1);           	
        rfiuRxDecBuf[2].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[2].mpeg4NRefBuf_Cb", 1);           	
        rfiuRxDecBuf[2].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[2].mpeg4NRefBuf_Cr", 1);           	
        rfiuRxDecBuf[2].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
        //----------//
        //===for PIP:目前沒有用到===//
    	addr                      = (u8 *)(((u32)addr + 63) & ~63);
        addr_pip=addr;
      #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)  //PIP 先暫用reconstructed frame 4
      	DrawMemoryBlock(addr_pip, (320*176), "PKBuf_PIP0Y", 0);           	    		
        PKBuf_PIP0Y             = (u8 *)addr_pip;  	
        addr_pip                   += (320*176);

    	DrawMemoryBlock(addr_pip, (320*176)/2, "PKBuf_PIP0CbCr", 0);           	    		
        PKBuf_PIP0CbCr          = (u8 *)addr_pip;
    	addr_pip                   += (320*176)/2;
    	
    	DrawMemoryBlock(addr_pip, (640*352), "PKBuf_PIP1Y", 0);           	    		    
        PKBuf_PIP1Y             = (u8 *)addr_pip;
        addr_pip                   += (640*352);
    	
    	DrawMemoryBlock(addr_pip, (640*352)/2, "PKBuf_PIP1CbCr", 0);           	    		
        PKBuf_PIP1CbCr          = (u8 *)addr_pip;
        addr_pip                   += (640*352)/2;
    	
    	DrawMemoryBlock(addr_pip, (640*352), "PKBuf_PIP2Y", 0);           	    		
        PKBuf_PIP2Y             = (u8 *)addr_pip;
        addr_pip                   += (640*352);
    	
    	DrawMemoryBlock(addr_pip, (640*352)/2, "PKBuf_PIP2CbCr", 0);           	    		
        PKBuf_PIP2CbCr          = (u8 *)addr_pip;
        addr_pip                   += (640*352)/2;
    	
      #else
      	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP0Y", 0);           	    		
        PKBuf_PIP0Y             = (u8 *)addr_pip;
        addr_pip                   += 0;

    	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP0CbCr", 0);           	    		
        PKBuf_PIP0CbCr          = (u8 *)addr_pip;
        addr_pip                   += 0;

    	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP1Y", 0);           	    		
        PKBuf_PIP1Y             = (u8 *)addr_pip;
        addr_pip                   += 0;
    	
    	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP1CbCr", 0);           	    		
        PKBuf_PIP1CbCr          = (u8 *)addr_pip;
        addr_pip                   += 0;

    	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP2Y", 0);           	    		
        PKBuf_PIP2Y             = (u8 *)addr_pip;
        addr_pip                   += 0;

    	DrawMemoryBlock(addr_pip, 0, "PKBuf_PIP2CbCr", 0);           	    		
        PKBuf_PIP2CbCr          = (u8 *)addr_pip;
        addr_pip                   += 0;
      #endif

      	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 63) & ~63), "rfiuRxDecBuf[3].mpeg4MVBuf", 0);           	    		
        rfiuRxDecBuf[3].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    	#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[3].mpeg4PRefBuf_Y         = MainVideodisplaybuf[4];
    	rfiuRxDecBuf[3].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[4]+(ULTRA_FHD_SIZE_Y);
        rfiuRxDecBuf[3].mpeg4PRefBuf_Cr        = MainVideodisplaybuf[4]+(ULTRA_FHD_SIZE_Y+ULTRA_FHD_SIZE_C/2);
    	#elif( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[3].mpeg4PRefBuf_Y         = MainVideodisplaybuf[4];
    	rfiuRxDecBuf[3].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[4]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)));
        rfiuRxDecBuf[3].mpeg4PRefBuf_Cr        = MainVideodisplaybuf[4]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0))+(((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0))/2);	
    	#else
       	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)), "rfiuRxDecBuf[3].mpeg4PRefBuf_Y", 0);           	    		
        rfiuRxDecBuf[3].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));

       	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[3].mpeg4PRefBuf_Cb", 0);           	    		
        rfiuRxDecBuf[3].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

    	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[3].mpeg4PRefBuf_Cr", 0);           	    				
        rfiuRxDecBuf[3].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        #endif
    	#if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
    	#if(USE_NEW_MEMORY_MAP)
    	#else
        DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	    				
        mpeg4VideBufEnd = (u8 *)addr;
    	#endif
    	#elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
    	#if(USE_NEW_MEMORY_MAP)
    	#else
        DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	    				
        mpeg4VideBufEnd = (u8 *)addr;
    	#endif
    	#else
    	DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	    				
        mpeg4VideBufEnd = (u8 *)addr;
    	#endif

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[3].mpeg4NRefBuf_Y", 1);
    	rfiuRxDecBuf[3].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
        //addr                   += ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32));

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[3].mpeg4NRefBuf_Cb", 1);
        rfiuRxDecBuf[3].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
        //addr                   += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[3].mpeg4NRefBuf_Cr", 1);
        rfiuRxDecBuf[3].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
        //addr                   += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));
       #endif
  #endif
    //===RF-4,RF-5,RF-6,RF-7===//
   #if( (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test) ||\
   		(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
       #if(RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)
        //--RF4--//
        addr                      = (u8 *)(((u32)addr + 63) & ~63);
    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4MVBuf", 0);           	
        rfiuRxDecBuf[4].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4PRefBuf_Y", 0);           	    
        rfiuRxDecBuf[4].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += 0;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4PRefBuf_Cb", 0);           	    	
        rfiuRxDecBuf[4].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += 0;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4PRefBuf_Cr", 0);           	    		
        rfiuRxDecBuf[4].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4NRefBuf_Y", 1);           	
        rfiuRxDecBuf[4].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4NRefBuf_Cb", 1);           	
        rfiuRxDecBuf[4].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4NRefBuf_Cr", 1);           	
        rfiuRxDecBuf[4].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
        
        //---RF5---//
      	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4MVBuf", 0);           	    		
        rfiuRxDecBuf[5].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

       	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4PRefBuf_Y", 0);           	    		
        rfiuRxDecBuf[5].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += 0;

       	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4PRefBuf_Cb", 0);           	    		
        rfiuRxDecBuf[5].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += 0;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4PRefBuf_Cr", 0);           	    				
        rfiuRxDecBuf[5].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        
        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4NRefBuf_Y", 1);
    	rfiuRxDecBuf[5].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4NRefBuf_Cb", 1);
        rfiuRxDecBuf[5].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4NRefBuf_Cr", 1);
        rfiuRxDecBuf[5].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
        //--RF6--//
        addr                      = (u8 *)(((u32)addr + 63) & ~63);
    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[6].mpeg4MVBuf", 0);           	
        rfiuRxDecBuf[6].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[6].mpeg4PRefBuf_Y", 0);           	    
        rfiuRxDecBuf[6].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += 0;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[6].mpeg4PRefBuf_Cb", 0);           	    	
        rfiuRxDecBuf[6].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += 0;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[6].mpeg4PRefBuf_Cr", 0);           	    		
        rfiuRxDecBuf[6].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[6].mpeg4NRefBuf_Y", 1);           	
        rfiuRxDecBuf[6].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[6].mpeg4NRefBuf_Cb", 1);           	
        rfiuRxDecBuf[6].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[6].mpeg4NRefBuf_Cr", 1);           	
        rfiuRxDecBuf[6].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
        
        //--RF7--//
      	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[7].mpeg4MVBuf", 0);           	    		
        rfiuRxDecBuf[7].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

       	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[7].mpeg4PRefBuf_Y", 0);           	    		
        rfiuRxDecBuf[7].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += 0;

       	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[7].mpeg4PRefBuf_Cb", 0);           	    		
        rfiuRxDecBuf[7].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += 0;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[7].mpeg4PRefBuf_Cr", 0);           	    				
        rfiuRxDecBuf[7].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += 0;

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        
    	DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	    				
        mpeg4VideBufEnd = (u8 *)addr;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[7].mpeg4NRefBuf_Y", 1);
    	rfiuRxDecBuf[7].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[7].mpeg4NRefBuf_Cb", 1);
        rfiuRxDecBuf[7].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[7].mpeg4NRefBuf_Cr", 1);
        rfiuRxDecBuf[7].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;

        
       #endif

   #else
       #if(RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)   //Lucian:還未完成,待續
        //--RF4--//
        addr                      = (u8 *)(((u32)addr + 63) & ~63);
    	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 63) & ~63), "rfiuRxDecBuf[4].mpeg4MVBuf", 0);           	
        rfiuRxDecBuf[4].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    	#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[4].mpeg4PRefBuf_Y         = MainVideodisplaybuf[3];
    	rfiuRxDecBuf[4].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[3]+(ULTRA_FHD_SIZE_Y);
        rfiuRxDecBuf[4].mpeg4PRefBuf_Cr		   = MainVideodisplaybuf[3]+(ULTRA_FHD_SIZE_Y+ULTRA_FHD_SIZE_C/2);
    	#elif( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[4].mpeg4PRefBuf_Y         = MainVideodisplaybuf[3];
    	rfiuRxDecBuf[4].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[3]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)));
        rfiuRxDecBuf[4].mpeg4PRefBuf_Cr		   = MainVideodisplaybuf[3]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0))+(((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0))/2);	
    	#else
    	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)), "rfiuRxDecBuf[4].mpeg4PRefBuf_Y", 0);           	    
        rfiuRxDecBuf[4].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));

    	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[4].mpeg4PRefBuf_Cb", 0);           	    	
        rfiuRxDecBuf[4].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

    	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[4].mpeg4PRefBuf_Cr", 0);           	    		
        rfiuRxDecBuf[4].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    	#endif
    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4NRefBuf_Y", 1);           	
        rfiuRxDecBuf[4].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4NRefBuf_Cb", 1);           	
        rfiuRxDecBuf[4].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4NRefBuf_Cr", 1);           	
        rfiuRxDecBuf[4].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
        
        //--RF5--//
      	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 63) & ~63), "rfiuRxDecBuf[5].mpeg4MVBuf", 0);           	    		
        rfiuRxDecBuf[5].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    	#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[5].mpeg4PRefBuf_Y         = MainVideodisplaybuf[4];
    	rfiuRxDecBuf[5].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[4]+(ULTRA_FHD_SIZE_Y);
        rfiuRxDecBuf[5].mpeg4PRefBuf_Cr        = MainVideodisplaybuf[4]+(ULTRA_FHD_SIZE_Y+ULTRA_FHD_SIZE_C/2);
    	#elif( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[5].mpeg4PRefBuf_Y         = MainVideodisplaybuf[4];
    	rfiuRxDecBuf[5].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[4]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)));
        rfiuRxDecBuf[5].mpeg4PRefBuf_Cr        = MainVideodisplaybuf[4]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0))+(((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0))/2);	
    	#else
       	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)), "rfiuRxDecBuf[5].mpeg4PRefBuf_Y", 0);           	    		
        rfiuRxDecBuf[5].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));

       	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[5].mpeg4PRefBuf_Cb", 0);           	    		
        rfiuRxDecBuf[5].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

    	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[5].mpeg4PRefBuf_Cr", 0);           	    				
        rfiuRxDecBuf[5].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        #endif
        
        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4NRefBuf_Y", 1);
    	rfiuRxDecBuf[5].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4NRefBuf_Cb", 1);
        rfiuRxDecBuf[5].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4NRefBuf_Cr", 1);
        rfiuRxDecBuf[5].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;

        //--RF6--//
        addr                      = (u8 *)(((u32)addr + 63) & ~63);
    	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 63) & ~63), "rfiuRxDecBuf[6].mpeg4MVBuf", 0);           	
        rfiuRxDecBuf[6].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    	#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[6].mpeg4PRefBuf_Y         = MainVideodisplaybuf[3];
    	rfiuRxDecBuf[6].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[3]+(ULTRA_FHD_SIZE_Y);
        rfiuRxDecBuf[6].mpeg4PRefBuf_Cr		   = MainVideodisplaybuf[3]+(ULTRA_FHD_SIZE_Y+ULTRA_FHD_SIZE_C/2);
    	#elif( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[6].mpeg4PRefBuf_Y         = MainVideodisplaybuf[3];
    	rfiuRxDecBuf[6].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[3]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)));
        rfiuRxDecBuf[6].mpeg4PRefBuf_Cr		   = MainVideodisplaybuf[3]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0))+(((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0))/2);	
    	#else
    	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)), "rfiuRxDecBuf[6].mpeg4PRefBuf_Y", 0);           	    
        rfiuRxDecBuf[6].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));

    	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[6].mpeg4PRefBuf_Cb", 0);           	    	
        rfiuRxDecBuf[6].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

    	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[6].mpeg4PRefBuf_Cr", 0);           	    		
        rfiuRxDecBuf[6].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
    	#endif
    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4NRefBuf_Y", 1);           	
        rfiuRxDecBuf[6].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4NRefBuf_Cb", 1);           	
        rfiuRxDecBuf[6].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

    	DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[4].mpeg4NRefBuf_Cr", 1);           	
        rfiuRxDecBuf[6].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
        
        //--RF7--//
      	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 63) & ~63), "rfiuRxDecBuf[7].mpeg4MVBuf", 0);           	    		
        rfiuRxDecBuf[7].mpeg4MVBuf              = (u8 *)addr; //Lsk: remove 
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 63) & ~63);

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

    	#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[7].mpeg4PRefBuf_Y         = MainVideodisplaybuf[4];
    	rfiuRxDecBuf[7].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[4]+(ULTRA_FHD_SIZE_Y);
        rfiuRxDecBuf[7].mpeg4PRefBuf_Cr        = MainVideodisplaybuf[4]+(ULTRA_FHD_SIZE_Y+ULTRA_FHD_SIZE_C/2);
    	#elif( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
    	rfiuRxDecBuf[7].mpeg4PRefBuf_Y         = MainVideodisplaybuf[4];
    	rfiuRxDecBuf[7].mpeg4PRefBuf_Cb        = MainVideodisplaybuf[4]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)));
        rfiuRxDecBuf[7].mpeg4PRefBuf_Cr        = MainVideodisplaybuf[4]+(((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0))+(((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0))/2);	
    	#else
       	DrawMemoryBlock(addr, ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0)), "rfiuRxDecBuf[7].mpeg4PRefBuf_Y", 0);           	    		
        rfiuRxDecBuf[7].mpeg4PRefBuf_Y          = (u8 *)addr;
        addr                   += ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));

       	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[7].mpeg4PRefBuf_Cb", 0);           	    		
        rfiuRxDecBuf[7].mpeg4PRefBuf_Cb         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

    	DrawMemoryBlock(addr, (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0)), "rfiuRxDecBuf[7].mpeg4PRefBuf_Cr", 0);           	    				
        rfiuRxDecBuf[7].mpeg4PRefBuf_Cr         = (u8 *)addr;
        addr                   += (((RF_RX_DEC_WIDTH_MAX >> 1) + 0) * ((RF_RX_DEC_HEIGHT_MAX >> 1) + 0));

        addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        #endif
        
    	#if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
        	#if(USE_NEW_MEMORY_MAP)
        	#else
            DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	    				
            mpeg4VideBufEnd = (u8 *)addr;
        	#endif
    	#elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
        	#if(USE_NEW_MEMORY_MAP)
        	#else
            DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	    				
            mpeg4VideBufEnd = (u8 *)addr;
        	#endif
    	#else
        	DrawMemoryBlock(addr, 0, "mpeg4VideBufEnd", 1);           	    				
            mpeg4VideBufEnd = (u8 *)addr;
    	#endif

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4NRefBuf_Y", 1);
    	rfiuRxDecBuf[7].mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4NRefBuf_Cb", 1);
        rfiuRxDecBuf[7].mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;

        DrawMemoryBlock(addr, 0, "rfiuRxDecBuf[5].mpeg4NRefBuf_Cr", 1);
        rfiuRxDecBuf[7].mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
       #endif
  #endif
  
   //==========RF operation buffer=============//
   #if( (RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_RXRX_PROTOCOL_B1B2 ||\
         RFI_SELF_TEST_TXRX_PROTOCOL || RFI_TEST_TXRX_COMMU || RFI_TEST_PERFORMANCE || RFI_TEST_TXRX_FUN || RFI_TEST_PKTBURST || RFI_TEST_PKTMAP || RFI_MEASURE_RX1RX2_SENSITIVITY) )
	  DrawMemoryBlock(addr, ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) ) * RFI_PAYLOAD_SIZE), "rfiuOperBuf[0]", 0);           	    				   		
      rfiuOperBuf[0]            =(u8 *)addr;
      addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) ) * RFI_PAYLOAD_SIZE); // 1MB, command packet:64, dummy packet:64
      #if RFI_TEST_TX_PROTOCOL_B1
     	DrawMemoryBlock(addr, 0, "rfiuRxVideoBuf[0]", 0);           	    				   		
	    rfiuRxVideoBuf[0]         =(u8 *)addr; // 1MB
	    addr                     += 0;
     	DrawMemoryBlock(addr, 0, "rfiuRxVideoBufEnd[0]", 1);           	    				   		
	    rfiuRxVideoBufEnd[0]      =addr;

     	DrawMemoryBlock(addr, 0, "rfiuRxAudioBuf[0]", 0);           	    				   		
	    rfiuRxAudioBuf[0]         =(u8 *)addr;
	    addr                     += 0;
		DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[0]", 1);           	    				   		
	    rfiuRxAudioBufEnd[0]      =addr;
	  #else
	  	DrawMemoryBlock(addr, MPEG4_MAX_BUF_SIZE, "rfiuRxVideoBuf[0]", 0);           	    				   		
	    rfiuRxVideoBuf[0]         =(u8 *)addr; // 1MB
	    addr                     +=MPEG4_MAX_BUF_SIZE;
		DrawMemoryBlock(addr, 0, "rfiuRxVideoBufEnd[0]", 1);           	    				   		
	    rfiuRxVideoBufEnd[0]      =addr;

		#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (!USE_NEW_MEMORY_MAP))
		DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[0]", 0);           	    				   		
	    rfiuRxAudioBuf[0]         =(u8 *)addr;
	    addr                     += 64*1024;
		DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[0]", 1);           	    				   		
	    rfiuRxAudioBufEnd[0]      =addr;
        #elif 0 //(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH)
		DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[0]", 0);           	    				   		
	    rfiuRxAudioBuf[0]         =(u8 *)addr;
	    addr                     += 64*1024;
		DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[0]", 1);           	    				   		
		#else
		DrawMemoryBlock(addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "rfiuRxAudioBuf[0]", 0);           	    				   		
	    rfiuRxAudioBuf[0]         =(u8 *)addr;
	    addr                     += IIS_BUF_NUM*IIS_CHUNK_SIZE;
		DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[0]", 1);           	    				   		
	    rfiuRxAudioBufEnd[0]      =addr;
		#endif
		
        #if RX_SNAPSHOT_SUPPORT
        DrawMemoryBlock(addr, (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 4), "rfiuRxDataBuf[0]", 0);           	    				   		
	    rfiuRxDataBuf[0]         =(u8 *)addr;
	    addr                     += (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 8);
		DrawMemoryBlock(addr, 0, "rfiuRxDataBufEnd[0]", 1);           	    				   		
	    rfiuRxDataBufEnd[0]      =addr;
        #endif
	  #endif
   #else
   	  DrawMemoryBlock(addr, 0, "rfiuOperBuf[0]", 0);           	    				   		
      rfiuOperBuf[0]            = (u8 *)addr;
      addr                     += 0; // 1MB
   #endif

   #if(RFI_TEST_TX_PROTOCOL_B2 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL ||\
       RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_SELF_TEST_TXRX_PROTOCOL || RFI_TEST_TXRX_COMMU || RFI_TEST_PERFORMANCE || RFI_TEST_TXRX_FUN ||\
       RFI_TEST_PKTBURST || RFI_TEST_PKTMAP || RFI_MEASURE_RX1RX2_SENSITIVITY)
   	  DrawMemoryBlock(addr, ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE), "rfiuOperBuf[1]", 0);           	    				   		   	
      rfiuOperBuf[1]            = (u8 *)addr;
      addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE); // 1MB
      #if RFI_TEST_TX_PROTOCOL_B2
     	DrawMemoryBlock(addr, 0, "rfiuRxVideoBuf[1]", 0);           	    				   		   	
	    rfiuRxVideoBuf[1]         =(u8 *)addr; // 1MB
	    addr                     += 0;
     	DrawMemoryBlock(addr, 0, "rfiuRxVideoBufEnd[1]", 1);           	    				   		   	
	    rfiuRxVideoBufEnd[1]      =addr;

     	DrawMemoryBlock(addr, 0, "rfiuRxAudioBuf[1]", 0); 
	    rfiuRxAudioBuf[1]         =(u8 *)addr;
	    addr                     += 0;
     	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[1]", 1);           	    				   		   			
	    rfiuRxAudioBufEnd[1]      =addr;
	  #else
		DrawMemoryBlock(addr, MPEG4_MAX_BUF_SIZE, "rfiuRxVideoBuf[1]", 0);           	    				   		   		  
	    rfiuRxVideoBuf[1]         =(u8 *)addr; // 1MB
	    addr                     += MPEG4_MAX_BUF_SIZE;
		DrawMemoryBlock(addr, 0, "rfiuRxVideoBufEnd[1]", 1);           	    				   		   	
	    rfiuRxVideoBufEnd[1]      =addr;

		#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (!USE_NEW_MEMORY_MAP))
		DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[1]", 0); 
	    rfiuRxAudioBuf[1]         =(u8 *)addr;
	    addr                     += 64*1024;
		DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[1]", 1);           	    				   		   			
	    rfiuRxAudioBufEnd[1]      =addr;	
        #elif 0 //(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH)
		DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[1]", 0); 
	    rfiuRxAudioBuf[1]         =(u8 *)addr;
	    addr                     += 64*1024;
		DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[1]", 1);           	    				   		   			
	    rfiuRxAudioBufEnd[1]      =addr;	
        
		#else		
		DrawMemoryBlock(addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "rfiuRxAudioBuf[1]", 0); 
	    rfiuRxAudioBuf[1]         =(u8 *)addr;
	    addr                     += IIS_BUF_NUM*IIS_CHUNK_SIZE;
		DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[1]", 1);           	    				   		   			
	    rfiuRxAudioBufEnd[1]      =addr;
		#endif
		
        #if RX_SNAPSHOT_SUPPORT
        DrawMemoryBlock(addr, (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 4), "rfiuRxDataBuf[1]", 0);           	    				   		
	    rfiuRxDataBuf[1]         =(u8 *)addr;
	    addr                     += (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 8);
		DrawMemoryBlock(addr, 0, "rfiuRxDataBufEnd[1]", 1);           	    				   		
	    rfiuRxDataBufEnd[1]      =addr;
        #endif
	  #endif
   #else
   	DrawMemoryBlock(addr, 0, "rfiuOperBuf[1]", 0); 
    rfiuOperBuf[1]            = (u8 *)addr;
    addr                     += 0; // 1MB
   #endif

   #if (RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
   	DrawMemoryBlock(addr, ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE), "rfiuOperBuf[2]", 0); 
    rfiuOperBuf[2]            = (u8 *)addr;
    addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE); // 1MB

   	DrawMemoryBlock(addr, MPEG4_MAX_BUF_SIZE, "rfiuRxVideoBuf[2]", 0); 
    rfiuRxVideoBuf[2]         =(u8 *)addr; // 1MB
    addr                     += MPEG4_MAX_BUF_SIZE;
   	DrawMemoryBlock(addr, 0, "rfiuRxVideoBufEnd[2]", 1); 
    rfiuRxVideoBufEnd[2]      =addr;

	#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (!USE_NEW_MEMORY_MAP))
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[2]", 0); 
        rfiuRxAudioBuf[2]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[2]", 1); 
        rfiuRxAudioBufEnd[2]      =addr;	
    
    #elif 0 //(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH)
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[2]", 0); 
        rfiuRxAudioBuf[2]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[2]", 1); 
        rfiuRxAudioBufEnd[2]      =addr;	
    
	#else
       	DrawMemoryBlock(addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "rfiuRxAudioBuf[2]", 0); 
        rfiuRxAudioBuf[2]         =(u8 *)addr;
        addr                     += IIS_BUF_NUM*IIS_CHUNK_SIZE;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[2]", 1); 
        rfiuRxAudioBufEnd[2]      =addr;
	#endif
	
    #if RX_SNAPSHOT_SUPPORT
        DrawMemoryBlock(addr, (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 4), "rfiuRxDataBuf[2]", 0);           	    				   		
        rfiuRxDataBuf[2]         =(u8 *)addr;
        addr                     += (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 8);
    	DrawMemoryBlock(addr, 0, "rfiuRxDataBufEnd[2]", 1);           	    				   		
        rfiuRxDataBufEnd[2]      =addr;
    #endif
    
   #else
   	DrawMemoryBlock(addr, 0, "rfiuOperBuf[2]", 0); 
    rfiuOperBuf[2]            = (u8 *)addr;
    addr                     += 0; // 1MB
   #endif

   #if(RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
	DrawMemoryBlock(addr, ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE), "rfiuOperBuf[3]", 0);    
    rfiuOperBuf[3]            = (u8 *)addr;
    addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE); // 1MB

	DrawMemoryBlock(addr, MPEG4_MAX_BUF_SIZE, "rfiuRxVideoBuf[3]", 0);    
    rfiuRxVideoBuf[3]         =(u8 *)addr; // 1MB
    addr                     += MPEG4_MAX_BUF_SIZE;
	DrawMemoryBlock(addr, 0, "rfiuRxVideoBufEnd[3]", 1);    
    rfiuRxVideoBufEnd[3]      =addr;

	#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (!USE_NEW_MEMORY_MAP))
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[3]", 0);    
        rfiuRxAudioBuf[3]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[3]", 1);    
        rfiuRxAudioBufEnd[3]      =addr;
    #elif 0 //(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH)
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[3]", 0);    
        rfiuRxAudioBuf[3]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[3]", 1);    
        rfiuRxAudioBufEnd[3]      =addr;
	#else
    	DrawMemoryBlock(addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "rfiuRxAudioBuf[3]", 0);    
        rfiuRxAudioBuf[3]         =(u8 *)addr;
        addr                     += IIS_BUF_NUM*IIS_CHUNK_SIZE;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[3]", 1);    
        rfiuRxAudioBufEnd[3]      =addr;
	#endif
	
    #if RX_SNAPSHOT_SUPPORT
    DrawMemoryBlock(addr, (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 4), "rfiuRxDataBuf[3]", 0);           	    				   		
    rfiuRxDataBuf[3]         =(u8 *)addr;
    addr                     += (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 8);
	DrawMemoryBlock(addr, 0, "rfiuRxDataBufEnd[3]", 1);           	    				   		
    rfiuRxDataBufEnd[3]      =addr;
    #endif
    
   #else
   	DrawMemoryBlock(addr, 0, "rfiuOperBuf[3]", 0);    
    rfiuOperBuf[3]            = (u8 *)addr;
    addr                     += 0; // 1MB
   #endif

   #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
    //--RF4--//
	DrawMemoryBlock(addr, ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE), "rfiuOperBuf[4]", 0);    
    rfiuOperBuf[4]            = (u8 *)addr;
    addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE); // 1MB

	DrawMemoryBlock(addr, MPEG4_MAX_BUF_SIZE, "rfiuRxVideoBuf[4]", 0);    
    rfiuRxVideoBuf[4]         =(u8 *)addr; // 1MB
    addr                     += MPEG4_MAX_BUF_SIZE;
	DrawMemoryBlock(addr, 0, "rfiuRxVideoBufEnd[4]", 1);    
    rfiuRxVideoBufEnd[4]      =addr;

	#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (!USE_NEW_MEMORY_MAP))
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[4]", 0);    
        rfiuRxAudioBuf[4]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[4]", 1);    
        rfiuRxAudioBufEnd[4]      =addr;
    #elif 0 //(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH)
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[4]", 0);    
        rfiuRxAudioBuf[4]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[4]", 1);    
        rfiuRxAudioBufEnd[4]      =addr;
	#else
    	DrawMemoryBlock(addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "rfiuRxAudioBuf[4]", 0);    
        rfiuRxAudioBuf[4]         =(u8 *)addr;
        addr                     += IIS_BUF_NUM*IIS_CHUNK_SIZE;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[4]", 1);    
        rfiuRxAudioBufEnd[4]      =addr;
	#endif
	
    #if RX_SNAPSHOT_SUPPORT
    DrawMemoryBlock(addr, (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 4), "rfiuRxDataBuf[4]", 0);           	    				   		
    rfiuRxDataBuf[4]         =(u8 *)addr;
    addr                     += (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 8);
	DrawMemoryBlock(addr, 0, "rfiuRxDataBufEnd[4]", 1);           	    				   		
    rfiuRxDataBufEnd[4]      =addr;
    #endif    
    
    //--RF5--//
	DrawMemoryBlock(addr, ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE), "rfiuOperBuf[5]", 0);    
    rfiuOperBuf[5]            = (u8 *)addr;
    addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE); // 1MB

	DrawMemoryBlock(addr, MPEG4_MAX_BUF_SIZE, "rfiuRxVideoBuf[5]", 0);    
    rfiuRxVideoBuf[5]         =(u8 *)addr; // 1MB
    addr                     += MPEG4_MAX_BUF_SIZE;
	DrawMemoryBlock(addr, 0, "rfiuRxVideoBufEnd[5]", 1);    
    rfiuRxVideoBufEnd[5]      =addr;

	#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (!USE_NEW_MEMORY_MAP))
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[5]", 0);    
        rfiuRxAudioBuf[5]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[5]", 1);    
        rfiuRxAudioBufEnd[5]      =addr;
    #elif 0 //(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH)
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[5]", 0);    
        rfiuRxAudioBuf[5]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[5]", 1);    
        rfiuRxAudioBufEnd[5]      =addr;
	#else
    	DrawMemoryBlock(addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "rfiuRxAudioBuf[5]", 0);    
        rfiuRxAudioBuf[5]         =(u8 *)addr;
        addr                     += IIS_BUF_NUM*IIS_CHUNK_SIZE;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[5]", 1);    
        rfiuRxAudioBufEnd[5]      =addr;
	#endif
	
    #if RX_SNAPSHOT_SUPPORT
    DrawMemoryBlock(addr, (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 4), "rfiuRxDataBuf[5]", 0);           	    				   		
    rfiuRxDataBuf[5]         =(u8 *)addr;
    addr                     += (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 8);
	DrawMemoryBlock(addr, 0, "rfiuRxDataBufEnd[5]", 1);           	    				   		
    rfiuRxDataBufEnd[5]      =addr;
    #endif    

    //--RF6--//
	DrawMemoryBlock(addr, ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE), "rfiuOperBuf[6]", 0);    
    rfiuOperBuf[6]            = (u8 *)addr;
    addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE); // 1MB

	DrawMemoryBlock(addr, MPEG4_MAX_BUF_SIZE, "rfiuRxVideoBuf[6]", 0);    
    rfiuRxVideoBuf[6]         =(u8 *)addr; // 1MB
    addr                     += MPEG4_MAX_BUF_SIZE;
	DrawMemoryBlock(addr, 0, "rfiuRxVideoBufEnd[6]", 1);    
    rfiuRxVideoBufEnd[6]      =addr;

	#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (!USE_NEW_MEMORY_MAP))
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[6]", 0);    
        rfiuRxAudioBuf[6]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[6]", 1);    
        rfiuRxAudioBufEnd[6]      =addr;
    #elif 0 //(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH)
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[6]", 0);    
        rfiuRxAudioBuf[6]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[6]", 1);    
        rfiuRxAudioBufEnd[6]      =addr;
	#else
    	DrawMemoryBlock(addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "rfiuRxAudioBuf[6]", 0);    
        rfiuRxAudioBuf[6]         =(u8 *)addr;
        addr                     += IIS_BUF_NUM*IIS_CHUNK_SIZE;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[6]", 1);    
        rfiuRxAudioBufEnd[6]      =addr;
	#endif
	
    #if RX_SNAPSHOT_SUPPORT
    DrawMemoryBlock(addr, (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 4), "rfiuRxDataBuf[6]", 0);           	    				   		
    rfiuRxDataBuf[6]         =(u8 *)addr;
    addr                     += (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 8);
	DrawMemoryBlock(addr, 0, "rfiuRxDataBufEnd[6]", 1);           	    				   		
    rfiuRxDataBufEnd[6]      =addr;
    #endif    

    //--RF7--//
	DrawMemoryBlock(addr, ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE), "rfiuOperBuf[7]", 0);    
    rfiuOperBuf[7]            = (u8 *)addr;
    addr                     += ((RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT+1+1+9) )* RFI_PAYLOAD_SIZE); // 1MB

	DrawMemoryBlock(addr, MPEG4_MAX_BUF_SIZE, "rfiuRxVideoBuf[7]", 0);    
    rfiuRxVideoBuf[7]         =(u8 *)addr; // 1MB
    addr                     += MPEG4_MAX_BUF_SIZE;
	DrawMemoryBlock(addr, 0, "rfiuRxVideoBufEnd[7]", 1);    
    rfiuRxVideoBufEnd[7]      =addr;

	#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (!USE_NEW_MEMORY_MAP))
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[7]", 0);    
        rfiuRxAudioBuf[7]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[7]", 1);    
        rfiuRxAudioBufEnd[7]      =addr;
    #elif 0 //(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH)
    	DrawMemoryBlock(addr, 64*1024, "rfiuRxAudioBuf[7]", 0);    
        rfiuRxAudioBuf[7]         =(u8 *)addr;
        addr                     += 64*1024;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[7]", 1);    
        rfiuRxAudioBufEnd[7]      =addr;
	#else
    	DrawMemoryBlock(addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "rfiuRxAudioBuf[7]", 0);    
        rfiuRxAudioBuf[7]         =(u8 *)addr;
        addr                     += IIS_BUF_NUM*IIS_CHUNK_SIZE;
    	DrawMemoryBlock(addr, 0, "rfiuRxAudioBufEnd[7]", 1);    
        rfiuRxAudioBufEnd[7]      =addr;
	#endif
	
    #if RX_SNAPSHOT_SUPPORT
    DrawMemoryBlock(addr, (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 4), "rfiuRxDataBuf[7]", 0);           	    				   		
    rfiuRxDataBuf[7]         =(u8 *)addr;
    addr                     += (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 8);
	DrawMemoryBlock(addr, 0, "rfiuRxDataBufEnd[7]", 1);           	    				   		
    rfiuRxDataBufEnd[7]      =addr;
    #endif    

   #endif

   #if(  (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) )
    for(i = 0; i < DISPLAY_BUF_NUM; i++)
    {
  	   char tmp[64]={0};
	   sprintf(tmp, "Sub1Videodisplaybuf%d", i);	

	   DrawMemoryBlock(addr, VIDEODISPBUF_SIZE, tmp, 0);    
	   Sub1Videodisplaybuf[i] = (u8 *)addr;
       addr                  += VIDEODISPBUF_SIZE;
    }
   #endif

#endif
//-----------------------------------------------------------------------------// 
#ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: RFIU %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif
    //----Jpeg thumbnail bitstream buffer----//
    addr_temp               = addr;
#if( (SW_APPLICATION_OPTION == MR6730_CARDVR_2CH) || (SW_APPLICATION_OPTION == MR6730_CARDVR_1CH) || (SW_APPLICATION_OPTION == MR9670_DOORPHONE) )
	DrawMemoryBlock(addr, EXIF_THUMBNAIL_MAX, "exifThumbnailBitstream", 0);    
    exifThumbnailBitstream  = (u8 *)addr;
    addr                   += EXIF_THUMBNAIL_MAX;

	DrawMemoryBlock(addr, IMG_MAX_WIDTH *IMG_MAX_HEIGHT, "exifPrimaryBitstream", 0);    
    exifPrimaryBitstream    = addr;
    addr                   += IMG_MAX_WIDTH *IMG_MAX_HEIGHT;

#else
	DrawMemoryBlock(addr, 0, "exifThumbnailBitstream", 0);    
    exifThumbnailBitstream  = (u8 *)addr;
    addr                   += 0;

	DrawMemoryBlock(addr, 0, "exifPrimaryBitstream", 0);    
    exifPrimaryBitstream    = addr;
    addr                   += 0;
#endif


#ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: Jpeg thumbnail bitstream %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif

    //----IIS test used-----//
#if IIS_TEST	
    addr                   += (44);//wave header
    DrawMemoryBlock(addr, (IIS_BUF_SIZ* IIS_REC_TIME), "iisBuf_play", 0);    	
    iisBuf_play             = (u8 *)addr;
    addr                   += IIS_BUF_SIZ* IIS_REC_TIME;
#elif IIS_4CH_REC_TEST
    addr                   += (44);//wave header
    DrawMemoryBlock(addr, (IIS_BUF_SIZ* IIS_REC_TIME), "iisBuf_play", 0);    	
    iisBuf_play             = (u8 *)addr;
    addr                   += IIS_BUF_SIZ* IIS_REC_TIME;
	
    addr                   += (44);//wave header
    DrawMemoryBlock(addr, (IIS_BUF_SIZ* IIS_REC_TIME), "iisBuf_play1", 0);    	
    iisBuf_play1             = (u8 *)addr;
    addr                   += IIS_BUF_SIZ* IIS_REC_TIME;
	
    addr                   += (44);//wave header
    DrawMemoryBlock(addr, (IIS_BUF_SIZ* IIS_REC_TIME), "iisBuf_play2", 0);    	
    iisBuf_play2             = (u8 *)addr;
    addr                   += IIS_BUF_SIZ* IIS_REC_TIME;
	
    addr                   += (44);//wave header
    DrawMemoryBlock(addr, (IIS_BUF_SIZ* IIS_REC_TIME), "iisBuf_play3", 0);    	
    iisBuf_play3             = (u8 *)addr;
    addr                   += IIS_BUF_SIZ* IIS_REC_TIME;

#endif

    //-------- Scalar overlay image(Front end OSD) -------//
   addr= (u8 *)(((u32)addr + 255) & 0xffffff00); //64 word alignment
   addr_temp               = addr;
#if(MULTI_CHANNEL_SEL & 0x01)
  #if ISU_OVERLAY_ENABLE
   DrawMemoryBlock(addr, 640 * 48 * 2, "ScalarOverlayImage", 0);    	
   ScalarOverlayImage      = (u32 *)addr;
   addr				   += 640 * 48 * 2;
  #endif
#else
  #if ISU_OVERLAY_ENABLE
   DrawMemoryBlock(addr, 0, "ScalarOverlayImage", 0);    	
   ScalarOverlayImage      = (u32 *)addr;
   addr				   += 0;
  #endif
#endif

#if(MULTI_CHANNEL_SEL & 0x02)
   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg1_Top", 0);    	
   CiuOverlayImg1_Top = (u32 *)addr;
   addr += 640 * 80;

   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg1_Bot", 0);    	   
   CiuOverlayImg1_Bot = (u32 *)addr;
   addr += 640 * 80;

   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg1_SP_Top", 0);    	   	
   CiuOverlayImg1_SP_Top = (u32 *)addr;
   addr += 640 * 80;

   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg1_SP_Bot", 0);    	   	   
   CiuOverlayImg1_SP_Bot = (u32 *)addr;
   addr += 640 * 80;
#else
   DrawMemoryBlock(addr, 0, "CiuOverlayImg1_Top", 0);    		
   CiuOverlayImg1_Top = (u32 *)addr;
   addr += 0;

   DrawMemoryBlock(addr, 0, "CiuOverlayImg1_Bot", 0);    	   
   CiuOverlayImg1_Bot = (u32 *)addr;
   addr += 0;

   DrawMemoryBlock(addr, 0, "CiuOverlayImg1_SP_Top", 0);    	   		
   CiuOverlayImg1_SP_Top = (u32 *)addr;
   addr += 0;

   DrawMemoryBlock(addr, 0, "CiuOverlayImg1_SP_Bot", 0);    	   	      
   CiuOverlayImg1_SP_Bot = (u32 *)addr;
   addr += 0;
#endif

#if(MULTI_CHANNEL_SEL & 0x04)
   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg2_Top", 0);    	   	      
   CiuOverlayImg2_Top = (u32 *)addr;
   addr += 640 * 80;

   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg2_Bot", 0);    	   	      
   CiuOverlayImg2_Bot = (u32 *)addr;
   addr += 640 * 80;

   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg2_SP_Top", 0);    	   	      
   CiuOverlayImg2_SP_Top = (u32 *)addr;
   addr += 640 * 80;
   
   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg2_SP_Bot", 0);    	   	         
   CiuOverlayImg2_SP_Bot = (u32 *)addr;
   addr += 640 * 80;
#else
   DrawMemoryBlock(addr, 0, "CiuOverlayImg2_Top", 0);    	   	      
   CiuOverlayImg2_Top = (u32 *)addr;
   addr += 0;

   DrawMemoryBlock(addr, 0, "CiuOverlayImg2_Bot", 0);    	   	      
   CiuOverlayImg2_Bot = (u32 *)addr;
   addr += 0;

    DrawMemoryBlock(addr, 0, "CiuOverlayImg2_SP_Top", 0);    	   	      	
   CiuOverlayImg2_SP_Top = (u32 *)addr;
   addr += 0;

   DrawMemoryBlock(addr, 0, "CiuOverlayImg2_SP_Bot", 0);    	   	         
   CiuOverlayImg2_SP_Bot = (u32 *)addr;
   addr += 0;
#endif

#if(MULTI_CHANNEL_SEL & 0x08)
   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg3_Top", 0);    	   	      
   CiuOverlayImg3_Top = (u32 *)addr;
   addr += 640 * 80;

   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg3_Bot", 0);    	   	      
   CiuOverlayImg3_Bot = (u32 *)addr;
   addr += 640 * 80;
#else
   DrawMemoryBlock(addr, 0, "CiuOverlayImg3_Top", 0);    	   	      
   CiuOverlayImg3_Top = (u32 *)addr;
   addr += 0;
   
   DrawMemoryBlock(addr, 0, "CiuOverlayImg3_Bot", 0);    	   	         
   CiuOverlayImg3_Bot = (u32 *)addr;
   addr += 0;
#endif

#if(MULTI_CHANNEL_SEL & 0x10)
	DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg4_Top", 0);    	   	      
   CiuOverlayImg4_Top = (u32 *)addr;
   addr += 640 * 80;

   	DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg4_Bot", 0);    	   	      
   CiuOverlayImg4_Bot = (u32 *)addr;
   addr += 640 * 80;
#else
	DrawMemoryBlock(addr, 0, "CiuOverlayImg4_Top", 0);    	   	      
   CiuOverlayImg4_Top = (u32 *)addr;
   addr += 0;

   	DrawMemoryBlock(addr, 0, "CiuOverlayImg4_Bot", 0);    	   	      
   CiuOverlayImg4_Bot = (u32 *)addr;
   addr += 0;
#endif

#if(MULTI_CHANNEL_SEL & 0x20)
	DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg5_Top", 0);    	   	      
   CiuOverlayImg5_Top = (u32 *)addr;
   addr += 640 * 80;

   	DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg5_Bot", 0);    	   	      
   CiuOverlayImg5_Bot = (u32 *)addr;
   addr += 640 * 80;

	DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg5_SP_Top", 0);    	   	      
   CiuOverlayImg5_SP_Top = (u32 *)addr;
   addr += 640 * 80;

   DrawMemoryBlock(addr, 640 * 80, "CiuOverlayImg5_SP_Bot", 0);    	   	      
   CiuOverlayImg5_SP_Bot = (u32 *)addr;
   addr += 640 * 80;
#else
	DrawMemoryBlock(addr, 0, "CiuOverlayImg5_Top", 0);    	   	      
   CiuOverlayImg5_Top = (u32 *)addr;
   addr += 0;

   	DrawMemoryBlock(addr, 0, "CiuOverlayImg5_Bot", 0);    	   	      
   CiuOverlayImg5_Bot = (u32 *)addr;
   addr += 0;

	DrawMemoryBlock(addr, 0, "CiuOverlayImg5_SP_Top", 0);    	   	      
   CiuOverlayImg5_SP_Top = (u32 *)addr;
   addr += 0;

    DrawMemoryBlock(addr, 0, "CiuOverlayImg5_SP_Bot", 0);    	   	      
   CiuOverlayImg5_SP_Bot = (u32 *)addr;
   addr += 0;
#endif


#ifndef DRAW_MEMORY_POOL
   DEBUG_MAIN("addr:0x%08x~0x%08x: Scalar overlay image %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif

    //============================================== Common Data(JPEG and MPEG data) ============================================================//
    addr                    = (u8 *)(((u32)addr + 255) & ~255); //Special cmd max burst length 64*4 byte
    addr_temp               = addr;
    DrawMemoryBlock(addr, 0, "PKBuf", 0);    	   	      
    PKBuf                   = (u8 *)addr;
    //===================Jpeg memory pools===================//
#if (JPEG_ENC_OPMODE_OPTION == JPEG_OPMODE_FRAME)
    addr                   += 0;//(IMG_MAX_WIDTH * (IMG_MAX_HEIGHT+32) );
#elif (JPEG_ENC_OPMODE_OPTION == JPEG_OPMODE_SLICE)
    addr                   += 0;//(640 * 480 * 4);
#endif

    DrawMemoryBlock(addr, 0, "siuRawBuf", 0);    	   	      
    siuRawBuf               = (u8 *)addr;
    addr                   += 0;//(SENSOR_VALID_SIZE_X * SENSOR_VALID_SIZE_Y);

	DrawMemoryBlock(PKBuf0, 0, "Jpeg_displaybuf[0]", 1);    	   	      
    Jpeg_displaybuf[0]      = PKBuf0;

	DrawMemoryBlock(PKBuf1, 0, "Jpeg_displaybuf[1]", 1);    	   	      
    Jpeg_displaybuf[1]      = PKBuf1;

	DrawMemoryBlock(PKBuf2, 0, "Jpeg_displaybuf[2]", 1);    	   	      
    Jpeg_displaybuf[2]      = PKBuf2;

			
    //for Jpeg decoder    

#if ADDAPP2TOJPEG
    //Appendix information
    DrawMemoryBlock(addr, (160*120+512), "exifApp2Data", 0);    	   	      
    exifApp2Data            = (DEF_APPENDIXINFO *)addr;
    addr                   += (160*120+512);
#endif

#if ADDAPP3TOJPEG
	DrawMemoryBlock(addr, 640 *480, "exifAPP3VGABitstream", 0);    	   	      
    exifAPP3VGABitstream    = addr;  //放pannel 大小縮圖
    addr                   += 640 *480;
#endif

#ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: JPEG Data %d bytes\n", (u32)addr_temp, (u32)addr, (u32)addr - (u32)addr_temp);
#endif
    //===================Mpeg memory pools===================//
    Mpeg_addr   = PKBuf;
    Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
    Mpeg_addr_temp          = Mpeg_addr;

    //Mpeg-input buffer in Encoding is  overlay with Mpeg-out buffer in decoding.
    #if(MULTI_CHANNEL_SEL & 0x01)
        //Decording using 與 SIU/ISU preview buffer 公用//
        DrawMemoryBlock(Mpeg_addr, PNBUF_SIZE_Y * 2, "mpeg4outputbuf[0]", 0);    	   	      
        mpeg4outputbuf[0]       = Mpeg_addr;		
        Mpeg_addr              += PNBUF_SIZE_Y * 2;

		DrawMemoryBlock(Mpeg_addr, PNBUF_SIZE_Y * 2, "mpeg4outputbuf[1]", 0);    	   	      
        mpeg4outputbuf[1]       = Mpeg_addr;
        Mpeg_addr              += PNBUF_SIZE_Y * 2;

		DrawMemoryBlock(Mpeg_addr, PNBUF_SIZE_Y * 2, "mpeg4outputbuf[2]", 0);    	   	      
        mpeg4outputbuf[2]       = Mpeg_addr;
        Mpeg_addr              += PNBUF_SIZE_Y * 2;

		
        PNBuf_Y[0]  = PNBuf_Y0 = PKBuf;
		DrawMemoryBlock(PNBuf_Y[0], 0, "PNBuf_Y[0]", 1);    	   	      

		PNBuf_C[0]  = PNBuf_C0 = PNBuf_Y0 + PNBUF_SIZE_Y;
		DrawMemoryBlock(PNBuf_C[0], 0, "PNBuf_C[0]", 1);    	   	      

		PNBuf_Y[1]  = PNBuf_Y1 = PNBuf_C0 + PNBUF_SIZE_C;
		DrawMemoryBlock(PNBuf_Y[1], 0, "PNBuf_Y[1]", 1);    	   	      
		
        PNBuf_C[1]  = PNBuf_C1 = PNBuf_Y1 + PNBUF_SIZE_Y;
		DrawMemoryBlock(PNBuf_C[1], 0, "PNBuf_C[1]", 1);    	   	      
		
        PNBuf_Y[2]  = PNBuf_Y2 = PNBuf_C1 + PNBUF_SIZE_C;
		DrawMemoryBlock(PNBuf_Y[2], 0, "PNBuf_Y[2]", 1);    	   	      
		
        PNBuf_C[2]  = PNBuf_C2 = PNBuf_Y2 + PNBUF_SIZE_Y;
		DrawMemoryBlock(PNBuf_C[2], 0, "PNBuf_C[2]", 1);    	   	      
		
        PNBuf_Y[3]  = PNBuf_Y3 = PNBuf_C2 + PNBUF_SIZE_C;
		DrawMemoryBlock(PNBuf_Y[3], 0, "PNBuf_Y[3]", 1);    	   	      
		
        PNBuf_C[3]  = PNBuf_C3 = PNBuf_Y3 + PNBUF_SIZE_Y;
		DrawMemoryBlock(PNBuf_C[3], 0, "PNBuf_C[3]", 1);    	   	      
    #else
        PNBuf_Y[0]  = PNBuf_Y0 = PKBuf;
		DrawMemoryBlock(PNBuf_Y[0], 0, "PNBuf_Y[0]", 1);    	   	      
		
        PNBuf_C[0]  = PNBuf_C0 = PNBuf_Y0 + 0;
		DrawMemoryBlock(PNBuf_C[0], 0, "PNBuf_C[0]", 1);    	   	      		
		
        PNBuf_Y[1]  = PNBuf_Y1 = PNBuf_C0 + 0;
		DrawMemoryBlock(PNBuf_Y[1], 0, "PNBuf_Y[1]", 1);    	   	      
		
        PNBuf_C[1]  = PNBuf_C1 = PNBuf_Y1 + 0;
		DrawMemoryBlock(PNBuf_C[1], 0, "PNBuf_C[1]", 1);    	   	      		
		
        PNBuf_Y[2]  = PNBuf_Y2 = PNBuf_C1 + 0;
		DrawMemoryBlock(PNBuf_Y[2], 0, "PNBuf_Y[2]", 1);    	   	      

        PNBuf_C[2]  = PNBuf_C2 = PNBuf_Y2 + 0;
		DrawMemoryBlock(PNBuf_C[2], 0, "PNBuf_C[2]", 1);    	   	      				
		
        PNBuf_Y[3]  = PNBuf_Y3 = PNBuf_C2 + 0;
		DrawMemoryBlock(PNBuf_Y[3], 0, "PNBuf_Y[3]", 1);    	   	      		
        PNBuf_C[3]  = PNBuf_C3 = PNBuf_Y3 + 0;
		DrawMemoryBlock(PNBuf_C[3], 0, "PNBuf_C[3]", 1);    	   	      				
    #endif

    #if ( (MULTI_CHANNEL_SEL & 0x01) == 0)
        Mpeg_addr   = PKBuf;
	    Mpeg_addr   = (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
	    Mpeg_addr_temp              = Mpeg_addr;
    #endif

 #ifndef DRAW_MEMORY_POOL   
    DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x: Ch0 mpeg4outputbuf %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
 #endif

    //-------------------------------------------//
    Mpeg_addr_temp              = Mpeg_addr;

    #if(MULTI_CHANNEL_SEL & 0x02)
      #if DUAL_MODE_DISP_SUPPORT
       #define PNBUF_SUB1_SIZE  ((PNBUF_SIZE_Y + PNBUF_SIZE_C) * 2)
      #else
         #if MULTI_STREAM_SUPPORT
            #define PNBUF_SUB1_SIZE  (PNBUF_SIZE_Y + PNBUF_SIZE_C + PNBUF_SP_SIZE_Y + PNBUF_SP_SIZE_C )
         #else
            #define PNBUF_SUB1_SIZE  (PNBUF_SIZE_Y + PNBUF_SIZE_C)
         #endif
      #endif
       #if IS_COMMAX_DOORPHONE
           #undef PNBUF_SUB1_SIZE
           #define PNBUF_SUB1_SIZE  (PNBUF_SIZE_Y*2)
       #endif

	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB1_SIZE, "PNBuf_sub1[0]", 0);    	   	      		
       PNBuf_sub1[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB1_SIZE;

	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB1_SIZE, "PNBuf_sub1[1]", 0);    	   	      		
       PNBuf_sub1[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB1_SIZE;

	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB1_SIZE, "PNBuf_sub1[2]", 0);    	   	      		
       PNBuf_sub1[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB1_SIZE;

	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB1_SIZE, "PNBuf_sub1[3]", 0);    	   	      		
       PNBuf_sub1[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB1_SIZE;

       #undef PNBUF_SUB1_SIZE
    #else
		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub1[0]", 0);    	   	      				
       PNBuf_sub1[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub1[1]", 0);    	   	      		
       PNBuf_sub1[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub1[2]", 0);    	   	      		
       PNBuf_sub1[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub1[3]", 0);    	   	      		
       PNBuf_sub1[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;
    #endif

    #if ( (MULTI_CHANNEL_SEL & 0x03) == 0)
       Mpeg_addr               = PKBuf;
	   Mpeg_addr= (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
	   Mpeg_addr_temp              = Mpeg_addr;
    #endif

       #ifndef DRAW_MEMORY_POOL
       DEBUG_MAIN("PNBuf_sub1:0x%08x~0x%08x: Ch1 PNBuf_sub1 %d bytes\n", (u32)PNBuf_sub1[0], (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)PNBuf_sub1[0]);
       #endif
       //----------------------------------//
       Mpeg_addr_temp          = Mpeg_addr;

    #if(MULTI_CHANNEL_SEL & 0x04)
      #if DUAL_MODE_DISP_SUPPORT
       PNBuf_sub2[0]           = PNBuf_sub1[0] + 640;
	   DrawMemoryBlock(PNBuf_sub2[0], 0, "PNBuf_sub2[0]", 1);    	   	      		
		  
       PNBuf_sub2[1]           = PNBuf_sub1[1] + 640;
	   DrawMemoryBlock(PNBuf_sub2[1], 0, "PNBuf_sub2[1]", 1);    	   	      			   
	   
       PNBuf_sub2[2]           = PNBuf_sub1[2] + 640;
   	   DrawMemoryBlock(PNBuf_sub2[2], 0, "PNBuf_sub2[2]", 1);    	   	      		
	   
       PNBuf_sub2[3]           = PNBuf_sub1[3] + 640;
   	   DrawMemoryBlock(PNBuf_sub2[3], 0, "PNBuf_sub2[3]", 1);    	   	      		
      #else
       #if MULTI_STREAM_SUPPORT
       #define PNBUF_SUB2_SIZE  (PNBUF_SIZE_Y + PNBUF_SIZE_C + PNBUF_SP_SIZE_Y + PNBUF_SP_SIZE_C )
       #else
       #define PNBUF_SUB2_SIZE  (PNBUF_SIZE_Y + PNBUF_SIZE_C)
       #endif
	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB2_SIZE, "PNBuf_sub2[0]", 0);    	   	      		
       PNBuf_sub2[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB2_SIZE;

	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB2_SIZE, "PNBuf_sub2[1]", 0);    	   	      		
       PNBuf_sub2[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB2_SIZE;

	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB2_SIZE, "PNBuf_sub2[2]", 0);    	   	      		
       PNBuf_sub2[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB2_SIZE;

	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB2_SIZE, "PNBuf_sub2[3]", 0);    	   	      		
       PNBuf_sub2[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB2_SIZE;
      #endif
    #else
		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub2[0]", 0);    	   	      		
       PNBuf_sub2[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub2[1]", 0);    	   	      		
       PNBuf_sub2[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub2[2]", 0);    	   	      		
       PNBuf_sub2[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub2[3]", 0);    	   	      		
       PNBuf_sub2[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;
    #endif


    #if ( (MULTI_CHANNEL_SEL & 0x07) == 0)
       Mpeg_addr               = PKBuf;
	   Mpeg_addr= (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
	   Mpeg_addr_temp              = Mpeg_addr;   
    #endif

    #ifndef DRAW_MEMORY_POOL
       DEBUG_MAIN("PNBuf_sub2:0x%08x~0x%08x: Ch1 PNBuf_sub2 %d bytes\n", (u32)PNBuf_sub1[0], (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)PNBuf_sub1[0]);
    #endif

    //----------------------------------//
       Mpeg_addr_temp          = Mpeg_addr;

    #if(MULTI_CHANNEL_SEL & 0x08)
		DrawMemoryBlock(Mpeg_addr, (PNBUF_SIZE_Y+PNBUF_SIZE_C), "PNBuf_sub3[0]", 0);    	   	      		
       PNBuf_sub3[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

		DrawMemoryBlock(Mpeg_addr, (PNBUF_SIZE_Y+PNBUF_SIZE_C), "PNBuf_sub3[1]", 0);    	   	      		
       PNBuf_sub3[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

		DrawMemoryBlock(Mpeg_addr, (PNBUF_SIZE_Y+PNBUF_SIZE_C), "PNBuf_sub3[2]", 0);    	   	      		
       PNBuf_sub3[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

		DrawMemoryBlock(Mpeg_addr, (PNBUF_SIZE_Y+PNBUF_SIZE_C), "PNBuf_sub3[3]", 0);    	   	      		
       PNBuf_sub3[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);
    #else
		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub3[0]", 0);    	   	      		
       PNBuf_sub3[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub3[1]", 0);    	   	      		
       PNBuf_sub3[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub3[2]", 0);    	   	      		
       PNBuf_sub3[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub3[3]", 0);    	   	      		
       PNBuf_sub3[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;
    #endif


    #if ( (MULTI_CHANNEL_SEL & 0x0f) == 0)
       Mpeg_addr               = PKBuf;
	   Mpeg_addr= (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
	   Mpeg_addr_temp              = Mpeg_addr;   
    #endif

       #ifndef DRAW_MEMORY_POOL
       DEBUG_MAIN("PNBuf_sub3:0x%08x~0x%08x: Ch1 PNBuf_sub3 %d bytes\n", (u32)PNBuf_sub1[0], (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)PNBuf_sub1[0]);
       #endif

    
       //----------------------------------//
       Mpeg_addr_temp          = Mpeg_addr;

    #if(MULTI_CHANNEL_SEL & 0x10)
		DrawMemoryBlock(Mpeg_addr, (PNBUF_SIZE_Y+PNBUF_SIZE_C), "PNBuf_sub4[0]", 0);    	   	      		
       PNBuf_sub4[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

		DrawMemoryBlock(Mpeg_addr, (PNBUF_SIZE_Y+PNBUF_SIZE_C), "PNBuf_sub4[1]", 0);    	   	      		
       PNBuf_sub4[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

		DrawMemoryBlock(Mpeg_addr, (PNBUF_SIZE_Y+PNBUF_SIZE_C), "PNBuf_sub4[2]", 0);    	   	      		
       PNBuf_sub4[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);

		DrawMemoryBlock(Mpeg_addr, (PNBUF_SIZE_Y+PNBUF_SIZE_C), "PNBuf_sub4[3]", 0);    	   	      		
       PNBuf_sub4[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);
    #else
		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub4[0]", 0);    	   	      		
       PNBuf_sub4[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub4[1]", 0);    	   	      		
       PNBuf_sub4[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub4[2]", 0);    	   	      		
       PNBuf_sub4[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub4[3]", 0);    	   	      		
       PNBuf_sub4[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;
    #endif

    #if ( (MULTI_CHANNEL_SEL & 0x1f) == 0)
       Mpeg_addr               = PKBuf;
	   Mpeg_addr= (u8 *)(((u32)Mpeg_addr + 255) & 0xffffff00); //64 word alignment
	   Mpeg_addr_temp              = Mpeg_addr; 
    #endif
    
    #ifndef DRAW_MEMORY_POOL
       DEBUG_MAIN("PNBuf_sub4:0x%08x~0x%08x: Ch1 PNBuf_sub4 %d bytes\n", (u32)PNBuf_sub1[0], (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)PNBuf_sub1[0]);
    #endif

    //---------------------------------------//
        Mpeg_addr_temp              = Mpeg_addr; 

    #if(MULTI_CHANNEL_SEL & 0x20)
      #if DUAL_MODE_DISP_SUPPORT
       PNBuf_sub5[0]           = PNBuf_sub1[0] + 640;
	   DrawMemoryBlock(PNBuf_sub5[0], 0, "PNBuf_sub2[0]", 1);    	   	      		
		  
       PNBuf_sub5[1]           = PNBuf_sub1[1] + 640;
	   DrawMemoryBlock(PNBuf_sub5[1], 0, "PNBuf_sub2[1]", 1);    	   	      			   
	   
       PNBuf_sub5[2]           = PNBuf_sub1[2] + 640;
   	   DrawMemoryBlock(PNBuf_sub5[2], 0, "PNBuf_sub2[2]", 1);    	   	      		
	   
       PNBuf_sub5[3]           = PNBuf_sub1[3] + 640;
   	   DrawMemoryBlock(PNBuf_sub5[3], 0, "PNBuf_sub2[3]", 1);    	   	      		
      #else
       #if MULTI_STREAM_SUPPORT
       #define PNBUF_SUB5_SIZE  (PNBUF_SIZE_Y + PNBUF_SIZE_C + PNBUF_SP_SIZE_Y + PNBUF_SP_SIZE_C )
       #else
       #define PNBUF_SUB5_SIZE  (PNBUF_SIZE_Y + PNBUF_SIZE_C)
       #endif
	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB5_SIZE, "PNBuf_sub2[0]", 0);    	   	      		
       PNBuf_sub5[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB5_SIZE;

	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB5_SIZE, "PNBuf_sub2[1]", 0);    	   	      		
       PNBuf_sub5[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB5_SIZE;

	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB5_SIZE, "PNBuf_sub2[2]", 0);    	   	      		
       PNBuf_sub5[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB5_SIZE;

	   DrawMemoryBlock(Mpeg_addr, PNBUF_SUB5_SIZE, "PNBuf_sub2[3]", 0);    	   	      		
       PNBuf_sub5[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += PNBUF_SUB5_SIZE;
      #endif
    #else
		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub5[0]", 0);    	   	      		
       PNBuf_sub5[0]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub5[1]", 0);    	   	      		
       PNBuf_sub5[1]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub5[2]", 0);    	   	      		
       PNBuf_sub5[2]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;

		DrawMemoryBlock(Mpeg_addr, 0, "PNBuf_sub5[3]", 0);    	   	      		
       PNBuf_sub5[3]           = (u8 *)Mpeg_addr;
       Mpeg_addr              += 0;
    #endif

    #ifndef DRAW_MEMORY_POOL
       DEBUG_MAIN("PNBuf_sub5:0x%08x~0x%08x: Ch1 PNBuf_sub5 %d bytes\n", (u32)PNBuf_sub1[0], (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)PNBuf_sub1[0]);
    #endif


    #if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
		DrawMemoryBlock(Mpeg_addr, (PNBUF_SIZE_Y+PNBUF_SIZE_C), "PNBuf_Quad", 0);    	   	      		
       PNBuf_Quad              = (u8 *)Mpeg_addr;
       Mpeg_addr              += (PNBUF_SIZE_Y+PNBUF_SIZE_C);
    #endif

    //--------------------------------------------------------------//
    Mpeg_addr_temp              = Mpeg_addr; 

#if MULTI_CHANNEL_VIDEO_REC
    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
		if(MULTI_CHANNEL_SEL & (1 << i) || (i >= MULTI_CHANNEL_LOCAL_MAX))
        {
            u32 mpeg4_max_width, mpeg4_max_height;
			
			
            mpeg4_max_width     = MPEG4_MAX_WIDTH;
            mpeg4_max_height    = MPEG4_MAX_HEIGHT;
            pVideoClipOption    = &VideoClipOption[i];
            
        #if ( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1)||\
              (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1)||\
              (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)||\
              (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)||\
              (SW_APPLICATION_OPTION == MR8110_BABYMONITOR)||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) || (SW_APPLICATION_OPTION == Standalone_Test)||\
              (SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
            #if(VIDEO_CODEC_OPTION == H264_CODEC)			
            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr+0x07) & 0xfffffff8); //double word aligned

			DrawMemoryBlock(Mpeg_addr, 0, "H264MBPredBuf", 0);    	   	      		
            pVideoClipOption->H264MBPredBuf     = (u8 *)Mpeg_addr;                              //double word aligned
            Mpeg_addr                          += 0; //include upper 4 YUV line, FRAME_WIDTH*2*4

			DrawMemoryBlock(Mpeg_addr, 0, "H264ILFPredBuf", 0);    	   	      		
			pVideoClipOption->H264ILFPredBuf    = (u8 *)Mpeg_addr;                             //double word aligned
            Mpeg_addr                          += 0; //include upper 4 YUV line, FRAME_WIDTH*2*4

			DrawMemoryBlock(Mpeg_addr, 0, "H264IntraPredBuf", 0);    	   	      					
            pVideoClipOption->H264IntraPredBuf  = (u8 *)Mpeg_addr;                            //double word aligned
            Mpeg_addr                          += 0;  //include upper YUV line, FRAME_WIDTH*2
            #endif

			DrawMemoryBlock(Mpeg_addr, 0, "mpeg4MVBuf", 0);    	   	      					
            pVideoClipOption->mpeg4MVBuf        = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;
			
            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
			DrawMemoryBlock(Mpeg_addr, 0, "mpeg4PRefBuf_Y", 0);    	   	      					            
            pVideoClipOption->mpeg4PRefBuf_Y    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

			DrawMemoryBlock(Mpeg_addr, 0, "mpeg4PRefBuf_Cb", 0);    	   	      					            
            pVideoClipOption->mpeg4PRefBuf_Cb   = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

			DrawMemoryBlock(Mpeg_addr, 0, "mpeg4PRefBuf_Cr", 0);    	   	      					            
            pVideoClipOption->mpeg4PRefBuf_Cr   = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;
			
            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
			DrawMemoryBlock(Mpeg_addr, 0, "mpeg4NRefBuf_Y", 0);    	   	      					                        
            pVideoClipOption->mpeg4NRefBuf_Y    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

			DrawMemoryBlock(Mpeg_addr, 0, "mpeg4NRefBuf_Cb", 0);    	   	      					                        
            pVideoClipOption->mpeg4NRefBuf_Cb   = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

			DrawMemoryBlock(Mpeg_addr, 0, "mpeg4NRefBuf_Cb", 0);    	   	      					                        
            pVideoClipOption->mpeg4NRefBuf_Cr    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += 0;

			DrawMemoryBlock(Mpeg_addr, IIS_BUF_NUM*0, "iisSounBuf", 0);    	   	      					                        
            for(j = 0; j < IIS_BUF_NUM; j++)
            {
                pVideoClipOption->iisSounBuf[j] = Mpeg_addr;
                Mpeg_addr                      += 0;
            }
        #else
            #if(VIDEO_CODEC_OPTION == H264_CODEC)			
            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr+0x07) & 0xfffffff8); //double word aligned
   			DrawMemoryBlock(Mpeg_addr, 0, "H264MBPredBuf", 1);    	   	      					                        
            pVideoClipOption->H264MBPredBuf     = (u8 *)Mpeg_addr;                              //double word aligned
            
            Mpeg_addr                          += (((mpeg4_max_width*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4
   			DrawMemoryBlock(Mpeg_addr, 0, "H264ILFPredBuf", 1);    	   	      					                                    
            pVideoClipOption->H264ILFPredBuf    = (u8 *)Mpeg_addr;                             //double word aligned

			Mpeg_addr                          += (((mpeg4_max_width*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4
   			DrawMemoryBlock(Mpeg_addr, 0, "H264IntraPredBuf", 1);    	   	      					                                    			
            pVideoClipOption->H264IntraPredBuf  = (u8 *)Mpeg_addr;                            //double word aligned
            Mpeg_addr                          += (((mpeg4_max_width*2)+0x07) & 0xfffffff8);  //include upper YUV line, FRAME_WIDTH*2
            #endif

			DrawMemoryBlock(Mpeg_addr, (MPEG4_MVBUF & ~63), "mpeg4MVBuf", 0);    	   	      					                                    			
            pVideoClipOption->mpeg4MVBuf        = (u8 *)Mpeg_addr; //Lsk: remove 
            Mpeg_addr                          += ((MPEG4_MVBUF + 63) & ~63);			
            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

			DrawMemoryBlock(Mpeg_addr, ((mpeg4_max_width + 0) * (mpeg4_max_height + 0)), "mpeg4PRefBuf_Y", 0);    	   	      					                                    			
            pVideoClipOption->mpeg4PRefBuf_Y    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += ((mpeg4_max_width + 0) * (mpeg4_max_height + 0));

			DrawMemoryBlock(Mpeg_addr, (((mpeg4_max_width >> 1) + 0) * ((mpeg4_max_height >> 1) + 0)), "mpeg4PRefBuf_Cb", 0);    	   	      					                                    			
            pVideoClipOption->mpeg4PRefBuf_Cb   = (u8 *)Mpeg_addr;
            Mpeg_addr                          += (((mpeg4_max_width >> 1) + 0) * ((mpeg4_max_height >> 1) + 0));

			DrawMemoryBlock(Mpeg_addr, (((mpeg4_max_width >> 1) + 0) * ((mpeg4_max_height >> 1) + 0)), "mpeg4PRefBuf_Cr", 0);    	   	      					                                    			
            pVideoClipOption->mpeg4PRefBuf_Cr         = (u8 *)Mpeg_addr;
            Mpeg_addr                          += (((mpeg4_max_width >> 1) + 0) * ((mpeg4_max_height >> 1) + 0));
            Mpeg_addr                           = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary

			DrawMemoryBlock(Mpeg_addr, ((mpeg4_max_width + 0) * (mpeg4_max_height + 0)), "mpeg4NRefBuf_Y", 0);    	   	      					                                    			
            pVideoClipOption->mpeg4NRefBuf_Y    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += ((mpeg4_max_width + 0) * (mpeg4_max_height + 0));

			DrawMemoryBlock(Mpeg_addr, (((mpeg4_max_width >> 1) + 0) * ((mpeg4_max_height >> 1) + 0)), "mpeg4NRefBuf_Cb", 0);    	   	      					                                    			
            pVideoClipOption->mpeg4NRefBuf_Cb   = (u8 *)Mpeg_addr;
            Mpeg_addr                          += (((mpeg4_max_width >> 1) + 0) * ((mpeg4_max_height >> 1) + 0));

			DrawMemoryBlock(Mpeg_addr, (((mpeg4_max_width >> 1) + 0) * ((mpeg4_max_height >> 1) + 0)), "mpeg4NRefBuf_Cr", 0);    	   	      					                                    			
            pVideoClipOption->mpeg4NRefBuf_Cr    = (u8 *)Mpeg_addr;
            Mpeg_addr                          += (((mpeg4_max_width >> 1) + 0) * ((mpeg4_max_height >> 1) + 0));

			DrawMemoryBlock(Mpeg_addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "iisSounBuf", 0);    	   	      					                        
            for(j = 0; j < IIS_BUF_NUM; j++)
            {
                pVideoClipOption->iisSounBuf[j] = Mpeg_addr;
                Mpeg_addr                      += IIS_CHUNK_SIZE;
            }
        #endif

        #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
			DrawMemoryBlock(Mpeg_addr, IMA_ADPCM_BLOCK_SIZE, "ImaAdpcmBuf", 0);    	   	      					                        
            pVideoClipOption->ImaAdpcmBuf       = Mpeg_addr;
            Mpeg_addr                          += IMA_ADPCM_BLOCK_SIZE;

			DrawMemoryBlock(Mpeg_addr, 0, "ImaAdpcmBufEnd", 1);    	   	      					                        
            pVideoClipOption->ImaAdpcmBufEnd    = Mpeg_addr;
        #endif

        #if ( (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) ||(SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) ||\
              (SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1) || (SW_APPLICATION_OPTION == Standalone_Test) || (SW_APPLICATION_OPTION == MR9100_RF_AHDIN_AVSED_RX1) ||\
              (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) ||  (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) ||\
              (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) ||\
              (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == MR8110_BABYMONITOR) || (SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) ||\
              (SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
            // Mpeg Video encoding/decoding bitstream buffer
            DrawMemoryBlock(Mpeg_addr, 0, "VideoBuf", 0);    	   	      					                        
            pVideoClipOption->VideoBuf          = Mpeg_addr;
            Mpeg_addr                          += 0;
	

			DrawMemoryBlock(Mpeg_addr, 0, "mpeg4VideBufEnd", 1);    	   	      					                        
            pVideoClipOption->mpeg4VideBufEnd   = Mpeg_addr;
        #else
            // Mpeg Video encoding/decoding bitstream buffer
            DrawMemoryBlock(Mpeg_addr, MPEG4_MAX_BUF_SIZE, "VideoBuf", 0);    	   	      					                        
            pVideoClipOption->VideoBuf          = Mpeg_addr;
            Mpeg_addr                          += MPEG4_MAX_BUF_SIZE;
	

			DrawMemoryBlock(Mpeg_addr, 0, "mpeg4VideBufEnd", 1);    	   	      					                        
            pVideoClipOption->mpeg4VideBufEnd   = Mpeg_addr;
        #endif

        #if CDVR_LOG
			Mpeg_addr                           = (u8*)(((u32)Mpeg_addr + 15) & ~15);
			DrawMemoryBlock(Mpeg_addr, LOG_FILE_MAX_SIZE, "LogFileBuf", 0);    	   	      					                        
            pVideoClipOption->LogFileBuf        = Mpeg_addr;
            Mpeg_addr                          += LOG_FILE_MAX_SIZE;

			DrawMemoryBlock(Mpeg_addr, 0, "LogFileBufEnd", 1);    	   	      					                        
            pVideoClipOption->LogFileBufEnd     = Mpeg_addr;
        #endif

        #if (CDVR_TEST_LOG)
            Mpeg_addr                           = (u8*)(((u32)Mpeg_addr + 15) & ~15);
			DrawMemoryBlock(Mpeg_addr, LOG_FILE_MAX_SIZE, "LogFileBuf", 0);    	   	      					                        
            pVideoClipOption->LogFileBuf        = Mpeg_addr;
            Mpeg_addr                          += LOG_FILE_MAX_SIZE;

			DrawMemoryBlock(Mpeg_addr, 0, "LogFileBufEnd", 1);    	   	      					                        
            pVideoClipOption->LogFileBufEnd     = Mpeg_addr;
        #endif

			DrawMemoryBlock(Mpeg_addr, MPEG4_INDEX_BUF_SIZE, "mpeg4IndexBuf", 0);    	   	      					                        
            pVideoClipOption->mpeg4IndexBuf     = Mpeg_addr;
            Mpeg_addr                          += MPEG4_INDEX_BUF_SIZE;

          #ifndef DRAW_MEMORY_POOL  
            DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x: Ch%d MPEG4 %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, i, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
          #endif
            Mpeg_addr_temp              = Mpeg_addr;
        }
    }

  //===============for playback===============//
    Mpeg_addr_temp              = Mpeg_addr;
     #if (RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)    //與 RF RX Decoding buffer 共用
       //與 RF RX Decoding buffer 共用
       #if(VIDEO_CODEC_OPTION == H264_CODEC)	   
        Mpeg_addr              = (u8 *)(((u32)Mpeg_addr+0x07) & 0xfffffff8); //double word aligned

		DrawMemoryBlock(Mpeg_addr, (((RF_RX_DEC_WIDTH_MAX*2*4)+0x07) & 0xfffffff8), "H264MBPredBuf", 0);    	   	      					                        
        H264MBPredBuf          = (u8 *)Mpeg_addr;                              //double word aligned
        Mpeg_addr              += (((RF_RX_DEC_WIDTH_MAX*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

		DrawMemoryBlock(Mpeg_addr, (((RF_RX_DEC_WIDTH_MAX*2*4)+0x07) & 0xfffffff8), "H264ILFPredBuf", 0);    	   	      					                        
        H264ILFPredBuf          = (u8 *)Mpeg_addr;                             //double word aligned
        Mpeg_addr              += (((RF_RX_DEC_WIDTH_MAX*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

		DrawMemoryBlock(Mpeg_addr, (((RF_RX_DEC_WIDTH_MAX*2)+0x07) & 0xfffffff8), "H264IntraPredBuf", 0);    	   	      					                        
        H264IntraPredBuf        = (u8 *)Mpeg_addr;                            //double word aligned
        Mpeg_addr              += (((RF_RX_DEC_WIDTH_MAX*2)+0x07) & 0xfffffff8);  //include upper YUV line, FRAME_WIDTH*2
       #endif


    	mpeg4MVBuf              = rfiuRxDecBuf[0].mpeg4MVBuf;
	    DrawMemoryBlock(mpeg4MVBuf, 0, "mpeg4MVBuf", 1);    	   	      					                        

        mpeg4PRefBuf_Y          = rfiuRxDecBuf[0].mpeg4PRefBuf_Y;
	    DrawMemoryBlock(mpeg4PRefBuf_Y, 0, "mpeg4PRefBuf_Y", 1);    	   	      					                        
		
        mpeg4PRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4PRefBuf_Cb;
	    DrawMemoryBlock(mpeg4PRefBuf_Cb, 0, "mpeg4PRefBuf_Cb", 1);    	   	      					                        

        mpeg4PRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4PRefBuf_Cr;
	    DrawMemoryBlock(mpeg4PRefBuf_Cr, 0, "mpeg4PRefBuf_Cr", 1);    	   	      					                        
		
        mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
	    DrawMemoryBlock(mpeg4NRefBuf_Y, 0, "mpeg4NRefBuf_Y", 1);    	   	      					                        

        mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
	    DrawMemoryBlock(mpeg4NRefBuf_Cb, 0, "mpeg4NRefBuf_Cb", 1);    	   	      					                        
		
        mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
	    DrawMemoryBlock(mpeg4NRefBuf_Cr, 0, "mpeg4NRefBuf_Cr", 1);    	   	      					                        

		DrawMemoryBlock(Mpeg_addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "iisSounBuf", 0);    	   	      					                        
        for(i = 0; i < IIS_BUF_NUM; i++)
        {
            iisSounBuf[i]       = Mpeg_addr;
            Mpeg_addr          += IIS_CHUNK_SIZE;
        }

       #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
	    DrawMemoryBlock(Mpeg_addr, IMA_ADPCM_BLOCK_SIZE, "ImaAdpcmBuf", 0);    	   	      					                        
        ImaAdpcmBuf             = Mpeg_addr;
        Mpeg_addr              += IMA_ADPCM_BLOCK_SIZE;

	    DrawMemoryBlock(Mpeg_addr, 0, "ImaAdpcmBufEnd", 1);    	   	      					                        		
        ImaAdpcmBufEnd          = Mpeg_addr;
       #endif

        // Mpeg Video encoding/decoding bitstream buffer
       #if 1    //與 RF RX Decoding buffer 共用
         //與 RF RX Decoding buffer 共用
       #else
         VideoBuf                = Mpeg_addr;
         Mpeg_addr              += MPEG4_MAX_BUF_SIZE;
         mpeg4VideBufEnd         = Mpeg_addr;
       #endif

       #if CDVR_LOG
        Mpeg_addr               = (u8*)(((u32)Mpeg_addr + 15) & ~15);
	   DrawMemoryBlock(Mpeg_addr, LOG_FILE_MAX_SIZE, "LogFileBuf", 0);    	   	      					                        		
        LogFileBuf              = Mpeg_addr;
        Mpeg_addr              += LOG_FILE_MAX_SIZE;

		DrawMemoryBlock(Mpeg_addr, 0, "LogFileBufEnd", 1);    	   	      					                        		
        LogFileBufEnd           = Mpeg_addr;
       #endif

       #if (CDVR_TEST_LOG)
        Mpeg_addr               = (u8*)(((u32)Mpeg_addr + 15) & ~15);
	   DrawMemoryBlock(Mpeg_addr, LOG_FILE_MAX_SIZE, "LogFileBuf", 0);    	   	      					                        		
        LogFileBuf              = Mpeg_addr;
        Mpeg_addr              += LOG_FILE_MAX_SIZE;

		DrawMemoryBlock(Mpeg_addr, 0, "LogFileBufEnd", 1);    	   	      					                        		
        LogFileBufEnd           = Mpeg_addr;
       #endif

		DrawMemoryBlock(Mpeg_addr, MPEG4_INDEX_BUF_SIZE, "mpeg4IndexBuf", 0);    	   	      					                        		
        mpeg4IndexBuf           = Mpeg_addr;
        Mpeg_addr              += MPEG4_INDEX_BUF_SIZE;

     #else  // for no RF project, doorphone  etc.
       #if(VIDEO_CODEC_OPTION == H264_CODEC)	   
        Mpeg_addr              = (u8 *)(((u32)Mpeg_addr+0x07) & 0xfffffff8); //double word aligned

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8), "H264MBPredBuf", 0);    	   	      					                        		
        H264MBPredBuf          = (u8 *)Mpeg_addr;                              //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8), "H264ILFPredBuf", 0);    	   	      					                        				
        H264ILFPredBuf          = (u8 *)Mpeg_addr;                             //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH*2)+0x07) & 0xfffffff8), "H264IntraPredBuf", 0);    	   	      					                        				
        H264IntraPredBuf        = (u8 *)Mpeg_addr;                            //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2)+0x07) & 0xfffffff8);  //include upper YUV line, FRAME_WIDTH*2
       #endif


    	mpeg4MVBuf              = pVideoClipOption->mpeg4MVBuf;
		DrawMemoryBlock(mpeg4MVBuf, 0, "mpeg4MVBuf", 1);    	   	      					                        				
		
        mpeg4PRefBuf_Y          = pVideoClipOption->mpeg4PRefBuf_Y;
		DrawMemoryBlock(mpeg4PRefBuf_Y, 0, "mpeg4PRefBuf_Y", 1);    	   	      					                        						

        mpeg4PRefBuf_Cb         = pVideoClipOption->mpeg4PRefBuf_Cb;
		DrawMemoryBlock(mpeg4PRefBuf_Cb, 0, "mpeg4PRefBuf_Cb", 1);    	   	      					                        						

        mpeg4PRefBuf_Cr         = pVideoClipOption->mpeg4PRefBuf_Cr;
		DrawMemoryBlock(mpeg4PRefBuf_Cr, 0, "mpeg4PRefBuf_Cr", 1);    	   	      					                        								

        mpeg4NRefBuf_Y          = pVideoClipOption->mpeg4NRefBuf_Y;
		DrawMemoryBlock(mpeg4NRefBuf_Y, 0, "mpeg4NRefBuf_Y", 1);    	   	      					                        								
		
        mpeg4NRefBuf_Cb         = pVideoClipOption->mpeg4NRefBuf_Cb;
		DrawMemoryBlock(mpeg4NRefBuf_Cb, 0, "mpeg4NRefBuf_Cb", 1);    	   	      					                        								
		
        mpeg4NRefBuf_Cr         = pVideoClipOption->mpeg4NRefBuf_Cr;
		DrawMemoryBlock(mpeg4NRefBuf_Cr, 0, "mpeg4NRefBuf_Cr", 1);    	   	      					                        								

		
       #if REMOTE_TALK_BACK    // Peter: APP talk back 會有邊錄邊放的可能,所以要有獨立的記憶體.
	   	DrawMemoryBlock(Mpeg_addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "iisSounBuf", 0);    	   	      					                        
        for(i = 0; i < IIS_BUF_NUM; i++)
        {
            iisSounBuf[i]       = Mpeg_addr;
            Mpeg_addr          += IIS_CHUNK_SIZE;
        }
       #else
	   	DrawMemoryBlock(Mpeg_addr, 0, "iisSounBuf", 1);    	   	      					                        
        for(i = 0; i < IIS_BUF_NUM; i++)
        {
            iisSounBuf[i]       = pVideoClipOption->iisSounBuf[i];
        }
       #endif

       #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        ImaAdpcmBuf             = pVideoClipOption->ImaAdpcmBuf;
	   DrawMemoryBlock(ImaAdpcmBuf, 0, "ImaAdpcmBuf", 1);    	   	      					                        
	   
        ImaAdpcmBufEnd          = pVideoClipOption->ImaAdpcmBufEnd;
		DrawMemoryBlock(ImaAdpcmBufEnd, 0, "ImaAdpcmBufEnd", 1);    	   	      					                        
       #endif

        // Mpeg Video encoding/decoding bitstream buffer
        VideoBuf                = pVideoClipOption->VideoBuf;
	
	   	DrawMemoryBlock(VideoBuf, 0, "VideoBuf", 1);    	   	      					                        
			
        mpeg4VideBufEnd         = pVideoClipOption->mpeg4VideBufEnd;
		DrawMemoryBlock(mpeg4VideBufEnd, 0, "mpeg4VideBufEnd", 1);    	   	      					                        		
       #if CDVR_LOG
        LogFileBuf              = pVideoClipOption->LogFileBuf;
		DrawMemoryBlock(LogFileBuf, 0, "LogFileBuf", 1);    	   	      					                        	   
		
        LogFileBufEnd           = pVideoClipOption->LogFileBufEnd;
		DrawMemoryBlock(LogFileBufEnd, 0, "LogFileBufEnd", 1);    	   	      					                        	   
       #endif

       #if (CDVR_TEST_LOG)
        LogFileBuf              = pVideoClipOption->LogFileBuf;
		DrawMemoryBlock(LogFileBuf, 0, "LogFileBuf", 1);    	   	      					                        	   
		
        LogFileBufEnd           = pVideoClipOption->LogFileBufEnd;
		DrawMemoryBlock(LogFileBufEnd, 0, "LogFileBufEnd", 1);    	   	      					                        	   		
       #endif

        mpeg4IndexBuf           = pVideoClipOption->mpeg4IndexBuf;
		DrawMemoryBlock(mpeg4IndexBuf, 0, "mpeg4IndexBuf", 1);    	   	      					                        	   			   
    #endif

  #ifndef DRAW_MEMORY_POOL  
    DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x:Playback(Audio) %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
  #endif


#else//--------------------------------(MULTI_CHANNEL_VIDEO_REC == 0) ----------------------------------
        Mpeg_addr_temp              = Mpeg_addr;

    #if ( (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) ||\
          (SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1) || (SW_APPLICATION_OPTION == Standalone_Test) || (SW_APPLICATION_OPTION == MR9100_RF_AHDIN_AVSED_RX1) ||\
          (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) ||\
          (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) ||\
          (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == MR8110_BABYMONITOR) ||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) ||\
          (SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
       #if(VIDEO_CODEC_OPTION == H264_CODEC)	   
        Mpeg_addr              = (u8 *)(((u32)Mpeg_addr+0x07) & 0xfffffff8); //double word aligned
		DrawMemoryBlock(Mpeg_addr, (((RF_RX_DEC_WIDTH_MAX*2*4)+0x07) & 0xfffffff8), "H264MBPredBuf", 0);    	   	      					                        	   			   
        H264MBPredBuf          = (u8 *)Mpeg_addr;                              //double word aligned
        Mpeg_addr              += (((RF_RX_DEC_WIDTH_MAX*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

		DrawMemoryBlock(Mpeg_addr, (((RF_RX_DEC_WIDTH_MAX*2*4)+0x07) & 0xfffffff8), "H264ILFPredBuf", 0);    	   	      					                        	   			   
        H264ILFPredBuf          = (u8 *)Mpeg_addr;                             //double word aligned
        Mpeg_addr              += (((RF_RX_DEC_WIDTH_MAX*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

		DrawMemoryBlock(Mpeg_addr, (((RF_RX_DEC_WIDTH_MAX*2)+0x07) & 0xfffffff8), "H264IntraPredBuf", 0);    	   	      					                        	   			   
        H264IntraPredBuf        = (u8 *)Mpeg_addr;                            //double word aligned
        Mpeg_addr              += (((RF_RX_DEC_WIDTH_MAX*2)+0x07) & 0xfffffff8);  //include upper YUV line, FRAME_WIDTH*2
       #endif

        mpeg4MVBuf              = rfiuRxDecBuf[0].mpeg4MVBuf;
		DrawMemoryBlock(mpeg4MVBuf, 0, "mpeg4MVBuf", 1);    	   	      					                        	   			   
		
        mpeg4PRefBuf_Y          = rfiuRxDecBuf[0].mpeg4PRefBuf_Y;
		DrawMemoryBlock(mpeg4PRefBuf_Y, 0, "mpeg4PRefBuf_Y", 1);    	   	      					                        	   			   
		
        mpeg4PRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4PRefBuf_Cb;
		DrawMemoryBlock(mpeg4PRefBuf_Cb, 0, "mpeg4PRefBuf_Cb", 1);    	   	      					                        	   			   
		
        mpeg4PRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4PRefBuf_Cr;
		DrawMemoryBlock(mpeg4PRefBuf_Cr, 0, "mpeg4PRefBuf_Cr", 1);    	   	      					                        	   			   
		
        mpeg4NRefBuf_Y          = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
		DrawMemoryBlock(mpeg4NRefBuf_Y, 0, "mpeg4NRefBuf_Y", 1);    	   	      					                        	   			   
		
        mpeg4NRefBuf_Cb         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
		DrawMemoryBlock(mpeg4NRefBuf_Cb, 0, "mpeg4NRefBuf_Cb", 1);    	   	      					                        	   			   
		
        mpeg4NRefBuf_Cr         = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
		DrawMemoryBlock(mpeg4NRefBuf_Cr, 0, "mpeg4NRefBuf_Cr", 1);    	   	      					                        	   			   


    #elif ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
            (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) ||\
            (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
       #if(VIDEO_CODEC_OPTION == H264_CODEC)	   
        Mpeg_addr              = (u8 *)(((u32)Mpeg_addr+0x07) & 0xfffffff8); //double word aligned
		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8), "H264MBPredBuf", 0);    	   	      					                        	   			   	
        H264MBPredBuf          = (u8 *)Mpeg_addr;                              //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8), "H264ILFPredBuf", 0);    	   	      					                        	   			   	
        H264ILFPredBuf          = (u8 *)Mpeg_addr;                             //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH*2)+0x07) & 0xfffffff8), "H264IntraPredBuf", 0);    	   	      					                        	   			   	
        H264IntraPredBuf        = (u8 *)Mpeg_addr;                            //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2)+0x07) & 0xfffffff8);  //include upper YUV line, FRAME_WIDTH*2
       #endif


        //For TX
        DrawMemoryBlock(Mpeg_addr, ((MPEG4_MAX_WIDTH + 63) & ~63), "mpeg4MVBuf", 0);    	   	      					                        	   			   	
        mpeg4MVBuf              = (u8 *)Mpeg_addr; //Lsk: remove 
        Mpeg_addr              += ((MPEG4_MAX_WIDTH + 63) & ~63);

        Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        DrawMemoryBlock(Mpeg_addr, (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT), "mpeg4PRefBuf_Y", 0);    	   	      					                        	   			   			
        mpeg4PRefBuf_Y          = (u8 *)Mpeg_addr;
        Mpeg_addr              += (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT);

        DrawMemoryBlock(Mpeg_addr, ((MPEG4_MAX_WIDTH >> 1) * (MPEG4_MAX_HEIGHT >> 1)), "mpeg4PRefBuf_Cb", 0);    	   	      					                        	   			   			
        mpeg4PRefBuf_Cb         = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((MPEG4_MAX_WIDTH >> 1) * (MPEG4_MAX_HEIGHT >> 1));

        DrawMemoryBlock(Mpeg_addr, ((MPEG4_MAX_WIDTH >> 1) * (MPEG4_MAX_HEIGHT >> 1)), "mpeg4PRefBuf_Cr", 0);    	   	      					                        	   			   				
        mpeg4PRefBuf_Cr         = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((MPEG4_MAX_WIDTH >> 1) * (MPEG4_MAX_HEIGHT >> 1));

    #if MULTI_STREAM_SUPPORT
		DrawMemoryBlock(Mpeg_addr, (PNBUF_SP_SIZE_Y + PNBUF_SP_SIZE_C*2), "MULTI_STREAM_SUPPORT", 0);    	   	      					                        	   			   				
        Mpeg_addr              += (PNBUF_SP_SIZE_Y + PNBUF_SP_SIZE_C*2) ;
    #endif

        Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
        DrawMemoryBlock(Mpeg_addr, (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT), mpeg4NRefBuf_Y, 0);    	   	      					                        	   			   				
        mpeg4NRefBuf_Y          = (u8 *)Mpeg_addr;
        Mpeg_addr              += (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT);

        DrawMemoryBlock(Mpeg_addr, ((MPEG4_MAX_WIDTH >> 1) * (MPEG4_MAX_HEIGHT >> 1)), mpeg4NRefBuf_Cb, 0);    	   	      					                        	   			   				
        mpeg4NRefBuf_Cb         = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((MPEG4_MAX_WIDTH >> 1) * (MPEG4_MAX_HEIGHT >> 1));

        DrawMemoryBlock(Mpeg_addr, ((MPEG4_MAX_WIDTH >> 1) * (MPEG4_MAX_HEIGHT >> 1)), mpeg4NRefBuf_Cr, 0);    	   	      					                        	   			   				
        mpeg4NRefBuf_Cr         = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((MPEG4_MAX_WIDTH >> 1) * (MPEG4_MAX_HEIGHT >> 1));

    #if MULTI_STREAM_SUPPORT
		DrawMemoryBlock(Mpeg_addr, (PNBUF_SP_SIZE_Y + PNBUF_SP_SIZE_C*2), "MULTI_STREAM_SUPPORT", 0);    	   	      					                        	   			   				
        Mpeg_addr              += (PNBUF_SP_SIZE_Y + PNBUF_SP_SIZE_C*2) ;
    #endif

    #else  
       #if(VIDEO_CODEC_OPTION == H264_CODEC)
        Mpeg_addr              = (u8 *)(((u32)Mpeg_addr+0x07) & 0xfffffff8); //double word aligned

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8), "H264MBPredBuf", 0);    	   	      					                        	   			   				
        H264MBPredBuf          = (u8 *)Mpeg_addr;                              //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8), "H264ILFPredBuf", 0);    	   	      					                        	   			   					
        H264ILFPredBuf          = (u8 *)Mpeg_addr;                             //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2*4)+0x07) & 0xfffffff8); //include upper 4 YUV line, FRAME_WIDTH*2*4

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH*2)+0x07) & 0xfffffff8), "H264IntraPredBuf", 0);    	   	      					                        	   			   					
        H264IntraPredBuf        = (u8 *)Mpeg_addr;                            //double word aligned
        Mpeg_addr              += (((MPEG4_MAX_WIDTH*2)+0x07) & 0xfffffff8);  //include upper YUV line, FRAME_WIDTH*2
       #endif


		DrawMemoryBlock(Mpeg_addr, ((MPEG4_MVBUF + 63) & ~63), "mpeg4MVBuf", 0);    	   	      					                        	   			   					
    	mpeg4MVBuf              = (u8 *)Mpeg_addr; //Lsk: remove 
        Mpeg_addr              += ((MPEG4_MVBUF + 63) & ~63);

        Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
		DrawMemoryBlock(Mpeg_addr, ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32)), "mpeg4PRefBuf_Y", 0);    	   	      					                        	   			   						
        mpeg4PRefBuf_Y          = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32));

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16)), "mpeg4PRefBuf_Cb", 0);    	   	      					                        	   			   						
        mpeg4PRefBuf_Cb         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16)), "mpeg4PRefBuf_Cr", 0);    	   	      					                        	   			   						
        mpeg4PRefBuf_Cr         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

        Mpeg_addr               = (u8 *)(((u32)Mpeg_addr + 255) & ~255); //Special cmd max burst length 64*4 byte, burst not cross 1kb address boundary
		DrawMemoryBlock(Mpeg_addr, ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32)), "mpeg4NRefBuf_Y", 0);    	   	      					                        	   			   						        
        mpeg4NRefBuf_Y          = (u8 *)Mpeg_addr;
        Mpeg_addr              += ((MPEG4_MAX_WIDTH + 32) * (MPEG4_MAX_HEIGHT + 32));

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16)), "mpeg4NRefBuf_Cb", 0);    	   	      					                        	   			   						        
        mpeg4NRefBuf_Cb         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));

		DrawMemoryBlock(Mpeg_addr, (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16)), "mpeg4NRefBuf_Cr", 0);    	   	      					                        	   			   						        
        mpeg4NRefBuf_Cr         = (u8 *)Mpeg_addr;
        Mpeg_addr              += (((MPEG4_MAX_WIDTH >> 1) + 16) * ((MPEG4_MAX_HEIGHT >> 1) + 16));
   #endif

		DrawMemoryBlock(Mpeg_addr, IIS_BUF_NUM*IIS_CHUNK_SIZE, "iisSounBuf", 0);    	   	      					                        	   			   						        
        for(i = 0; i < IIS_BUF_NUM; i++)
        {
            iisSounBuf[i]       = Mpeg_addr;
            Mpeg_addr          += IIS_CHUNK_SIZE;
        }

   #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
   DrawMemoryBlock(Mpeg_addr, IMA_ADPCM_BLOCK_SIZE, "ImaAdpcmBuf", 0);    	   	      					                        	   			   						        
    ImaAdpcmBuf             = Mpeg_addr;
    Mpeg_addr              += IMA_ADPCM_BLOCK_SIZE;

	DrawMemoryBlock(Mpeg_addr, 0, "ImaAdpcmBufEnd", 1);    	   	      					                        	   			   						        
    ImaAdpcmBufEnd          = Mpeg_addr;
   #endif

    // Mpeg Video encoding/decoding bitstream buffer
   #if (RFI_TEST_RXRX_PROTOCOL_B1B2 ||  RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_RX_PROTOCOL_B1)    //與 RF RX Decoding buffer 共用
    //與 RF RX Decoding buffer 共用
   #else
    DrawMemoryBlock(Mpeg_addr, MPEG4_MAX_BUF_SIZE, "VideoBuf", 0);    	   	      					                        	   			   						        
    VideoBuf                = Mpeg_addr;
    Mpeg_addr              += MPEG4_MAX_BUF_SIZE;

	DrawMemoryBlock(Mpeg_addr, 0, "mpeg4VideBufEnd", 1);    	   	      					                        	   			   						        
    mpeg4VideBufEnd         = Mpeg_addr;
   #endif

   #if CDVR_LOG
    Mpeg_addr               = (u8*)(((u32)Mpeg_addr + 15) & ~15);
      DrawMemoryBlock(Mpeg_addr, LOG_FILE_MAX_SIZE, "LogFileBuf", 0);    	   	      					                        	   			   						        
    LogFileBuf              = Mpeg_addr;
    Mpeg_addr              += LOG_FILE_MAX_SIZE;

	DrawMemoryBlock(Mpeg_addr, 0, "LogFileBufEnd", 1);    	   	      					                        	   			   						        
    LogFileBufEnd           = Mpeg_addr;
   #endif

   #if (CDVR_TEST_LOG)
    Mpeg_addr               = (u8*)(((u32)Mpeg_addr + 15) & ~15);
     DrawMemoryBlock(Mpeg_addr, LOG_FILE_MAX_SIZE, "LogFileBuf", 0);    	   	      					                        	   			   						        
    LogFileBuf              = Mpeg_addr;
    Mpeg_addr              += LOG_FILE_MAX_SIZE;
	
	DrawMemoryBlock(Mpeg_addr, 0, "LogFileBufEnd", 1);    	   	      					                        	   			   						        
    LogFileBufEnd           = Mpeg_addr;
   #endif

   #if ((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
        (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
        (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) ||(SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
	DrawMemoryBlock(Mpeg_addr, 0, "mpeg4IndexBuf", 0);    	   	      					                        	   			   						        
    mpeg4IndexBuf           = Mpeg_addr;
    Mpeg_addr              += 0;
   #else
	DrawMemoryBlock(Mpeg_addr, MPEG4_INDEX_BUF_SIZE, "mpeg4IndexBuf", 0);    	   	      					                        	   			   						        
    mpeg4IndexBuf           = Mpeg_addr;
    Mpeg_addr              += MPEG4_INDEX_BUF_SIZE;
   #endif
  
   #ifndef DRAW_MEMORY_POOL  
    DEBUG_MAIN("Mpeg_addr:0x%08x~0x%08x:Mpeg(Audio) %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
   #endif
#endif


    //-----------------------------------------VMD mean buffer--------------------------------//
    Mpeg_addr_temp          = Mpeg_addr;
#if VMDSW
    Mpeg_addr              = (u8 *)(((u32)Mpeg_addr + 63) & ~63);
    #if INTERPOLATION
	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE_INTER, "VMDMeanBuf1", 0);    	   	      					                        	   			   						        
    VMDMeanBuf1            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE_INTER);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE_INTER, "VMDMeanBuf2", 0);    	   	      					                        	   			   						        	
    VMDMeanBuf2            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE_INTER);	
    #else
	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDMeanBuf1", 0);    	   	      					                        	   			   						        
    VMDMeanBuf1            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDMeanBuf2", 0);    	   	      					                        	   			   						        	
    VMDMeanBuf2            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);
    #endif

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDPositiveCnt_X", 0);    	   	      					                        	   			   						        	
    VMDPositiveCnt_X       = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDPositiveCnt_Y", 0);    	   	      					                        	   			   						        	
    VMDPositiveCnt_Y       = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDNegativeCnt_X", 0);    	   	      					                        	   			   						        		
    VMDNegativeCnt_X       = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDNegativeCnt_Y", 0);    	   	      					                        	   			   						        			
    VMDNegativeCnt_Y       = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDFilterPX", 0);    	   	      					                        	   			   						        			
    VMDFilterPX            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDFilterNX", 0);    	   	      					                        	   			   						        			
    VMDFilterNX            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDFilterPY", 0);    	   	      					                        	   			   						        			
    VMDFilterPY            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDFilterNY", 0);    	   	      					                        	   			   						        				
    VMDFilterNY            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDFlterMap", 0);    	   	      					                        	   			   						        				
    VMDFlterMap            = (u8 *)Mpeg_addr;
    Mpeg_addr             += (VMD_BUF_SIZE);

    for(i = 0; i < PeriodTime; i++)
    {
   	    char tmp[64]={0};
	    sprintf(tmp, "VMDPositive/NegativeBuf %d", i);	

		DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE*4, tmp, 0);    	   	      					                        	   			   						        				
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
	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDMotionPos_X", 0);    	   	      					                        	   			   						        				
    VMDMotionPos_X       = (u8 *)Mpeg_addr;
    Mpeg_addr                += (VMD_BUF_SIZE);

	DrawMemoryBlock(Mpeg_addr, VMD_BUF_SIZE, "VMDMotionPos_Y", 0);    	   	      					                        	   			   						        				
    VMDMotionPos_Y       = (u8 *)Mpeg_addr;
    Mpeg_addr                += (VMD_BUF_SIZE);

    #if 0
    for(i = 0; i < PeriodTime; i++)
    {
        VMDMotionBuf_X[i]       = (u8 *)Mpeg_addr;
        Mpeg_addr                += (VMD_BUF_SIZE);
        VMDMotionBuf_Y[i]       = (u8 *)Mpeg_addr;
        Mpeg_addr                += (VMD_BUF_SIZE);
    }
    #endif

    #endif
#endif

  #ifndef DRAW_MEMORY_POOL
    DEBUG_MAIN("addr:0x%08x~0x%08x: VMDBuf %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
  #endif

    //============P2P Video Buffer(App playback use)=============//
    Mpeg_addr_temp               = Mpeg_addr;
#if (NIC_SUPPORT)
   #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
	DrawMemoryBlock(Mpeg_addr, PNBUF_SIZE_Y/4, "P2PPBVideoBuf", 0);    	   	      					                        	   			   						        				
    P2PPBVideoBuf           = Mpeg_addr;
    Mpeg_addr              += PNBUF_SIZE_Y/2;
   #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
	DrawMemoryBlock(Mpeg_addr, PNBUF_SIZE_Y/2, "P2PPBVideoBuf", 0);    	   	      					                        	   			   						        				
    P2PPBVideoBuf           = Mpeg_addr;
    Mpeg_addr              += PNBUF_SIZE_Y/2;
   #elif(SW_APPLICATION_OPTION  == MR9211_DUALMODE_TX5)
     #if PWIFI_SUPPORT
    	DrawMemoryBlock(Mpeg_addr,0, "P2PPBVideoBuf", 0);    	   	      					                        	   			   						        				
        P2PPBVideoBuf           = Mpeg_addr;
        Mpeg_addr              += 0;
     #else
    	DrawMemoryBlock(Mpeg_addr, 0, "P2PPBVideoBuf", 0);    	   	      					                        	   			   						        				
        P2PPBVideoBuf           = Mpeg_addr;
        Mpeg_addr              += 0;
     #endif   
   #else
	DrawMemoryBlock(Mpeg_addr, PNBUF_SIZE_Y, "P2PPBVideoBuf", 0);    	   	      					                        	   			   						        				
    P2PPBVideoBuf           = Mpeg_addr;
    Mpeg_addr              += PNBUF_SIZE_Y;
   #endif
   
	DrawMemoryBlock(Mpeg_addr, 0, "P2PPBVideBufEnd", 1);    	   	      					                        	   			   						        				
    P2PPBVideBufEnd         = Mpeg_addr;
  #ifndef DRAW_MEMORY_POOL  
    DEBUG_MAIN("addr:0x%08x~0x%08x: P2P Video Buffer %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
  #endif
#endif

    Mpeg_addr_temp          = Mpeg_addr;
#if (USB_DONGLE_SUPPORT == 1)
    //===========USB used=============//
	DrawMemoryBlock(Mpeg_addr, 1024 *50, "usb_device_buf", 0);    	   	      					                        	   			   						        				
    usb_device_buf          = Mpeg_addr;
    Mpeg_addr              += 1024 *50;
#endif
#if USB_DONGLE_SUPPORT
	DrawMemoryBlock(Mpeg_addr, 2048 *512, "usb_AV_buf", 0);    	   	      					                        	   			   						        				
    usb_AV_buf              = Mpeg_addr;
    Mpeg_addr              += 2048 *512;

	DrawMemoryBlock(Mpeg_addr, 0, "usb_AV_buf_end", 1);    	   	      					                        	   			   						        				
    usb_AV_buf_end          = Mpeg_addr;
#endif
#if (USB_HOST == 1)
    /*
	//QH1
	Mpeg_addr                   += 0x3f ;
	Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*12), "usb_qh_buf_1", 0);    	   	      					                        	   			   						        				
    usb_qh_buf_1            = (u8 *)Mpeg_addr ;
    Mpeg_addr                   += (4*12) ;

	//QH2
	Mpeg_addr                   += 0x3f ;	
	Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*12), "usb_qh_buf_2", 0);    	   	      					                        	   			   						        				
	usb_qh_buf_2            = (u8 *)Mpeg_addr ;
    Mpeg_addr                   += (4*12) ;

    //QH3
	Mpeg_addr                   += 0x3f ;
	Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*12), "usb_qh_buf_3", 0);    	   	      					                        	   			   						        				
	usb_qh_buf_3            = (u8 *)Mpeg_addr ;
    Mpeg_addr                   += (4*12) ;

	//QTD
	Mpeg_addr                   += 0x1f ;
	Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*8)*30, "usb_qtd_buf", 0);    	   	      					                        	   			   						        				
	usb_qtd_buf           	= (u8 *)Mpeg_addr ;
    Mpeg_addr                   += (4*8)*30 ;

	//iTD1
	Mpeg_addr                   += 0x1f ;
	Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*16), "usb_itd_buf_1", 0);    	   	      					                        	   			   						        				
	usb_itd_buf_1           = (u8 *)Mpeg_addr ;
    Mpeg_addr                   += (4*16) ;

	//Page0
    Mpeg_addr                   += 0xfff ;
    Mpeg_addr                    = (u8 *)((u32)Mpeg_addr & 0xfffff000);
	DrawMemoryBlock(Mpeg_addr, 1024*4, "usb_Page_buf_0", 0);    	   	      					                        	   			   						        				
    usb_Page_buf_0          = (u32 *)Mpeg_addr ;
    Mpeg_addr                   +=  1024*4;
    */
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*12), "qh_list_addr", 0);    	   	      					                        	   			   						        				
    qh_list_addr        = (u8 *)Mpeg_addr ;
    Mpeg_addr           += (4*12) ;
    
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*12), "qh_addr", 0);    	   	      					                        	   			   						        				
	qh_addr             = (u8 *)Mpeg_addr ;
   	Mpeg_addr           += (4*12) ;
   	
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*12), "hub_qh_addr", 0);    	   	      					                        	   			   						        				
	hub_qh_addr         = (u8 *)Mpeg_addr ;
   	Mpeg_addr           += (4*12) ;
   	
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*12), "mouse_qh_addr", 0);    	   	      					                        	   			   						        				
	mouse_qh_addr       = (u8 *)Mpeg_addr ;
   	Mpeg_addr           += (4*12) ;
   	
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*12), "keyboard_qh_addr", 0);    	   	      					                        	   			   						        				
	keyboard_qh_addr    = (u8 *)Mpeg_addr ;
   	Mpeg_addr           += (4*12) ;
   	
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*8)*30, "qtd_addr[0]", 0);    	   	      					                        	   			   						        					
	qtd_addr[0]         = (u8 *)Mpeg_addr ;
    Mpeg_addr           += (4*8)*30 ;
    
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*8)*30, "qtd_addr[1]", 0);    	   	      					                        	   			   						        					
	qtd_addr[1]         = (u8 *)Mpeg_addr ;
    Mpeg_addr           += (4*8)*30;
    
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*8)*30, "qtd_addr[2]", 0);    	   	      					                        	   			   						        						
	qtd_addr[2]         = (u8 *)Mpeg_addr ;
    Mpeg_addr           += (4*8)*30;
    
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*8)*30, "qtd_addr[3]", 0);    	   	      					                        	   			   						        						
	qtd_addr[3]         = (u8 *)Mpeg_addr ;
    Mpeg_addr           += (4*8)*30;
    
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*8)*30, "qtd_addr[4]", 0);    	   	      					                        	   			   						        							
	qtd_addr[4]         = (u8 *)Mpeg_addr ;
    Mpeg_addr           += (4*8)*30;
    
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*8)*30, "hub_qtd", 0);    	   	      					                        	   			   						        								
	hub_qtd             = (u8 *)Mpeg_addr ;
    Mpeg_addr           += (4*8)*30;
    
	Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
	DrawMemoryBlock(Mpeg_addr, (4*8)*30, "mouse_qtd", 0);    	   	      					                        	   			   						        									
	mouse_qtd           = (u8 *)Mpeg_addr ;
    Mpeg_addr           += (4*8)*30;

    Mpeg_addr           += 0x1f ;
	Mpeg_addr           = (u8 *)((u32)Mpeg_addr & 0xffffffe0);
    DrawMemoryBlock(Mpeg_addr, (4*8)*30, "keyboard_qtd", 0);    	   	      					                        	   			   						        									
	keyboard_qtd        = (u8 *)Mpeg_addr ;
    Mpeg_addr           += (4*8)*30;

	periodic_base = (u32 *)(((u32)Mpeg_addr + 0x0FFF) & 0xFFFFF000);
	DrawMemoryBlock((u8 *)periodic_base, 4*1024, "periodic_base", 0);    	   	      					                        	   			   						        									
	Mpeg_addr           += 4*1024;
#endif

#ifndef DRAW_MEMORY_POOL 
    DEBUG_MAIN("addr:0x%08x~0x%08x: USB %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
#endif


#ifndef DRAW_MEMORY_POOL 
    DEBUG_MAIN("addr:0x%08x~0x%08x: iHome %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
#endif

    //=========Jpeg decbuf for UI=========//
    Mpeg_addr_temp          = Mpeg_addr;
    #if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION==MR8200_RFCAM_RX1RX2)  || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) ||\
        (SW_APPLICATION_OPTION==MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) ||  (SW_APPLICATION_OPTION == Standalone_Test)  )
        Mpeg_addr = (u8 *)(((u32)Mpeg_addr + 0x0FFF) & 0xFFFFF000);
    	DrawMemoryBlock(Mpeg_addr, 256 *1024, "exifDecBuf-re", 0);    	   	      					                        	   			   						        										
    	exifDecBuf = (u8 *)Mpeg_addr;
        Mpeg_addr +=256 *1024; //Max size = 256kb
    #elif FPGA_BOARD_A1018_SERIES
        Mpeg_addr = (u8 *)(((u32)Mpeg_addr + 0x0FFF) & 0xFFFFF000);
        DrawMemoryBlock(Mpeg_addr, 256 *1024, "exifDecBuf-re", 0);                                                                                                                                                  
        exifDecBuf = (u8 *)Mpeg_addr;
        Mpeg_addr +=256 *1024; //Max size = 256kb
    #else
        exifDecBuf = (u8 *)Mpeg_addr;
        Mpeg_addr += 0; //Max size = 256kb
	#endif
#ifndef DRAW_MEMORY_POOL 
    DEBUG_MAIN("addr:0x%08x~0x%08x: JPG-UI %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
#endif

    //========JPEG bitstream buffer for TX snapshot========//
#if TX_SNAPSHOT_SUPPORT
    Mpeg_addr_temp          = Mpeg_addr;
	Mpeg_addr = (u8 *)(((u32)Mpeg_addr + 0x0FFF) & 0xFFFFF000);
	DrawMemoryBlock(Mpeg_addr,MPEG4_MAX_WIDTH*MPEG4_MAX_HEIGHT/4, "sysRFTXImgData", 0);    	   	      					                        	   			   						        										
    sysRFTXImgData=Mpeg_addr;
    Mpeg_addr +=MPEG4_MAX_WIDTH*MPEG4_MAX_HEIGHT/4;
   #ifndef DRAW_MEMORY_POOL 
    DEBUG_MAIN("addr:0x%08x~0x%08x: JPG-TX Snapshot %d bytes\n", (u32)Mpeg_addr_temp, (u32)Mpeg_addr, (u32)Mpeg_addr - (u32)Mpeg_addr_temp);
   #endif
#endif	


    //========================================================//
    //DEBUG_MAIN("Mpeg_addr   = 0x%08x\n", Mpeg_addr);
    //DEBUG_MAIN("addr        = 0x%08x\n", addr);
	DrawMemoryPoolEnd();
	DumpMemoryOverlapInfo();
	DumpMemoryUnknownInfo();
	DumpMemoryZeroInfo();	

	/*
	for(i=0; i<DISPLAY_BUF_NUM; i++)
	{
		DEBUG_RED("MainVideodisplaybuf[%d]    = 0x%08x\n", i, (u32)MainVideodisplaybuf[i]);
		DEBUG_RED("MainVideodisplaybuf[%d].y  = 0x%08x\n", i, (u32)MainVideodisplaybuf[i]+DISPLAY_BUFFER_Y_OFFSET);
		DEBUG_RED("MainVideodisplaybuf[%d].uv = 0x%08x\n", i, (u32)MainVideodisplaybuf[i]+ULTRA_FHD_SIZE_Y+DISPLAY_BUFFER_UV_OFFSET);
		//DEBUG_RED("rfiuRxDecBuf[%d].mpeg4PRefBuf_Y = 0x%08x\n", i, rfiuRxDecBuf[i].mpeg4PRefBuf_Y);
		//DEBUG_RED("rfiuRxDecBuf[%d].mpeg4NRefBuf_Y = 0x%08x\n", i, rfiuRxDecBuf[i].mpeg4NRefBuf_Y);
		//DEBUG_RED("VideoClipOption[%d].VideoBuf = 0x%08x\n"      , i, VideoClipOption[i].VideoBuf);
		//DEBUG_RED("VideoClipOption[%d].mpeg4VideBufEnd = 0x%08x\n", i, VideoClipOption[i].mpeg4VideBufEnd);
	}
	*/
	
    if(Mpeg_addr>addr)
        return Mpeg_addr;
    else
        return addr;

	
}


#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (USE_NEW_MEMORY_MAP))
int memDispBufArrage(int mode)
{
    int i;
    u8 *addr;
    
    addr = PKBuf0; 
	//printf("VIDEODISPBUF_SIZE = 0x%08x\n", VIDEODISPBUF_SIZE);
    if(mode == DISPBUF_NORMAL)
    {       
       for(i = 0; i < DISPLAY_BUF_NUM; i++)
       {  
           MainVideodisplaybuf[i]  = addr; //Lsk: upper black area
           addr += VIDEODISPBUF_SIZE; 
		   //DEBUG_IDU("1. MainVideodisplaybuf[%d] = 0x%08x\n", i, MainVideodisplaybuf[i]);
       }
    }
    else if(mode == DISPBUF_9300FHD)
    {
       for(i = 0; i < DISPLAY_BUF_NUM; i++)
       {  
           MainVideodisplaybuf[i]  = addr;
           addr += 1920*1440*3/2; 
		   //DEBUG_IDU("2. MainVideodisplaybuf[%d] = 0x%08x\n", i, MainVideodisplaybuf[i]);		   
       }    
    }
}
#else
int memDispBufArrage(int mode)
{
    int i;
    u8 *addr;
    
    addr = PKBuf0; 
	//printf("VIDEODISPBUF_SIZE = 0x%08x\n", VIDEODISPBUF_SIZE);
    if(mode == DISPBUF_NORMAL)
    {       
       for(i = 0; i < DISPLAY_BUF_NUM; i++)
       {  
           MainVideodisplaybuf[i]  = addr;
           addr += VIDEODISPBUF_SIZE; 
		   //DEBUG_IDU("1. MainVideodisplaybuf[%d] = 0x%08x\n", i, MainVideodisplaybuf[i]);
       }
    }
    else if(mode == DISPBUF_9300FHD)
    {
       for(i = 0; i < DISPLAY_BUF_NUM; i++)
       {  
           MainVideodisplaybuf[i]  = addr;
           addr += 1920*1440*3/2; 
		   //DEBUG_IDU("2. MainVideodisplaybuf[%d] = 0x%08x\n", i, MainVideodisplaybuf[i]);		   
       }    
    }
}
#endif
