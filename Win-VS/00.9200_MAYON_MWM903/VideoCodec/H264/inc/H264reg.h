/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	mpeg4reg.h

Abstract:

   	The registers of H264 encoder/decoder.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/08/30	Lsk	Create	

*/

#ifndef __H264_REG_H__
#define __H264_REG_H__


//#define H264ADDR_ENCODE_CTL0    *((volatile unsigned *)(H264CtrlBase + 0x0018))
#define H264_ENC_CLR_INT        0x00000021
#define H264_ENC_EN_IP          0x00000002
#define H264_ENC_SPS_PPS        0x00000004

//#define H264ENC_ADDR_SEL_UV         *((volatile unsigned *)(H264Enc_CtrlBase + 0x0140))
#define H264_ENC_UV_DIVIDED     0x00000000
#define H264_ENC_UV_INTERLEAVE  0x00000001
#define H264_ENC_DOWNSAMPLE     0x00000002

#define H264_DEC_DOWNSAMPLE_DIS   0x00000000
#define H264_DEC_DOWNSAMPLE_1     0x00000010
#define H264_DEC_DOWNSAMPLE_2     0x00000020
#define H264_DEC_DOWNSAMPLE_4     0x00000030

#define H264_ENC_SLICE_SIZE_EQUAL_1     0X00010000
#define H264_ENC_SLICE_SIZE_EQUAL_2     0X00020000
#define H264_ENC_SLICE_SIZE_EQUAL_4     0X00030000
#define H264_ENC_SLICE_SIZE_EQUAL_8     0X00040000

#define H264_ENC_I_MB_REFRESH_MODE_EN     0X01000000
#define H264_ENC_I_MB_REFRESH_MODE_DIS    0X00000000

#endif
