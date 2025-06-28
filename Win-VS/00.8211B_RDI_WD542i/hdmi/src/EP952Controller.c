/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005 
                           ALL RIGHTS RESERVED 
 
--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP952Controller.c 

  Description :  EP952Controller program 
                 Control SFR directory and use HCI functions 

  Codeing     :  Shihken

\******************************************************************************/


// standard Lib
#include <stdlib.h>
#include <string.h>

#include "EP952api.h"
//#include "Edid.h"
//#include "DDC_If.h"
#include "EP952Controller.h"
#include "EP952SettingsData.h"

#include "EP952RegDef.h"

//
// Defines
//


#if Enable_HDCP
// HDCP Key  
unsigned char HDCP_Key[64][8];
#endif

//
// Global State and Flags
//

// System flags
unsigned char is_Cap_HDMI;
unsigned char is_Cap_YCC444;
unsigned char is_Cap_YCC422;
unsigned char is_Forced_Output_RGB;

unsigned char is_Hot_Plug;
unsigned char is_Connected;
unsigned char is_ReceiverSense;
unsigned char ChkSum, ConnectionState;
unsigned char EP952_Debug = 0;

#if Enable_HDCP
unsigned char is_HDCP_Info_BKSV_Rdy;
#endif


// System Data
unsigned char HP_ChangeCount;
unsigned char PowerUpCount;
TX_STATE TX_State;
VDO_PARAMS Video_Params;
ADO_PARAMS Audio_Params;
PEP952C_REGISTER_MAP pEP952C_Registers;

unsigned char is_RSEN;
unsigned char Cache_EP952_DE_Control;

//
// Private Functions
//
void EP952_HDCP_Reset(void);
void TXS_RollBack_Stream_952(void);
void EP_HDMI_DumpMessage_952(void);

#if Enable_HDCP
void TXS_RollBack_HDCP(void);
#endif

void EP952_Reg_Write(unsigned char Addr, unsigned char Data, unsigned int size)
{
    i2cWrite_Byte(0x52, Addr, Data);
}

void EP952_Reg_Read(unsigned char Addr, unsigned char *buf, unsigned int size)
{
    i2cRead_Byte(0x53, Addr, buf);
}

SMBUS_STATUS EP952_Reg_Clear_Bit(unsigned char ByteAddr, unsigned char BitMask)
{
	int result = 1;
    unsigned char Temp_Data[15];
    
	result = i2cRead_Byte(0x53, ByteAddr, Temp_Data);
	if(result == 1)
	{
		// Write back to Reg Reg_Addr
		Temp_Data[0] &= ~BitMask;
		
		return i2cWrite_Byte(0x52, ByteAddr, Temp_Data[0]);
	}
	else
	{
		return result;
	}
}

SMBUS_STATUS EP952_Reg_Set_Bit(unsigned char ByteAddr, unsigned char BitMask)
{
	int result = 0;
    unsigned char temp ;
	//result = IIC_Read(IIC_EP932_Addr, ByteAddr, Temp_Data, 1);
    result = i2cRead_Byte(0x53, ByteAddr, &temp);
	if(result == 1)
	{
		// Write back to Reg Reg_Addr
		temp |= BitMask;
		
		//return IIC_Write(IIC_EP932_Addr, ByteAddr, Temp_Data, 1);
		return i2cWrite_Byte(0x52, ByteAddr, temp);
	}
    else if(result == 0)
	{
		return result;
	}
}

void EP952_IIC_Initial()
{
    unsigned char Temp_Data[15];
	unsigned char check ;
	// Initial Variables
    EP952_Reg_Read(EP952_TX_PHY_Control_0, &check, 1);
    DBG_printf("Before %x\n", check);
    EP952_Reg_Read(EP952_General_Control_1, &check, 1);
    DBG_printf("Addr 08 %x\n", check);
    
	Temp_Data[0] = EP952_TX_PHY_Control_0__TERM_ON;
	EP952_Reg_Write(EP952_TX_PHY_Control_0, Temp_Data[0], 1);
    EP952_Reg_Read(EP952_TX_PHY_Control_0, &check, 1);
    DBG_printf("Check %x\n", check);

	Temp_Data[0] = 0;
	EP952_Reg_Write(EP952_TX_PHY_Control_1, Temp_Data[0], 1);
}

void HDMI_Tx_Power_Down_952(void)
{
	// Software power down
	EP952_Reg_Clear_Bit(EP952_General_Control_1, EP952_General_Control_1__PU);
	DBG_printf("< EP952 Tx Power Down >\r\n");	
}

void HDMI_Tx_AMute_Enable_952(void)
{
	EP952_Reg_Set_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__AMUTE);
	EP952_Reg_Set_Bit(EP952_Pixel_Repetition_Control, EP952_Pixel_Repetition_Control__CTS_M);

	EP952_Reg_Clear_Bit(EP952_IIS_Control, EP952_IIS_Control__ADO_EN);		
	EP952_Reg_Clear_Bit(EP952_IIS_Control, EP952_IIS_Control__AUDIO_EN);	

	DBG_printf("< EP952 Audio_Mute_enable >\r\n");
}

void HDMI_Tx_VMute_Enable_952(void)
{		
	EP952_Reg_Set_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__VMUTE);

	DBG_printf("< EP952 Video_Mute_enable >\r\n");
}

void HDMI_Tx_Mute_Enable_952(void)
{
	HDMI_Tx_AMute_Enable_952();
	HDMI_Tx_VMute_Enable_952();	
}

void HDMI_Tx_HDCP_Disable_952(void)
{
	EP952_Reg_Clear_Bit(EP952_General_Control_5, EP952_General_Control_5__ENC_EN);
}

void EP952_Info_Reset(void)
{
	int i;
    unsigned char Temp_Data[15];
    unsigned char check ;

	// Global date for HDMI Transmiter
	is_RSEN = 0;
	Cache_EP952_DE_Control = 0x03;

	// Initial Settings
	EP952_Reg_Set_Bit(EP952_Packet_Control, EP952_Packet_Control__VTX0);
	EP952_Reg_Set_Bit(EP952_General_Control_1, EP952_General_Control_1__INT_OD);
    EP952_Reg_Read(EP952_General_Control_1, &check, 1);
    //DBG_printf("# check addr 08 %x\n", check);
	//
	// Set Default AVI Info Frame
	//
	memset(Temp_Data, 0x00, 14);

	// Set AVI Info Frame to RGB
	Temp_Data[1] &= 0x60;
	Temp_Data[1] |= 0x00; // RGB

	// Set AVI Info Frame to 601
	Temp_Data[2] &= 0xC0;
	Temp_Data[2] |= 0x40;

	// Write AVI Info Frame
	Temp_Data[0] = 0;
	for(i=1; i<14; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);
	//EP952_Reg_Write(EP952_AVI_Packet, Temp_Data, 14);
	for(i=0; i<14; ++i)
	    EP952_Reg_Write((EP952_AVI_Packet + i), Temp_Data[i], 1);
	    

	//
	// Set Default ADO Info Frame
	//
	memset(Temp_Data, 0x00, 6);

	// Write ADO Info Frame
	Temp_Data[0] = 0;
	for(i=1; i<6; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);
	//EP952_Reg_Write(EP952_ADO_Packet, Temp_Data, 6);
    for(i=0; i<6; ++i)
	    EP952_Reg_Write((EP952_ADO_Packet + i), Temp_Data[i], 1);

	//
	// Set Default CS Info Frame
	//
	memset(Temp_Data, 0x00, 5);
		
	//EP952_Reg_Write(EP952_Channel_Status, Temp_Data, 5);
    for(i=0; i<5; ++i)
	    EP952_Reg_Write((EP952_Channel_Status + i), Temp_Data[i], 1);
/*
	//
	// clear Packet_1 Info Frame
	//
	Temp_Data[0] = 0;
	for(i=EP952_Data_Packet_Header; i<= 0x5F; i++) {	
		EP952_Reg_Write(i, Temp_Data, 1);
	}
*/
}

unsigned char HDMI_Tx_HTPLG_952(void)
{
    unsigned char Temp_Data[15];
    
	// EP952 RSEN Enable
	EP952_Reg_Clear_Bit(EP952_TX_PHY_Control_1, EP952_TX_PHY_Control_1__RESN_DIS);
	
	// read register 
	EP952_Reg_Read(EP952_General_Control_2, Temp_Data, 1);

#if Enable_HDCP
	// check HDCP Ri Interrupt Flag 
	if(Temp_Data[0] & EP952_General_Control_2__RIFE) {
		HDCP_Ext_Ri_Trigger();
	}
#endif
	
	// check RSEN status
	is_RSEN = (Temp_Data[0] & EP952_General_Control_2__RSEN)? 1:0;

	// return HTPLG status
	if(Temp_Data[0] & EP952_General_Control_2__HTPLG) 
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

unsigned char HDMI_Tx_RSEN_952(void)
{
	return is_RSEN;
}

Downstream_Rx_read_EDID_952(unsigned char *pEDID)
{
	int i;
	unsigned char seg_ptr = 1, BlockCount = 0, Block1Found = 0; 
    unsigned char DDC_Data[128];

	// =========================================================
	// I. Read the block 0

	//status = DDC_Read(EDID_ADDR, 0, pEDID, 128);
	for(i=0; i<128 ; i++)
    {
        i2cRead_Byte(EDID_ADDR, (0x00 + i), (pEDID + i));
    }
    /*
	if(status != SMBUS_STATUS_Success) {
		DBG_printf("ERROR: EDID b0 read - DN DDC %d\r\n", (int)status);
		return status;
	}
	*/
	//DBG_printf("EDID b0 read:");
	/*
	for(i=0; i<128; ++i) {
		if(i%16 == 0) DBG_printf("\r\n");
		if(i%8 == 0) DBG_printf(" ");
		DBG_printf("0x%02X, ", (int)pEDID[i] );
	}
	DBG_printf("\r\n");
	*/
	
	// EDID header check fail
	if( (pEDID[0] != 0x00) ||
	    (pEDID[1] != 0xFF) ||
	    (pEDID[2] != 0xFF) ||
	    (pEDID[3] != 0xFF) ||
	    (pEDID[4] != 0xFF) ||
	    (pEDID[5] != 0xFF) ||
	    (pEDID[5] != 0xFF) ||
	    (pEDID[7] != 0x00))
	{
	    DBG_printf("EDID header check fail \n");
        DBG_printf("pEDID[0] = %x\n", pEDID[0]);
        DBG_printf("pEDID[1] = %x\n", pEDID[1]);
        DBG_printf("pEDID[2] = %x\n", pEDID[2]);
        DBG_printf("pEDID[3] = %x\n", pEDID[3]);
        DBG_printf("pEDID[4] = %x\n", pEDID[4]);
        DBG_printf("pEDID[5] = %x\n", pEDID[5]);
        DBG_printf("pEDID[6] = %x\n", pEDID[6]);
        DBG_printf("pEDID[7] = %x\n", pEDID[7]);
        
		//return EDID_STATUS_ExtensionOverflow;
	}


	// =========================================================
	// II. Read other blocks and find Timing Extension Block

	BlockCount = pEDID[126];
	Block1Found = 0;
	if(BlockCount >= 8)
	{
		BlockCount = 1;	
	}

	if(BlockCount != 0)
	{
		for(seg_ptr = 1; seg_ptr <= BlockCount; ++seg_ptr) 
		{
			//status = DDC_Read(EDID_ADDR, (seg_ptr & 0x01) << 7, DDC_Data, 128);
			for(i=0; i<128 ; i++)
            {
                i2cRead_Byte(EDID_ADDR, (0x80 + i), (DDC_Data + i));
            }
			/*
			if(status != SMBUS_STATUS_Success) {
				DBG_printf("ERROR: EDID bi read - DN DDC %d\r\n", (int)status);
				return status;
			}
			*/
	
			// Check EDID
			if(DDC_Data[0] == 0x02 && Block1Found == 0) {
				Block1Found = 1;
				memcpy(&pEDID[128], DDC_Data, 128);
			}
			
			//DBG_printf("EDID b%d read:", (int)seg_ptr);
			for(i=0; i<128; ++i) {
				if(i%16 == 0) DBG_printf("\r\n");
				if(i%8 == 0) DBG_printf(" ");
				DBG_printf("0x%02X, ", (int)DDC_Data[i] );
			}
			DBG_printf("\r\n");
		}
		DBG_printf("\r\n");
	}
		
	if(Block1Found) {
		pEDID[126] = 1;
	}
	else {
		pEDID[126] = 0;
	}

	return 0;
}

unsigned char EDID_GetHDMICap_952(unsigned char *pTarget)
{
    int i;
    
	if(pTarget[126] == 0x01) {
		for(i=4; i<pTarget[EDID_BLOCK_SIZE+2]; ++i) {
			if((pTarget[EDID_BLOCK_SIZE+i] & 0xE0) == 0x60) { // find tag code - Vendor Specific Block
				if( (pTarget[EDID_BLOCK_SIZE+1+i] == 0x03) && (pTarget[EDID_BLOCK_SIZE+2+i] == 0x0C) && (pTarget[EDID_BLOCK_SIZE+3+i] == 0x00) ) {
					return 1;
				}
			}
			else {
				i += (pTarget[EDID_BLOCK_SIZE+i] & 0x1F);
			}
		}
		if(i>=pTarget[EDID_BLOCK_SIZE+2]) { // Error, can not find the Vendor Specific Block
			return 0;
		}
	}
	return 0;
}

unsigned char EDID_GetPCMFreqCap_952(unsigned char *pTarget)
{
    int i, j;
    
	if(pTarget[126] >= 0x01) {
		for(i=4; i<pTarget[EDID_BLOCK_SIZE+2]; ++i) {
			if((pTarget[EDID_BLOCK_SIZE+i] & 0xE0) == 0x20) { // find tag code - Audio Data Block
				for(j=1; j<(pTarget[EDID_BLOCK_SIZE+i] & 0x1F); j+=3) {
					if((pTarget[EDID_BLOCK_SIZE+i+j] >> 3) == 1) {
						return pTarget[EDID_BLOCK_SIZE+i+j+1];
					}
				}
			}
			else {
				i += (pTarget[EDID_BLOCK_SIZE+i] & 0x1F);
			}
		}
		if(i>=pTarget[EDID_BLOCK_SIZE+2]) { // Error, can not find the Audio Data Block
			return 0x07;
		}
	}
	return 0x00;
}

void HDMI_Tx_Audio_Config_952(PADO_PARAMS Params)
{
	int i;
	unsigned char N_CTS_Index;
	unsigned long N_Value, CTS_Value;
	ADSFREQ FinalFrequency;
	unsigned char FinalADSRate;
    unsigned char Temp_Data[15];

	DBG_printf("\r\n ========== Update EP952 Audio Registers ==========\r\n");

	////////////////////////////////////////////////////////
	// Audio Settings
	
	// Update WS_M, WS_POL, SCK_POL
	EP952_Reg_Read(EP952_IIS_Control, Temp_Data, 1);
	Temp_Data[0] &= ~0x07;						//clear WS_M, WS_POL, SCK_POL
	Temp_Data[0] |= Params->Interface & 0x07; 	//set WS_M, WS_POL, SCK_POL
	EP952_Reg_Write(EP952_IIS_Control, Temp_Data[0], 1);

	////////////////////////////////////////////////////////
	// IIS or SPDIF
	if(Params->Interface & 0x08) { // IIS
		//DBG_printf("EP952 set to IIS IN \r\n");
		Temp_Data[0] = 0;
		
		// Update Flat = 0
		EP952_Reg_Clear_Bit(EP952_Packet_Control, EP952_Packet_Control__FLAT);

		// Power down OSC
		EP952_Reg_Set_Bit(EP952_General_Control_1, EP952_General_Control_1__OSC_PD);
		
		// Set to IIS Input
		Temp_Data[0] = EP952_General_Control_8__ADO_IIS_IN | EP952_General_Control_8__COMR_DIS;
		EP952_Reg_Write(EP952_General_Control_8, Temp_Data[0], 1); 

		// Downsample Convert
		FinalADSRate = Params->ADSRate;
		switch(Params->ADSRate) {
			default:
			case 0: // Bypass
				//DBG_printf("Audio ADS = 0\r\n");
				FinalADSRate = 0;
				FinalFrequency = Params->InputFrequency;
				break;
			case 1: // 1/2
				//DBG_printf("Audio ADS = 1_2\r\n");
				switch(Params->InputFrequency) {
					default: // Bypass
						//DBG_printf("Audio ADS = 0\r\n");
						FinalADSRate = 0;
						FinalFrequency = Params->InputFrequency;
						break;
					case ADSFREQ_88200Hz:
						FinalFrequency = ADSFREQ_44100Hz;
						break;
					case ADSFREQ_96000Hz:
						FinalFrequency = ADSFREQ_48000Hz;
						break;
					case ADSFREQ_176400Hz:
						FinalFrequency = ADSFREQ_88200Hz;
						break;
					case ADSFREQ_192000Hz:
						FinalFrequency = ADSFREQ_96000Hz;
						break;
				}
				break;
			case 2: // 1/3
				//DBG_printf("Audio ADS = 1_3\r\n");
				switch(Params->InputFrequency) {
					default: // Bypass
						//DBG_printf("Audio ADS = 0\r\n");
						FinalADSRate = 0;
						FinalFrequency = Params->InputFrequency;
						break;
					case ADSFREQ_96000Hz:
						FinalFrequency = ADSFREQ_32000Hz;
						break;
				}
				break;
			case 3: // 1/4
				//DBG_printf("Audio ADS = 1_4\r\n");
				switch(Params->InputFrequency) {
					default: // Bypass
						//DBG_printf("Audio ADS = 0\r\n");
						FinalADSRate = 0;
						FinalFrequency = Params->InputFrequency;
						break;
					case ADSFREQ_176400Hz:
						FinalFrequency = ADSFREQ_44100Hz;
						break;
					case ADSFREQ_192000Hz:
						FinalFrequency = ADSFREQ_48000Hz;
						break;
				}
				break;
		}
	
		// Update Audio Down Sample (ADSR) 
		EP952_Reg_Read(EP952_Pixel_Repetition_Control, Temp_Data, 1);
		Temp_Data[0] &= ~0x30;
		Temp_Data[0] |= (FinalADSRate << 4) & 0x30;
		EP952_Reg_Write(EP952_Pixel_Repetition_Control, Temp_Data[0], 1);

		///////////////////////////////////////////////////////////////
		// Channel Status
		
		memset(Temp_Data, 0x00, 5);
		
		Temp_Data[0] = (Params->NoCopyRight)? 0x04:0x00;
		Temp_Data[1] = 0x00; 			// Category code ??
		Temp_Data[2] = 0x00; 			// Channel number ?? | Source number ??
		Temp_Data[3] = FinalFrequency; 	// Clock accuracy ?? | Sampling frequency
		Temp_Data[4] = 0x01; 			// Original sampling frequency ?? | Word length ??
		//EP952_Reg_Write(EP952_Channel_Status, Temp_Data, 5);
        for(i=0 ; i<5 ; i++)
            EP952_Reg_Write((EP952_Channel_Status + i), Temp_Data[i], 1);
		// print for debug
		DBG_printf("EP952 set CS Info: ");
		for(i=0; i<5; ++i) {
			DBG_printf("0x%02X, ", (int)Temp_Data[i] );
		}
		DBG_printf("\r\n");

		// set CS_M = 1 (use channel status regiater)
		EP952_Reg_Set_Bit(EP952_Pixel_Repetition_Control, EP952_Pixel_Repetition_Control__CS_M);
	}
	else { // SPIDIF
		DBG_printf("EP952 set to SPDIF IN \r\n");
	
		// power up OSC
		EP952_Reg_Clear_Bit(EP952_General_Control_1, EP952_General_Control_1__OSC_PD);

		// Set SPDIF in
		Temp_Data[0] = EP952_General_Control_8__ADO_SPDIF_IN | EP952_General_Control_8__COMR_DIS;
		EP952_Reg_Write(EP952_General_Control_8, Temp_Data[0], 1); 
		
		// Update Flat = 0
		EP952_Reg_Clear_Bit(EP952_Packet_Control, EP952_Packet_Control__FLAT);

		// No Downsample
		FinalADSRate = 0;
		FinalFrequency = Params->InputFrequency;

		// Disable Down Sample and Bypass Channel Status
		EP952_Reg_Clear_Bit(EP952_Pixel_Repetition_Control, EP952_Pixel_Repetition_Control__ADSR | EP952_Pixel_Repetition_Control__CS_M);

		Params->ChannelNumber = 0;
	}

	////////////////////////////////////////////////////////
	// Set CTS/N
	if(Params->VideoSettingIndex < EP952_VDO_Settings_Max) {	
		N_CTS_Index = EP952_VDO_Settings[Params->VideoSettingIndex].Pix_Freq_Type;
		if(EP952_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.Vprd % 500) { // 59.94/60 Hz
			N_CTS_Index += Params->VFS;	
			//DBG_printf("EP952 Use N_CTS_Index shift(VFS) = %d\r\n", (int)Params->VFS);				 
		}
	}
	else {
		DBG_printf("EP952 Use default N_CTS_Index\r\n");
		N_CTS_Index = PIX_FREQ_25200KHz;
	}

	switch(FinalFrequency) {

		default:
		case ADSFREQ_32000Hz:
			DBG_printf("EP952 Set to 32KHz");
			N_Value = EP952_N_CTS_32K[N_CTS_Index].N;
			CTS_Value = EP952_N_CTS_32K[N_CTS_Index].CTS;
			break;
		case ADSFREQ_44100Hz:
			DBG_printf("EP952 Set to 44.1KHz");
			N_Value = EP952_N_CTS_44K1[N_CTS_Index].N;
			CTS_Value = EP952_N_CTS_44K1[N_CTS_Index].CTS;
			break;
		case ADSFREQ_48000Hz:
			DBG_printf("EP952 Set to 48KHz");
			N_Value = EP952_N_CTS_48K[N_CTS_Index].N;
			CTS_Value = EP952_N_CTS_48K[N_CTS_Index].CTS;
			break;
		case ADSFREQ_88200Hz:
			DBG_printf("EP952 Set to 88.2KHz");
			N_Value = EP952_N_CTS_44K1[N_CTS_Index].N * 2;
			CTS_Value = EP952_N_CTS_44K1[N_CTS_Index].CTS * 2;
			break;
		case ADSFREQ_96000Hz:
			DBG_printf("EP952 Set to 96KHz");
			N_Value = EP952_N_CTS_48K[N_CTS_Index].N * 2;
			CTS_Value = EP952_N_CTS_48K[N_CTS_Index].CTS * 2;
			break;
		case ADSFREQ_176400Hz:
			DBG_printf("EP952 Set to 176.4KHz");
			N_Value = EP952_N_CTS_44K1[N_CTS_Index].N * 4;
			CTS_Value = EP952_N_CTS_44K1[N_CTS_Index].CTS * 4;
			break;
		case ADSFREQ_192000Hz:
			DBG_printf("EP952 Set to 192KHz");
			N_Value = EP952_N_CTS_48K[N_CTS_Index].N * 4;
			CTS_Value = EP952_N_CTS_48K[N_CTS_Index].CTS * 4;
			break;
	}

	// write to EP952 - CTS.N value
	Temp_Data[0] = CTS_Value>>16;
	EP952_Reg_Write(EP952_CTS_H, Temp_Data[0], 1);
	Temp_Data[0] = CTS_Value>>8;
	EP952_Reg_Write(EP952_CTS_M, Temp_Data[0], 1);
	Temp_Data[0] = CTS_Value;
	EP952_Reg_Write(EP952_CTS_L, Temp_Data[0], 1);

	Temp_Data[0] = N_Value>>16;
	EP952_Reg_Write(EP952_N_H, Temp_Data[0], 1);
	Temp_Data[0] = N_Value>>8;
	EP952_Reg_Write(EP952_N_M, Temp_Data[0], 1);
	Temp_Data[0] = N_Value;
	EP952_Reg_Write(EP952_N_L, Temp_Data[0], 1);
	
	DBG_printf(" table[%d]: N=%ld, CTS=%ld (VIC=%d)\r\n", (int)N_CTS_Index, N_Value, CTS_Value, (int)Params->VideoSettingIndex);
	
	/*
	// for debug
	EP952_Reg_Read(EP952_CTS_H, Temp_Data, 1);
	DBG_printf("EP952_CTS_0(Reg addr 0x60) = 0x%02X\r\n",(int)Temp_Data[0]);
	EP952_Reg_Read(EP952_CTS_M, Temp_Data, 1);
	DBG_printf("EP952_CTS_1(Reg addr 0x61) = 0x%02X\r\n",(int)Temp_Data[0]);
	EP952_Reg_Read(EP952_CTS_L, Temp_Data, 1);
	DBG_printf("EP952_CTS_2(Reg addr 0x62) = 0x%02X\r\n",(int)Temp_Data[0]);
	
	EP952_Reg_Read(EP952_N_H, Temp_Data, 1);
	DBG_printf("EP952_N_0(Reg addr 0x63) = 0x%02X\r\n",(int)Temp_Data[0]);
	EP952_Reg_Read(EP952_N_M, Temp_Data, 1);
	DBG_printf("EP952_N_1(Reg addr 0x64) = 0x%02X\r\n",(int)Temp_Data[0]);
	EP952_Reg_Read(EP952_N_L, Temp_Data, 1);
	DBG_printf("EP952_N_2(Reg addr 0x65) = 0x%02X\r\n",(int)Temp_Data[0]);
	*/

	//////////////////////////////////////////////////////
	// ADO InfoFrame
	//

	// clear Default ADO InfoFrame
	memset(Temp_Data, 0x00, 6);

	// Overwrite ADO InfoFrame	
	Temp_Data[1] = Params->ChannelNumber;
	Temp_Data[4] = EP952_ADO_Settings[Params->ChannelNumber].SpeakerMapping;
	
	// ADO InfoFrame data byte 0 is checksum
	Temp_Data[0] = 0x8F;
	for(i=1; i<6; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);
	 
	// Write ADO Info Frame back
	//EP952_Reg_Write(EP952_ADO_Packet, Temp_Data, 6);
    for(i=0 ; i<6 ; i++)
            EP952_Reg_Write((EP952_ADO_Packet + i), Temp_Data[i], 1);

	// print for Debug 
	DBG_printf("EP952 set ADO Info: ");
	for(i=0; i<6; ++i) {
		DBG_printf("[%d]0x%0.2X, ",(int)i, (int)Temp_Data[i] );
	}
	DBG_printf("\r\n");

	// enable ADO packet
	EP952_Reg_Set_Bit(EP952_IIS_Control, EP952_IIS_Control__ACR_EN | EP952_IIS_Control__ADO_EN | EP952_IIS_Control__GC_EN | EP952_IIS_Control__AUDIO_EN);
}

void HDMI_Tx_AMute_Disable_952(void)
{
	EP952_Reg_Clear_Bit(EP952_Pixel_Repetition_Control, EP952_Pixel_Repetition_Control__CTS_M);
	EP952_Reg_Clear_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__AVMUTE);
	EP952_Reg_Clear_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__AMUTE);

	EP952_Reg_Set_Bit(EP952_IIS_Control, EP952_IIS_Control__ADO_EN);	
	EP952_Reg_Set_Bit(EP952_IIS_Control, EP952_IIS_Control__AUDIO_EN);

	DBG_printf("< EP952 Audio_Mute_disable >\r\n");
}

void HDMI_Tx_VMute_Disable_952(void)
{
	EP952_Reg_Clear_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__AVMUTE);
	EP952_Reg_Clear_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__VMUTE);

	DBG_printf("< EP952 Video_Mute_disable >\r\n");
}

void HDMI_Tx_DVI_952(void)
{
	EP952_Reg_Clear_Bit(EP952_General_Control_4, EP952_General_Control_4__HDMI);
	DBG_printf("EP952 Set to DVI output mode\r\n");
}

void HDMI_Tx_HDMI_952(void)
{
	EP952_Reg_Set_Bit(EP952_General_Control_4, EP952_General_Control_4__HDMI);
	DBG_printf("EP952 Set to HDMI output mode\r\n");
}

void HDMI_Tx_Video_Config_952(PVDO_PARAMS Params)
{
	int i;
    unsigned char Temp_Data[15];
    unsigned short TempUSHORT;
    
	DBG_printf("\r\n ========== Update EP952 video Registers ==========\r\n");

	// Disable auto transmission AVI packet 
	EP952_Reg_Clear_Bit(EP952_IIS_Control, EP952_IIS_Control__AVI_EN);

	////////////////////////////////////////////////////////
	// Video Interface
	
	// De_Skew
	EP952_Reg_Read(EP952_General_Control_3, Temp_Data, 1);
	Temp_Data[0] &= ~0xF0;
	Temp_Data[0] |= Params->Interface & 0xF0;
	EP952_Reg_Write(EP952_General_Control_3, Temp_Data[0], 1);	
	
	// input DSEL BSEL EDGE
	EP952_Reg_Read(EP952_General_Control_1, Temp_Data, 1);
	Temp_Data[0] &= ~0x0E;
	Temp_Data[0] |= Params->Interface & 0x0E;
	EP952_Reg_Write(EP952_General_Control_1, Temp_Data[0], 1);	

	if(Params->Interface & 0x01) {
		EP952_Reg_Set_Bit(EP952_General_Control_4, EP952_General_Control_4__FMT12);
	}
	else {
		EP952_Reg_Clear_Bit(EP952_General_Control_4, EP952_General_Control_4__FMT12);
	}

	// update HVPol
	EP952_Reg_Read(EP952_General_Control_4, Temp_Data, 1);
	Params->HVPol = Temp_Data[0] & (EP952_DE_Control__VSO_POL | EP952_DE_Control__HSO_POL);	

	////////////////////////////////////////////////////////
	// Sync Mode
	switch(Params->SyncMode) {
		default:
	 	case SYNCMODE_HVDE:
			// Disable E_SYNC
			EP952_Reg_Clear_Bit(EP952_General_Control_4, EP952_General_Control_4__E_SYNC);
			// Disable DE_GEN
			Cache_EP952_DE_Control &= ~EP952_DE_Control__DE_GEN;
	
			// Regular VSO_POL, HSO_POL
			if((Params->HVPol & VNegHPos) != (EP952_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VNegHPos)) { // V
				Cache_EP952_DE_Control |= EP952_DE_Control__VSO_POL; // Invert
			}
			else {
				Cache_EP952_DE_Control &= ~EP952_DE_Control__VSO_POL;
			}
			if((Params->HVPol & VPosHNeg) != (EP952_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VPosHNeg)) { // H
				Cache_EP952_DE_Control |= EP952_DE_Control__HSO_POL; // Invert
			}
			else {
				Cache_EP952_DE_Control &= ~EP952_DE_Control__HSO_POL;
			}
			DBG_printf("EP952 Set Sync mode to (H+V+DE)input mode\r\n");
			break;

		case SYNCMODE_HV:
			// Disable E_SYNC
			EP952_Reg_Clear_Bit(EP952_General_Control_4, EP952_General_Control_4__E_SYNC);
			// Enable DE_GEN
			Cache_EP952_DE_Control |= EP952_DE_Control__DE_GEN;

			// Regular VSO_POL, HSO_POL
			if((Params->HVPol & VNegHPos) != (EP952_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VNegHPos)) { // V
				Cache_EP952_DE_Control |= EP952_DE_Control__VSO_POL; // Invert
			}
			else {
				Cache_EP952_DE_Control &= ~EP952_DE_Control__VSO_POL;
			}
			if((Params->HVPol & VPosHNeg) != (EP952_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VPosHNeg)) { // H
				Cache_EP952_DE_Control |= EP952_DE_Control__HSO_POL; // Invert
			}
			else {
				Cache_EP952_DE_Control &= ~EP952_DE_Control__HSO_POL;
			}

			// Set DE generation params
			if(Params->VideoSettingIndex < EP952_VDO_Settings_Max) {
				Cache_EP952_DE_Control &= ~0x03;

			#ifdef Little_Endian
				Cache_EP952_DE_Control |= ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY)[1];
				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY)[0];
				EP952_Reg_Write(EP952_DE_DLY, Temp_Data[0], 1);
			#else	// Big Endian
				Cache_EP952_DE_Control |= ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY)[0];
				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY)[1];
				EP952_Reg_Write(EP952_DE_DLY, Temp_Data[0], 1);
			#endif

				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_TOP)[0];
				EP952_Reg_Write(EP952_DE_TOP, Temp_Data[0], 1);
				
			#ifdef Little_Endian
				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT)[0];
				Temp_Data[1] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT)[1];
				EP952_Reg_Write(EP952_DE_CNT, Temp_Data[0], 1);
                EP952_Reg_Write((EP952_DE_CNT + 1), Temp_Data[1], 1);
			#else	// Big Endian
				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT)[1];
				Temp_Data[1] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT)[0];
				EP952_Reg_Write(EP952_DE_CNT, Temp_Data[0], 1);
                EP952_Reg_Write((EP952_DE_CNT + 1), Temp_Data[1], 1);
			#endif


			#ifdef Little_Endian
				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN)[1];
				Temp_Data[1] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN)[0];
				//EP952_Reg_Write(EP952_DE_LIN, Temp_Data, 2);
                EP952_Reg_Write(EP952_DE_LIN, Temp_Data[0], 1);
                EP952_Reg_Write((EP952_DE_LIN + 1), Temp_Data[1], 1);
			#else	// Big Endian
				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN)[1];
				Temp_Data[1] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN)[0];
				//EP952_Reg_Write(EP952_DE_LIN, Temp_Data, 2);
                EP952_Reg_Write(EP952_DE_LIN, Temp_Data[0], 1);
                EP952_Reg_Write((EP952_DE_LIN + 1), Temp_Data[1], 1);
			#endif
	
				DBG_printf("EP952 DE_GEN params (DE_DLY=%u", (unsigned short)EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY);
				DBG_printf(", DE_CNT=%u", (unsigned short)EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT);
				DBG_printf(", DE_TOP=%u", (unsigned short)EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_TOP);
				DBG_printf(", DE_LIN=%u)", (unsigned short)EP952_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN);
				DBG_printf("\r\n");
			}
			else {
				DBG_printf("ERROR: VideoCode overflow DE_GEN table\r\n");
			}

			DBG_printf("EP952 Set Sync mode to (H+V)input + DE_GEN mode\r\n");
			break;
			
		case SYNCMODE_Embeded:
			// Disable DE_GEN
			DBG_printf("# SYNCMODE_Embeded\n");
			Cache_EP952_DE_Control &= ~EP952_DE_Control__DE_GEN;
			// Enable E_SYNC
			EP952_Reg_Set_Bit(EP952_General_Control_4, EP952_General_Control_4__E_SYNC);
			
			// Set E_SYNC params
			if(Params->VideoSettingIndex < EP952_VDO_Settings_Max) {

				Temp_Data[0] = EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.CTL;
				EP952_Reg_Write(EP952_Embedded_Sync_Control, Temp_Data[0], 1);

				TempUSHORT = EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_DLY;
                DBG_printf("# H_DLY = %d\n", EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_DLY);
				if(!(Params->Interface & 0x04)) { // Mux Mode
					TempUSHORT += 2;
				}

			#ifdef Little_Endian
				Temp_Data[0] = ((unsigned char *)&TempUSHORT)[0];
				Temp_Data[1] = ((unsigned char *)&TempUSHORT)[1];
				//EP952_Reg_Write(EP952_H_Delay, Temp_Data, 2);
                EP952_Reg_Write(EP952_H_Delay, Temp_Data[0], 1);
                EP952_Reg_Write((EP952_H_Delay + 1), Temp_Data[1], 1);
			#else	// Big Endian
				Temp_Data[0] = ((unsigned char *)&TempUSHORT)[1];
				Temp_Data[1] = ((unsigned char *)&TempUSHORT)[0];
				//EP952_Reg_Write(EP952_H_Delay, Temp_Data, 2);
                EP952_Reg_Write(EP952_H_Delay, Temp_Data[1], 1);
                EP952_Reg_Write((EP952_H_Delay + 1), Temp_Data[0], 1);
			#endif


			#ifdef Little_Endian
				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH)[0];
				Temp_Data[1] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH)[1];
				//EP952_Reg_Write(EP952_H_Width, Temp_Data, 2);
                EP952_Reg_Write(EP952_H_Width, Temp_Data[0], 1);
                EP952_Reg_Write((EP952_H_Width + 1), Temp_Data[1], 1);
			#else	// Big Endian
				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH)[1];
				Temp_Data[1] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH)[0];
                DBG_printf("H_Width 0= %d\n", Temp_Data[0]);
                DBG_printf("H_Width 1= %d\n", Temp_Data[1]);
				//EP952_Reg_Write(EP952_H_Width, Temp_Data, 2);
                EP952_Reg_Write(EP952_H_Width, Temp_Data[1], 1);
                EP952_Reg_Write((EP952_H_Width + 1), Temp_Data[0], 1);
			#endif

				Temp_Data[0] = EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_DLY;
				EP952_Reg_Write(EP952_V_Delay, Temp_Data[0], 1);

				Temp_Data[0] = EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_WIDTH;
				EP952_Reg_Write(EP952_V_Width, Temp_Data[0], 1);

			#ifdef Little_Endian
				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST)[0];
				Temp_Data[1] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST)[1];
				//EP952_Reg_Write(EP952_V_Off_Set, Temp_Data, 2);
                EP952_Reg_Write(EP952_V_Off_Set, Temp_Data[0], 1);
                EP952_Reg_Write((EP952_V_Off_Set + 1), Temp_Data[1], 1);
			#else	// Big Endian
				Temp_Data[0] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST)[1];
				Temp_Data[1] = ((unsigned char *)&EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST)[0];
				//EP952_Reg_Write(EP952_V_Off_Set, Temp_Data, 2);
                EP952_Reg_Write(EP952_V_Off_Set, Temp_Data[0], 1);
                EP952_Reg_Write((EP952_V_Off_Set + 1), Temp_Data[1], 1);
			#endif
	
				DBG_printf("EP952 E_SYNC params (CTL=0x%02X", (unsigned short)EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.CTL);
				DBG_printf(", H_DLY=%u", (unsigned short)EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_DLY);
				DBG_printf(", H_WIDTH=%u", (unsigned short)EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH);
				DBG_printf(", V_DLY=%u", (unsigned short)EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_DLY);
				DBG_printf(", V_WIDTH=%u", (unsigned short)EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_WIDTH);
				DBG_printf(", V_OFST=%u", (unsigned short)EP952_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST);
				DBG_printf(")\r\n");

				// Regular VSO_POL, HSO_POL
				if(EP952_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VNegHPos) { // VNeg?
					Cache_EP952_DE_Control |= EP952_DE_Control__VSO_POL;
				}
				else {
					Cache_EP952_DE_Control &= ~EP952_DE_Control__VSO_POL;
				}
				if(EP952_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VPosHNeg) { // HNeg?
					Cache_EP952_DE_Control |= EP952_DE_Control__HSO_POL;
				}
				else {
					Cache_EP952_DE_Control &= ~EP952_DE_Control__HSO_POL;
				}
			}
			else {
				DBG_printf("ERROR: VideoCode overflow E_SYNC table\r\n");
			}

			DBG_printf("EP952 Set Sync mode to (Embeded)input mode\r\n");
			break;
	}
	EP952_Reg_Write(EP952_DE_Control, Cache_EP952_DE_Control, 1);
	
	////////////////////////////////////////////////////////
	// Pixel Repetition
	EP952_Reg_Read(EP952_Pixel_Repetition_Control, Temp_Data, 1);
	Temp_Data[0] &= ~EP952_Pixel_Repetition_Control__PR;
	if(Params->VideoSettingIndex < EP952_VDO_Settings_Max) {
		Temp_Data[0] |= EP952_VDO_Settings[Params->VideoSettingIndex].AR_PR & 0x03;
	}
	EP952_Reg_Write(EP952_Pixel_Repetition_Control, Temp_Data[0], 1);

	////////////////////////////////////////////////////////
	// Color Space
	switch(Params->FormatIn) {
		default:
	 	case COLORFORMAT_RGB:
			EP952_Reg_Clear_Bit(EP952_General_Control_4, EP952_General_Control_4__YCC_IN | EP952_General_Control_4__422_IN);
			DBG_printf("EP952 Set to RGB In\r\n");
			break;
	 	case COLORFORMAT_YCC444:
			EP952_Reg_Set_Bit(EP952_General_Control_4, EP952_General_Control_4__YCC_IN);
			EP952_Reg_Clear_Bit(EP952_General_Control_4, EP952_General_Control_4__422_IN);
			DBG_printf("EP952 Set to YCC444 In\r\n");
			break;
	 	case COLORFORMAT_YCC422:
			EP952_Reg_Set_Bit(EP952_General_Control_4, EP952_General_Control_4__YCC_IN | EP952_General_Control_4__422_IN);
			DBG_printf("EP952 Set to YCC422 In\r\n");
			break;
	}
	switch(Params->FormatOut) {
		default:
	 	case COLORFORMAT_RGB:
			// Set to RGB
			if(Params->VideoSettingIndex < EP952_VDO_Settings_IT_Start) { // CE Timing
				EP952_Reg_Clear_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__YCC_OUT | EP952_Color_Space_Control__422_OUT);
				EP952_Reg_Set_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__YCC_Range); // Output limit range RGB
			}
			else { // IT Timing
				EP952_Reg_Clear_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__YCC_OUT | EP952_Color_Space_Control__422_OUT | EP952_Color_Space_Control__YCC_Range);
			}
			DBG_printf("EP952 Set to RGB Out\r\n");
			break;

	 	case COLORFORMAT_YCC444:
			// Set to YCC444
			EP952_Reg_Set_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__YCC_OUT);
			EP952_Reg_Clear_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__422_OUT);
			DBG_printf("EP952 Set to YCC444 Out\r\n");
			break;
			
	 	case COLORFORMAT_YCC422:
			// Set to YCC422
			EP952_Reg_Set_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__YCC_OUT | EP952_Color_Space_Control__422_OUT);
			DBG_printf("EP952 Set to YCC422 Out\r\n");
			break;
	}

	// Color Space
	switch(Params->ColorSpace) {
		default:
	 	case COLORSPACE_601:
			// Set to 601
			EP952_Reg_Clear_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__COLOR);
			DBG_printf("EP952 Set to 601 color definition ");
			break;

	 	case COLORSPACE_709:
			// Set to 709
			EP952_Reg_Set_Bit(EP952_Color_Space_Control, EP952_Color_Space_Control__COLOR);
			DBG_printf("EP952 Set to 709 color definition ");
			break;
	}
	//DBG_printf("(VIC=%d) \r\n",(int)Params->VideoSettingIndex);

	///////////////////////////////////////////////////////////////////
	// AVI Info Frame
	//
	
	// clear AVI Info Frame
	memset(Temp_Data, 0x00, 14);
	
	// AVI InfoFrame Data Byte 1
	switch(Params->FormatOut) {
		default:
	 	case COLORFORMAT_RGB:
			Temp_Data[1] |= 0x00; // AVI_Y1,Y0 = RGB
			break;

	 	case COLORFORMAT_YCC444:
			Temp_Data[1] |= 0x40; // AVI_Y1,Y0 = YCC 444
			break;

	 	case COLORFORMAT_YCC422:
			Temp_Data[1] |= 0x20; // AVI_Y1,Y0 = YCC 422
			break;
	}
	Temp_Data[1] |= 0x10; // AVI_A0 = Active Format Information valid
	
	//SCAN
	switch(Params->SCAN) {
		default:
		case 0: 					
			Temp_Data[1] &= ~0x03;	// AVI_S1,S0 = No Data
			break;
		case 1: 					// AVI_S1,S0 = overscan
			Temp_Data[1] |= 0x01;	
			break;
		case 2: 					// AVI_S1,S0 = underscan
			Temp_Data[1] |= 0x02;	
			break;
	}

	// AVI InfoFrame Data Byte 2
	switch(Params->ColorSpace) {
		default:
	 	case COLORSPACE_601:
			Temp_Data[2] |= 0x40;	// AVI_C1,C0 = 601
			break;

	 	case COLORSPACE_709:
			Temp_Data[2] |= 0x80;	// AVI_C1,C0 = 709
			break;
	}

	if(Params->VideoSettingIndex < EP952_VDO_Settings_Max) {
		Temp_Data[2] |= EP952_VDO_Settings[Params->VideoSettingIndex].AR_PR & 0x30; // AVI_M1,M0 : Picture Aspect Ratio
	}
	Temp_Data[2] |= Params->AFARate & 0x0F;// AVI_R3~0: Active format Aspect Ratio

	// AVI InfoFrame Data Byte 3 is 0

	// AVI InfoFrame Data Byte 4 is VIC
	if(Params->VideoSettingIndex < EP952_VDO_Settings_IT_Start) {
		Temp_Data[4] |= EP952_VDO_Settings[Params->VideoSettingIndex].VideoCode;// AVI_VIC6~0 : Vedio Identification code
	}

	// AVI InfoFrame Data Byte 5
	if(Params->VideoSettingIndex < EP952_VDO_Settings_Max) {
		Temp_Data[5] |= (EP952_VDO_Settings[Params->VideoSettingIndex].AR_PR & 0x0C) >> 2;// AVI_PR3~0 : Pixel Repetition
	}

	// AVI InfoFrame Data Byte 0 is checksum
	Temp_Data[0] = 0x91;
	for(i=1; i<6; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);	// checksum
	
	// Write AVI InfoFrame Data Byte
	//EP952_Reg_Write(EP952_AVI_Packet, Temp_Data, 14);
    for(i=0 ; i<14 ; i++)
        EP952_Reg_Write((EP952_AVI_Packet+ i), Temp_Data[i], 1);

	// print for debug 
	DBG_printf("EP952 set AVI Info: ");
	for(i=0; i<6; ++i) {
		DBG_printf("[%d]0x%0.2X, ",(int)i, (int)Temp_Data[i] );
	}
	DBG_printf("\r\n");

	// Enable auto transmission AVI packet 
	EP952_Reg_Set_Bit(EP952_IIS_Control, EP952_IIS_Control__AVI_EN | EP952_IIS_Control__GC_EN);


}

void HDMI_Tx_Mute_Disable_952(void)
{
	HDMI_Tx_VMute_Disable_952();	
	HDMI_Tx_AMute_Disable_952();
}

void HDMI_Tx_Power_Up_952(void)
{
	// Software power up
	EP952_Reg_Set_Bit(EP952_General_Control_1, EP952_General_Control_1__PU);	
	DBG_printf("< EP952 Tx Power Up >\r\n");	
}

//--------------------------------------------------------------------------------------------------------------------

void EP952Controller_Initial(PEP952C_REGISTER_MAP pEP952C_RegMap)
{		
	DBG_printf("EP952 Code Version : %d.%d\r\n", (int)VERSION_MAJOR, (int)VERSION_MINOR );
	
	// Save the Logical Hardware Assignment
	pEP952C_Registers = pEP952C_RegMap;

	// Hardware Reset  
	EP_EP952_Reset();

	// Initial IIC address	   
	EP952_IIC_Initial();

	// Software power down
	HDMI_Tx_Power_Down_952();
	
	// Enable Audio Mute and Video Mute
	HDMI_Tx_Mute_Enable_952();	

	// Reset Variables
	is_Cap_HDMI = 0;
	is_Cap_YCC444 = is_Cap_YCC422 = 0;
	is_Forced_Output_RGB = 0;
	is_Connected = 1;
	TX_State = TXS_Search_EDID;
	HP_ChangeCount = 0;
	PowerUpCount = 0;
	is_ReceiverSense = 0;

	// Reset all EP952 parameters
	pEP952C_Registers->System_Status = 0;
	pEP952C_Registers->EDID_ASFreq = 0;
	pEP952C_Registers->EDID_AChannel = 0;
	pEP952C_Registers->Video_change = 0;
	pEP952C_Registers->Audio_change = 0; 
	pEP952C_Registers->System_Configuration = 0;
	pEP952C_Registers->Video_Input_Format[1]= 0;
	pEP952C_Registers->Video_Output_Format = 0;		// Auto select output 
//	pEP952C_Registers->Video_Output_Format = 0x03;	// Forced set RGB out

#if Enable_HDCP

	// Initial HDCP Info
	pEP952C_Registers->HDCP_Status = 0;			
	pEP952C_Registers->HDCP_State = 0;
	memset(pEP952C_Registers->HDCP_AKSV, 0x00, sizeof(pEP952C_Registers->HDCP_AKSV));
	memset(pEP952C_Registers->HDCP_BKSV, 0x00, sizeof(pEP952C_Registers->HDCP_BKSV));
	memset(pEP952C_Registers->HDCP_BCAPS3, 0x00, sizeof(pEP952C_Registers->HDCP_BCAPS3));
	memset(pEP952C_Registers->HDCP_KSV_FIFO, 0x00, sizeof(pEP952C_Registers->HDCP_KSV_FIFO));
	memset(pEP952C_Registers->HDCP_SHA, 0x00, sizeof(pEP952C_Registers->HDCP_SHA));
	memset(pEP952C_Registers->HDCP_M0, 0x00, sizeof(pEP952C_Registers->HDCP_M0));

	// Set Revocation List address
	HDCP_Extract_BKSV_BCAPS3(pEP952C_Registers->HDCP_BKSV);
	HDCP_Extract_FIFO((unsigned char*)pEP952C_Registers->HDCP_KSV_FIFO, sizeof(pEP952C_Registers->HDCP_KSV_FIFO));
	HDCP_Stop();

#else

	// Disable HDCP
	HDMI_Tx_HDCP_Disable_952();

#endif 

	// HDCP KEY reset
	EP952_HDCP_Reset();

	// Info Frame Reset
	EP952_Info_Reset();
	
}

void EP952_HDCP_Reset(void)
{
#if Enable_HDCP
	int i;

	//////////////////////////////////////////////////////////////////
	// Read HDCP Key for EEPROM
	if(HDMI_Tx_read_AKSV(pEP952C_Registers->HDCP_AKSV)) {
		pEP952C_Registers->System_Status &= ~EP_TX_System_Status__KEY_FAIL;
		DBG_printf("HDCP Check AKSV PASS\r\n");
		return;
	}
		
	status = HDMI_Tx_Get_Key((unsigned char *)HDCP_Key); // read HDCP Key from EEPROM

	DBG_printf("Read HDCP Key = 0x%02X\r\n",(int)status);
	pEP952C_Registers->System_Status &= ~EP_TX_System_Status__KEY_FAIL;

	// Check HDCP key and up load the key
	if(status) {
		// Do not upload the default Key!
		pEP952C_Registers->System_Status |= EP_TX_System_Status__KEY_FAIL;
		pEP952C_Registers->System_Configuration |= EP_TX_System_Configuration__HDCP_DIS;
		DBG_printf("No HDCP Key - Disable HDCP function\r\n");
	}
	else {
		// Check HDCP key and up load the key
		ChkSum = 0;
		for(i=0; i<328; ++i) {
			ChkSum += *((unsigned char *)HDCP_Key+i);
		}	
		DBG_printf("HDCP Key Check Sum 0x%02X\r\n", (int)ChkSum );
		
		if(HDCP_Key[3][7] != 0x50 || HDCP_Key[12][7] != 0x01 || ChkSum != 0x00) {
			pEP952C_Registers->System_Status |= EP_TX_System_Status__KEY_FAIL;
			pEP952C_Registers->System_Configuration |= EP_TX_System_Configuration__HDCP_DIS;
			DBG_printf("HDCP Key Check failed! - Disable HDCP function\r\n");
		}
		else {
			// Upload the key 0-39
			for(i=0; i<40; ++i) {
				DDC_Data[0] = (unsigned char)i;
				status |= EP952_Reg_Write(EP952_Key_Add, DDC_Data, 1);
				memcpy(DDC_Data,&HDCP_Key[i][0],7);
				status |= EP952_Reg_Write(EP952_Key_Data, DDC_Data, 7);
			}
			// Read and check	
			for(i=0; i<40; ++i) {
				DDC_Data[0] = (unsigned char)i;
				status |= EP952_Reg_Write(EP952_Key_Add, DDC_Data, 1);
				status |= EP952_Reg_Read(EP952_Key_Data, DDC_Data, 7);
				if((memcmp(DDC_Data,&HDCP_Key[i][0],7) != 0) || status) {
					// Test failed
					pEP952C_Registers->System_Status |= EP_TX_System_Status__KEY_FAIL;
					pEP952C_Registers->System_Configuration |= EP_TX_System_Configuration__HDCP_DIS;
					DBG_printf("HDCP Key Check failed! - Disable HDCP function\r\n");
					break;
				}
			}
			// Upload final KSV 40
			DDC_Data[0] = 40;
			status |= EP952_Reg_Write(EP952_Key_Add, DDC_Data, 1);
			memcpy(DDC_Data,&HDCP_Key[40][0],7);
			status |= EP952_Reg_Write(EP952_Key_Data, DDC_Data, 7);
			// Read back and check
	    	if(!HDMI_Tx_read_AKSV(pEP952C_Registers->HDCP_AKSV)) {
				// Test failed
				pEP952C_Registers->System_Status |= EP_TX_System_Status__KEY_FAIL;
				pEP952C_Registers->System_Configuration |= EP_TX_System_Configuration__HDCP_DIS;
				DBG_printf("HDCP Check KSV failed! - Disable HDCP function\r\n");
			}	
		}
	}

#else

	pEP952C_Registers->System_Status |= EP_TX_System_Status__KEY_FAIL;
	pEP952C_Registers->System_Configuration |= EP_TX_System_Configuration__HDCP_DIS;
	//DBG_printf("User define - Disable HDCP function\r\n");

#endif 

}

void EP952Controller_Timer(void)
{

#if Enable_HDCP
	if(TX_State == TXS_HDCP) HDCP_Timer();
#endif

}

void EP952Controller_Task(void)
{
	// Polling check Hot-Plug status 		
	ConnectionState = HDMI_Tx_HTPLG_952();

	is_Hot_Plug = (ConnectionState == 1)? 1:0;
	is_ReceiverSense = HDMI_Tx_RSEN_952(); 
		
	if(is_Connected != ((ConnectionState)?1:0) ) 
	{
		if(HP_ChangeCount++ >= 50)  // Hotplug continuous low 500ms	(10ms x 5)
		{	
			HP_ChangeCount = 0;

			is_Connected = ((ConnectionState)?1:0);
				
			if(!is_Connected){

				DBG_printf("#####################\r\n");
				DBG_printf("# HDMI - Disconnect #\r\n");
				DBG_printf("#####################\r\n");
					
				// power down EP952 Tx
				HDMI_Tx_Power_Down_952();
				
				DBG_printf("\r\nState Transist: Power Down -> [TXS_Search_EDID]\r\n");							
				TX_State = TXS_Search_EDID;						
			}
		}
	}
	else {
		HP_ChangeCount = 0;
	}

	if(EP952_Debug){

		EP952_Debug = 0;
					
		EP_HDMI_DumpMessage_952();	// dump EP952 register value for debug 	
	}

							
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Update EP952 Registers according to the System Process
	//
	switch(TX_State) 
	{
		case TXS_Search_EDID:

			if(is_Hot_Plug && is_ReceiverSense) {
					
				unsigned char EDID_DDC_Status;

				// clear EDID buffer
				memset(pEP952C_Registers->Readed_EDID, 0xFF, 256);
					
				// Read EDID
				DBG_printf("\r\n[Read EDID] :\r\n");				
				EDID_DDC_Status = Downstream_Rx_read_EDID_952(pEP952C_Registers->Readed_EDID);
					
				if(EDID_DDC_Status) {
					DBG_printf("WARNING: EDID read failed 0x%02X\r\n", (int)EDID_DDC_Status);
					break;
				}
 
				// check EDID
				is_Cap_HDMI = EDID_GetHDMICap_952(pEP952C_Registers->Readed_EDID);
					
				if(is_Cap_HDMI) {			
					DBG_printf("EDID : Support HDMI");

					// Default Capability
					is_Cap_YCC444 =	0;
					is_Cap_YCC422 = 0;
					pEP952C_Registers->EDID_ASFreq = 0x07;
					pEP952C_Registers->EDID_AChannel = 1;

					if(!EDID_DDC_Status) {

						if(pEP952C_Registers->Readed_EDID[131] & 0x20) {	// Support YCC444
							is_Cap_YCC444 = 1;
							DBG_printf(" YCC444");
						}
						if(pEP952C_Registers->Readed_EDID[131] & 0x10) {	// Support YCC422
							is_Cap_YCC422 = 1;
							DBG_printf(" YCC422");
						}
						DBG_printf("\r\n");
						
						// Audio
						pEP952C_Registers->EDID_ASFreq = EDID_GetPCMFreqCap_952(pEP952C_Registers->Readed_EDID);
						//DBG_printf(("EDID : ASFreq = 0x%02X\r\n",(int)pEP952C_Registers->EDID_ASFreq));

						// Optional 
						//pEP952C_Registers->EDID_VideoDataAddr = 0x00;
						//pEP952C_Registers->EDID_AudioDataAddr = 0x00;
						//pEP952C_Registers->EDID_SpeakerDataAddr = 0x00;
						//pEP952C_Registers->EDID_VendorDataAddr = 0x00;
							
						//pEP952C_Registers->EDID_VideoDataAddr = EDID_GetDataBlockAddr(pEP952C_Registers->Readed_EDID, 0x40);
						//pEP952C_Registers->EDID_AudioDataAddr = EDID_GetDataBlockAddr(pEP952C_Registers->Readed_EDID, 0x20);
						//pEP952C_Registers->EDID_SpeakerDataAddr = EDID_GetDataBlockAddr(pEP952C_Registers->Readed_EDID, 0x80);
						//pEP952C_Registers->EDID_VendorDataAddr = EDID_GetDataBlockAddr(pEP952C_Registers->Readed_EDID, 0x60);
					}
				}
				else {
					DBG_printf("EDID : Support DVI(RGB) only\r\n");
					is_Cap_YCC444 =	0;
					is_Cap_YCC422 = 0;
					pEP952C_Registers->EDID_ASFreq = 0;
					pEP952C_Registers->EDID_AChannel = 0;
				}

				is_Connected = 1; // HDMI is connected

				DBG_printf("\r\nState Transit: Read EDID -> [TXS_Wait_Upstream]\r\n");				
				TX_State = TXS_Wait_Upstream;
			}
			break;
			
		case TXS_Wait_Upstream:

			if(pEP952C_Registers->Video_Input_Format[0] != 0)
			{
				// power down Tx
				HDMI_Tx_Power_Down_952();
				
				// update EP952 register setting
				EP952_Audio_reg_set();
				EP952_Video_reg_set();
				
				EP952_Debug = 1;
					
				// Power Up Tx
				HDMI_Tx_Power_Up_952();
				
				DBG_printf("\r\nState Transist: Power Up -> [TXS_Stream]\r\n");							
				TX_State = TXS_Stream;
			}
			break;

		case TXS_Stream:

			if(pEP952C_Registers->Audio_change){	// Audio change

				DBG_printf("--- Audio source change ---\r\n");
				EP952_Audio_reg_set();
				pEP952C_Registers->Audio_change = 0;
				EP952_Debug = 1;	
			}

			if(pEP952C_Registers->Video_change){	// Video change
				
				TXS_RollBack_Stream_952();
				pEP952C_Registers->Video_change = 0;

				if(pEP952C_Registers->Video_Input_Format[0] != 0){
					DBG_printf("\r\nState Transit: Video Source Change -> [TXS_Wait_Upstream]\r\n");				
					TX_State = TXS_Wait_Upstream;
				}
				else{
					DBG_printf("\r\nState Transit: VIC = %d -> [TXS_Search_EDID]\r\n",(int)pEP952C_Registers->Video_Input_Format[0]);				
					TX_State = TXS_Search_EDID;
				}
			}

			if(!is_Connected) {						// HDMI	not connected

				DBG_printf("\r\nTXS_Stream: HDMI is not Connected\r\n");							
				TXS_RollBack_Stream_952();
				TX_State = TXS_Search_EDID;
			}

#if Enable_HDCP
			else if(!(pEP952C_Registers->System_Configuration & EP_TX_System_Configuration__HDCP_DIS) && is_ReceiverSense) {
			
				if(!is_HDCP_Info_BKSV_Rdy) {
					// Get HDCP Info
			    	if(!Downstream_Rx_read_BKSV(pEP952C_Registers->HDCP_BKSV)) {
						pEP952C_Registers->HDCP_Status = HDCP_ERROR_AKSV;
					}
					pEP952C_Registers->HDCP_BCAPS3[0] = Downstream_Rx_BCAPS();
					is_HDCP_Info_BKSV_Rdy = 1;
				}
				
				// Enable mute
				DBG_printf("Mute first for HDCP Authentication start\r\n");
				HDMI_Tx_Mute_Enable();
				
				DBG_printf("\r\nState Transist: Start HDCP -> [TXS_HDCP]\r\n");
				TX_State = TXS_HDCP;
			}
#endif			
			break;

#if Enable_HDCP

		case TXS_HDCP:
		
			if(pEP952C_Registers->Audio_change){	// Audio change
			
				DBG_printf("--- Audio source change ---\r\n");
				EP952_Audio_reg_set();
				pEP952C_Registers->Audio_change = 0;	
				EP952_Debug = 1;
			}

			if(pEP952C_Registers->Video_change){	// Video change
				
				TXS_RollBack_HDCP();
				TXS_RollBack_Stream();
				pEP952C_Registers->Video_change = 0;
				
				if(pEP952C_Registers->Video_Input_Format[0] != 0){
					DBG_printf("\r\nState Transit: Video Source Change -> [TXS_Wait_Upstream]\r\n");				
					TX_State = TXS_Wait_Upstream;
				}
				else{
					DBG_printf("\r\nState Transit: VIC = %d -> [TXS_Search_EDID]\r\n",(int)pEP952C_Registers->Video_Input_Format[0]);				
					TX_State = TXS_Search_EDID;
				}
			}

			if(!is_Connected) {						// HDMI	not connected

				TXS_RollBack_HDCP();
				TXS_RollBack_Stream();
				TX_State = TXS_Search_EDID;
			}
			else {
				pEP952C_Registers->HDCP_State = HDCP_Authentication_Task(is_ReceiverSense && is_Connected/*is_Hot_Plug*/);
				pEP952C_Registers->HDCP_Status = HDCP_Get_Status();
				if(pEP952C_Registers->HDCP_Status != 0)
				{
					DBG_printf("ERROR : HDCP_Status = 0x%02X\r\n",(int)pEP952C_Registers->HDCP_Status);
					
					TXS_RollBack_HDCP();
					TXS_RollBack_Stream();
					TX_State = TXS_Search_EDID;
				}
			}
			break;
#endif

	}
}

void EP952_Video_reg_set(void)
{
	DBG_printf("\r\n ========== EP952 Video Parameter setting ==========\r\n");

	// Mute Control
	HDMI_Tx_Mute_Enable_952();

	if(pEP952C_Registers->Video_Input_Format[0] != 0)
	{
		// HDMI Mode
		if(!is_Cap_HDMI ) {
			HDMI_Tx_DVI_952();	// Set to DVI output mode (The Info Frame and Audio Packets would not be send)
			is_Forced_Output_RGB = 1;
		}
		else {
			HDMI_Tx_HDMI_952();	// Set to HDMI output mode
		}
	
		///////////////////////////////////////////////////////////////////////
		// Update Video Params
		//
		DBG_printf("pEP952 Video_Interface[0] = 0x%02X\r\n",(int)pEP952C_Registers->Video_Interface[0]);
		DBG_printf("pEP952 Video_Interface[1] = 0x%02X\r\n",(int)pEP952C_Registers->Video_Interface[1]);
		DBG_printf("pEP952 Video_Output_Format = 0x%02X \r\n",(int)pEP952C_Registers->Video_Output_Format );
		DBG_printf("pEP952 Video_Input_Format[0] = 0x%02X \r\n",(int)pEP952C_Registers->Video_Input_Format[0] );
				
		// Video Interface
		Video_Params.Interface = pEP952C_Registers->Video_Interface[0];
			
		// Video Timing
		if(pEP952C_Registers->Video_Input_Format[0] < 35) {
			Video_Params.VideoSettingIndex = pEP952C_Registers->Video_Input_Format[0];
		}
		else{
			Video_Params.VideoSettingIndex = 0;
			DBG_printf("ERROR: pEP952 Video_Input_Format[0] = 0x%02X OVER CEA-861-B SPEC\r\n",(int)pEP952C_Registers->Video_Input_Format[0]);
		}
			
		// Select Sync Mode
		Video_Params.SyncMode = (pEP952C_Registers->Video_Interface[1] & EP_TX_Video_Interface_Setting_1__SYNC) >> 2;
			
		// Select Color Space
		switch(Video_Params.VideoSettingIndex) {
			case  4: case  5: case 16: case 19: case 20: case 31: case 32: 
			case 33: case 34: 													// HD Timing
				Video_Params.ColorSpace = COLORSPACE_709;
				break;
			
			default:
				if(Video_Params.VideoSettingIndex) { 							// SD Timing
					Video_Params.ColorSpace = COLORSPACE_601;
				}
				else {															// IT Timing
					Video_Params.ColorSpace = COLORSPACE_709;
				}
				break;
		}
	
		// Forced Output RGB Format
		if(pEP952C_Registers->Video_Output_Format == 0x03) {
			is_Forced_Output_RGB = 1;
		}
			
		// Set In and Output Color Format	
		switch(pEP952C_Registers->Video_Interface[1] & EP_TX_Video_Interface_Setting_1__VIN_FMT) {
			
			default:
			case EP_TX_Video_Interface_Setting_1__VIN_FMT__RGB:	 	// input is RGB
				Video_Params.FormatIn = COLORFORMAT_RGB;
				Video_Params.FormatOut = COLORFORMAT_RGB;
				break;
			
			case EP_TX_Video_Interface_Setting_1__VIN_FMT__YCC444: 	// input is YCC444
				Video_Params.FormatIn = COLORFORMAT_YCC444;
				if(!is_Forced_Output_RGB && is_Cap_YCC444) {
					Video_Params.FormatOut = COLORFORMAT_YCC444;
				}
				else {
					Video_Params.FormatOut = COLORFORMAT_RGB;
				}
				break;
		
			case EP_TX_Video_Interface_Setting_1__VIN_FMT__YCC422: 	// inut is YCC422
				Video_Params.FormatIn = COLORFORMAT_YCC422;
				if(!is_Forced_Output_RGB && is_Cap_YCC422) {
					Video_Params.FormatOut = COLORFORMAT_YCC422;
				}
				else {
					Video_Params.FormatOut = COLORFORMAT_RGB;
				}
				break;
		}
		// AFAR
		Video_Params.AFARate = 0;
	
		// SCAN 		
		Video_Params.SCAN = 0;
	
		// Update EP952 Video Registers 
		HDMI_Tx_Video_Config_952(&Video_Params);
	
		// mute control
		HDMI_Tx_Mute_Disable_952();
	}
	else
	{
		DBG_printf("### [Warning]: Tx Mute Enable (VIC = %d) ###\r\n"
			,(int)pEP952C_Registers->Video_Input_Format[0]);
	}

	// clear flag
	pEP952C_Registers->Video_change = 0;

}

void EP952_Audio_reg_set(void)
{
	DBG_printf("\r\n ========== EP952 Audio Parameter setting ==========\r\n");

	// Mute Control
	HDMI_Tx_AMute_Enable_952();

	if( (pEP952C_Registers->Audio_Input_Format != 0) && (pEP952C_Registers->Video_Input_Format[0] != 0))
	{
		///////////////////////////////////////////////////////////////////////
		// Update Audio Params
		//
		//DBG_printf("pEP952 Audio_Interface = 0x%02X\r\n",(int)pEP952C_Registers->Audio_Interface);
		//DBG_printf("pEP952 Audio_Input_Format = 0x%02X\r\n",(int)pEP952C_Registers->Audio_Input_Format);
	
		Audio_Params.Interface = pEP952C_Registers->Audio_Interface & 0x0F; // IIS, WS_M, WS_POL, SCK_POL
		Audio_Params.VideoSettingIndex = pEP952C_Registers->Video_Input_Format[0]; //Video_Params.VideoSettingIndex;
	
		// Audio Channel Number
		Audio_Params.ChannelNumber = 1; // 2 ch
	
		// Update VFS
		if(Audio_Params.VideoSettingIndex < EP952_VDO_Settings_IT_Start) {
			Audio_Params.VFS = 1;  
		}
		else {
			Audio_Params.VFS = 0;
		}
	
		Audio_Params.NoCopyRight = 0;
	
		// Write Frequency info (Use ADO_FREQ or Auto)
		switch( pEP952C_Registers->Audio_Input_Format & EP_TX_Audio_Input_Format__ADO_FREQ ) {
			
			case EP_TX_Audio_Input_Format__ADO_FREQ__32000Hz:
				Audio_Params.InputFrequency = ADSFREQ_32000Hz;
				Audio_Params.ADSRate = 0; // Disable Down Sample
				break;
			
			default:
			case EP_TX_Audio_Input_Format__ADO_FREQ__44100Hz:
				Audio_Params.InputFrequency = ADSFREQ_44100Hz;
				Audio_Params.ADSRate = 0; // Disable Down Sample
				break;
			
			case EP_TX_Audio_Input_Format__ADO_FREQ__48000Hz:
				Audio_Params.InputFrequency = ADSFREQ_48000Hz;
				Audio_Params.ADSRate = 0; // Disable Down Sample
				break;
			
			case EP_TX_Audio_Input_Format__ADO_FREQ__88200Hz:
				Audio_Params.InputFrequency = ADSFREQ_88200Hz;
				if(pEP952C_Registers->EDID_ASFreq & 0x08) { // 88.2kHz
					Audio_Params.ADSRate = 0; // Disable Down Sample
				}
				else {
					Audio_Params.ADSRate = 1; // Enable Down Sample 1/2
				}
				break;
			
			case EP_TX_Audio_Input_Format__ADO_FREQ__96000Hz:
				Audio_Params.InputFrequency = ADSFREQ_96000Hz;
				if(pEP952C_Registers->EDID_ASFreq & 0x10) { // 96kHz
					Audio_Params.ADSRate = 0; // Disable Down Sample
				}
				else {
					if(pEP952C_Registers->EDID_ASFreq & 0x04) { // 48kHz
						Audio_Params.ADSRate = 1; // Enable Down Sample 1/2
					}
					else {
						Audio_Params.ADSRate = 2; // Enable Down Sample 1/3
					}
				}
				break;
			
			case EP_TX_Audio_Input_Format__ADO_FREQ__176400Hz:
				Audio_Params.InputFrequency = ADSFREQ_176400Hz;
				if(pEP952C_Registers->EDID_ASFreq & 0x20) { // 176kHz
					Audio_Params.ADSRate = 0; // Disable Down Sample
				}
				else {
					if(pEP952C_Registers->EDID_ASFreq & 0x08) { // 88.2kHz
						Audio_Params.ADSRate = 1; // Enable Down Sample 1/2
					}
					else {
						Audio_Params.ADSRate = 3; // Enable Down Sample 1/4
					}
				}
				break;
		
			case EP_TX_Audio_Input_Format__ADO_FREQ__192000Hz:
				Audio_Params.InputFrequency = ADSFREQ_192000Hz;
				if(pEP952C_Registers->EDID_ASFreq & 0x40) { // 192kHz
					Audio_Params.ADSRate = 0; // Disable Down Sample
				}
				else {
					if(pEP952C_Registers->EDID_ASFreq & 0x10) { // 96kHz
						Audio_Params.ADSRate = 1; // Enable Down Sample 1/2
				}
				else {
						Audio_Params.ADSRate = 3; // Enable Down Sample 1/4
					}
				}
				break;
		}
			
		// Update EP952 Audio Registers 
		HDMI_Tx_Audio_Config_952(&Audio_Params);
	
		// mute control
		HDMI_Tx_AMute_Disable_952();
	}
	else
	{
		DBG_printf("[Warning]: Audio Mute Enable (Audio Sample Frequency = %d, VIC = %d)\r\n"
			,(int)pEP952C_Registers->Audio_Input_Format
			,(int)pEP952C_Registers->Video_Input_Format[0]);
	}

	// clear flag
	pEP952C_Registers->Audio_change = 0;
}

void TXS_RollBack_Stream_952(void)
{
	DBG_printf("\r\nState Rollback: Power Down -> [TXS_Search_EDID]\r\n");

	// Power Down
	HDMI_Tx_Power_Down_952();

#if Enable_HDCP
	// Reset HDCP Info
	memset(pEP952C_Registers->HDCP_BKSV, 0x00, sizeof(pEP952C_Registers->HDCP_BKSV));
	is_HDCP_Info_BKSV_Rdy = 0;
#endif

}

#if Enable_HDCP
void TXS_RollBack_HDCP(void)
{
	DBG_printf("\r\nState Rollback: Stop HDCP -> [TXS_Stream]\r\n");
	
	HDCP_Stop();
	pEP952C_Registers->HDCP_Status = 0;
	pEP952C_Registers->HDCP_State = 0;

}
#endif

//----------------------------------------------------------------------------------------------------------------------
void EP952_EXTINT_init(unsigned char INT_Enable, unsigned char INT_OD, unsigned char INT_POL)
{
	if(INT_Enable)
	{
		// monitor sense is set to HTPLG detect
		EP952_Reg_Set_Bit(EP952_General_Control_1, EP952_General_Control_1__TSEL_HTP);

		if(INT_OD == INT_OPEN_DRAIN)
		{
			// INT pin is open drain output (need external pull high)
			EP952_Reg_Set_Bit(EP952_General_Control_1, EP952_General_Control_1__INT_OD);
		}
		else
		{
			// INT pin is push-pull output 
			EP952_Reg_Clear_Bit(EP952_General_Control_1, EP952_General_Control_1__INT_OD);
		}

		if(INT_POL == INT_High)
		{
			// INT pin is high when interrupt
			EP952_Reg_Set_Bit(EP952_General_Control_1, EP952_General_Control_1__INT_POL);
		}
		else
		{
			// INT pin is low when interrupt
			EP952_Reg_Clear_Bit(EP952_General_Control_1, EP952_General_Control_1__INT_POL);
		}

		// monitor sense is enable
		EP952_Reg_Set_Bit(EP952_General_Control_2, EP952_General_Control_2__MIFE);
	}
	else
	{
		// monitor sense is Disable
		EP952_Reg_Clear_Bit(EP952_General_Control_2, EP952_General_Control_2__MIFE);
	}

}


void EP_HDMI_DumpMessage_952(void)
{
	unsigned char temp_R = 0xFF, reg_addr = 0;

	DBG_printf("[EP952 Register value]\r\n");
	DBG_printf("    -0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F\r\n");
	DBG_printf("    -----------------------------------------------");
	for(reg_addr=0; reg_addr<=0x88; reg_addr++)
	{
		EP952_Reg_Read(reg_addr, &temp_R, 1);
	
		if(reg_addr%16 == 0)
		{
			DBG_printf("\r\n%02X| ",(int)((reg_addr/16)<<4));
		}
		DBG_printf("%02X ",(int)temp_R);
	
	}
	DBG_printf("\r\n");
	DBG_printf("    -----------------------------------------------\r\n");
}


