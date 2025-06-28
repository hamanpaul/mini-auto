/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	jpegapi.h

Abstract:

   	The application interface of the JPEG encoder/decoder.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __JPEG_API_H__
#define __JPEG_API_H__

/* Structure */

/* Constant */

#define JPEG_IMAGE_PRIMARY			0x00
#define JPEG_IMAGE_THUMBNAIL	    0x01
#define JPEG_IMAGE_APP3IMG          0x02



#define JPEG_COMPONENT_Y			0x00
#define JPEG_COMPONENT_Cb			0x01
#define JPEG_COMPONENT_Cr			0x02

#define JPEG_QUANTIZATION_PRECISION_8		0x00
#define JPEG_QUANTIZATION_PRECISION_16		0x01

#define JPEG_HUFFMAN_DC				0x00
#define JPEG_HUFFMAN_AC				0x10

#define JPEG_FORMAT_YCbCr_422           0x00000000
#define JPEG_FORMAT_YCbCr_420           0x00000010
#define JPEG_FORMAT_YCbCr_440           0x00000040
#define JPEG_FORMAT_YCbCr_444           0x00000050

/* Variable */

/* Function prototype */

extern s32 jpegSetImageResolution(u16, u16);
extern s32 jpegSetQuantizationQuality(u8, u8);
extern s32 jpegSetQuantizationTable(u8, u8, u8*,u8);/*BJ 0523 S*/
extern s32 jpegSetHuffmanTable(u8, u8, u8*, u8*,u8);/*BJ 0523 S*/
extern s32 jpegSetDataFormat(u8);
extern s32 jpegSetRestartInterval(u16);
extern s32 jpegSetCaptureDefault(void);
extern s32 jpegCapturePrimary(u8*,s16);
extern s32 jpegCaptureAPP3Image(u8*,u8*,s16,u32);
extern s32 jpegCapturePreviewImg(u8*,u8* ,s16,u16,u16);

extern s32 jpegCaptureThumbnail(u8*, u32*);
extern s32 jpegDecompression(u8*,u8*);/*BJ 0523 S*/
extern s32 jpegDecompressionYUV420(u8* pBuf,       // Bitstream address, must align 4
                                               u8* pResultY,   // Output image address, must align 32. Y data, if DataType == YUV420. YUV422 data, if DataType == YUV422.
                                               u8* pResultUV,  // Output image address, must align 32. UV data, if DataType == YUV420. Invalid when DataType == YUV422.
                                               u32 DataType,   // for A1018, image data type, 0:YUV422, 1:YUV420.
                                               u32 Stride);    // for A1018, stride in bytes. Only valid when DataType == YUV420.
extern s32 jpegDecompressionSlice(u8* pBuf , u8* pResult);/*Peter 071022 S*/
extern void isuGetImgOutResolution(u16*, u16*);
extern s32 WaitJpegEncComplete(void);
extern u32 GetJpegImagePixelCount(void);

extern s32 jpegInit(void);
extern void jpegIntHandler(void);
extern void jpegTest(void);
extern void sysJpegRst(void);

#if (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
extern OS_EVENT* VideoTrgSemEvt;
extern OS_EVENT* VideoCmpSemEvt;
extern OS_EVENT* VideoCpleSemEvt; /*BJ 0530 S*/
extern u32 MJPEG_Pend;
extern u32 MJPG_Task_Go;  // 0: never run, 1: ever run
extern s32 mjpgResumeTask(void);
extern s32 mjpgSuspendTask(void);

extern s32 mjpgInit(void);
extern void mjpgIntHandler(void);
extern void mjpg4Test(void);

#endif

#endif
