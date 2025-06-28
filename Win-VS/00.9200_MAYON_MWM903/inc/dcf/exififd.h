/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	exififd.h

Abstract:

   	The declarations of EXIF.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __EXIF_IFD_H__
#define __EXIF_IFD_H__

/* Type definition */
typedef __packed struct _EXIF_IFD
{
	u16	tag;
	u16	type;
	u32	count;
	u32	valueOffset;
} EXIF_IFD;

/*---- TIFF header ----*/
typedef __packed struct _EXIF_TIFF_HEADER
{
	u16	byteOrder;
	u16	versionNumber;
	u32	offsetToIfd0;
} EXIF_TIFF_HEADER;

/* byteOrder */
#define EXIF_TIFF_LITTLE_ENDIAN				0x4949		/* "II" */
#define EXIF_TIFF_BIG_ENDDIAN				0x4d4d		/* "MM" */

/* versionNumber */
#define EXIF_TIFF_VERSION_NUMBER			0x002a		/* 42 */

/* offsetToIfd0 */
#define EXIF_TIFF_OFFS_TO_IFD0				0x00000008

/*---- IFD0 ----*/
#define EXIF_IFD0_NUM_INTEROP				0x000b
typedef __packed struct _EXIF_IFD0
{
	u16		numInterop;
	EXIF_IFD	imageDescription;
	EXIF_IFD	make;
	EXIF_IFD	model;
	EXIF_IFD	orientation;
	EXIF_IFD	xResolution;
	EXIF_IFD	yResolution;
	EXIF_IFD	resolutionUnit;
	EXIF_IFD	dateTime;
	EXIF_IFD	yCbCrPositioning;
	EXIF_IFD	copyRight;
	EXIF_IFD	exifIfdPointer;
	u32		nextIfdOffset;
} EXIF_IFD0;

#define EXIF_IFD0_IMAGE_DESCRIPTION_VALUE_COUNT		0x0e
#define EXIF_IFD0_MAKE_VALUE_COUNT			0x0a
#define EXIF_IFD0_MODEL_VALUE_COUNT			0x08
#define EXIF_IFD0_DATE_TIME_VALUE_COUNT			0x14
#define EXIF_IFD0_COPYRIGHT_VALUE_COUNT			0x16

#define EXIF_IFD0_IMAGE_DESCRIPTION_VALUE_OFFSET	(sizeof(EXIF_TIFF_HEADER) + sizeof(EXIF_IFD0))
#define EXIF_IFD0_MAKE_VALUE_OFFSET			(EXIF_IFD0_IMAGE_DESCRIPTION_VALUE_OFFSET + EXIF_IFD0_IMAGE_DESCRIPTION_VALUE_COUNT)
#define EXIF_IFD0_MODEL_VALUE_OFFSET			(EXIF_IFD0_MAKE_VALUE_OFFSET + EXIF_IFD0_MAKE_VALUE_COUNT)
#define EXIF_IFD0_X_RESOLUTION_VALUE_OFFSET		(EXIF_IFD0_MODEL_VALUE_OFFSET + EXIF_IFD0_MODEL_VALUE_COUNT)
#define EXIF_IFD0_Y_RESOLUTION_VALUE_OFFSET		(EXIF_IFD0_X_RESOLUTION_VALUE_OFFSET + 8)
#define EXIF_IFD0_DATE_TIME_VALUE_OFFSET		(EXIF_IFD0_Y_RESOLUTION_VALUE_OFFSET + 8)
#define EXIF_IFD0_COPYRIGHT_VALUE_OFFSET		(EXIF_IFD0_DATE_TIME_VALUE_OFFSET + EXIF_IFD0_DATE_TIME_VALUE_COUNT)

typedef __packed struct _EXIF_IFD0_VALUE
{
	u8		imageDescriptionValue[EXIF_IFD0_IMAGE_DESCRIPTION_VALUE_COUNT];
	u8		makeValue[EXIF_IFD0_MAKE_VALUE_COUNT];
	u8		modelValue[EXIF_IFD0_MODEL_VALUE_COUNT];
	u32		xResolutionValue[2];
	u32		yResolutionValue[2];
	u8		dateTimeValue[EXIF_IFD0_DATE_TIME_VALUE_COUNT];	
	u8		copyrightValue[EXIF_IFD0_COPYRIGHT_VALUE_COUNT];
} EXIF_IFD0_VALUE; 	

/* Exif IFD */
#define EXIF_IFD0E_NUM_INTEROP				0x0019
typedef __packed struct _EXIF_IFD0E
{
	u16		numInterop;
	EXIF_IFD	exposureTime;
	EXIF_IFD	fNumber;
	EXIF_IFD	exifVersion;
	EXIF_IFD	dateTimeOriginal;
	EXIF_IFD	dateTimeDigitized;
	EXIF_IFD	componentsConfiguration;
	EXIF_IFD	compressedBitsPerPixel;
	EXIF_IFD	shutterSpeedValue;
	EXIF_IFD	apertureValue;
	EXIF_IFD	brightnessValue;
	EXIF_IFD	exposureBiasValue;
	EXIF_IFD	maxApertureValue;
	EXIF_IFD	subjectDistance;
	EXIF_IFD	meteringMode;
	EXIF_IFD	lightSource;
	EXIF_IFD	flash;
	EXIF_IFD	focalLength;
	EXIF_IFD	userComments;
	EXIF_IFD	subsecTime;
	EXIF_IFD	subsecTimeOriginal;
	EXIF_IFD	subsecTimeDigitized;
	EXIF_IFD	flashpixVersion;
	EXIF_IFD	colorSpace;
	EXIF_IFD	pixelXDimension;
	EXIF_IFD	pixelYDimension;	
	u32		nextIfdOffset;
} EXIF_IFD0E;

#define EXIF_IFD0E_DATE_TIME_ORIGINAL_VALUE_COUNT	0x14
#define EXIF_IFD0E_DATE_TIME_DIGITIZED_VALUE_COUNT	0x14
#define EXIF_IFD0E_USER_COMMENT_VALUE_COUNT		0x27

#define EXIF_IFD0E_EXPOSURE_TIME_VALUE_OFFSET		(sizeof(EXIF_TIFF_HEADER) + sizeof(EXIF_IFD0) + sizeof(EXIF_IFD0_VALUE) + sizeof(EXIF_IFD0E))
#define EXIF_IFD0E_F_NUMBER_VALUE_OFFSET		(EXIF_IFD0E_EXPOSURE_TIME_VALUE_OFFSET + 8)
#define EXIF_IFD0E_DATE_TIME_ORIGINAL_VALUE_OFFSET	(EXIF_IFD0E_F_NUMBER_VALUE_OFFSET + 8)
#define EXIF_IFD0E_DATE_TIME_DIGITIZED_VALUE_OFFSET	(EXIF_IFD0E_DATE_TIME_ORIGINAL_VALUE_OFFSET + EXIF_IFD0E_DATE_TIME_ORIGINAL_VALUE_COUNT)
#define EXIF_IFD0E_COMPRESSED_BITS_PER_PIXEL_VALUE_OFFSET (EXIF_IFD0E_DATE_TIME_DIGITIZED_VALUE_OFFSET + EXIF_IFD0E_DATE_TIME_DIGITIZED_VALUE_COUNT)
#define EXIF_IFD0E_SHUTTER_SPEED_VALUE_VALUE_OFFSET	(EXIF_IFD0E_COMPRESSED_BITS_PER_PIXEL_VALUE_OFFSET + 8)
#define EXIF_IFD0E_APERTURE_VALUE_VALUE_OFFSET		(EXIF_IFD0E_SHUTTER_SPEED_VALUE_VALUE_OFFSET + 8)
#define EXIF_IFD0E_BRIGHTNESS_VALUE_VALUE_OFFSET	(EXIF_IFD0E_APERTURE_VALUE_VALUE_OFFSET + 8)
#define EXIF_IFD0E_EXPOSURE_BIAS_VALUE_VALUE_OFFSET	(EXIF_IFD0E_BRIGHTNESS_VALUE_VALUE_OFFSET + 8)
#define EXIF_IFD0E_MAX_APERTURE_VALUE_VALUE_OFFSET 	(EXIF_IFD0E_EXPOSURE_BIAS_VALUE_VALUE_OFFSET + 8)
#define EXIF_IFD0E_SUBJECT_DISTANCE_VALUE_OFFSET	(EXIF_IFD0E_MAX_APERTURE_VALUE_VALUE_OFFSET + 8)
#define EXIF_IFD0E_FOCAL_LENGTH_VALUE_OFFSET		(EXIF_IFD0E_SUBJECT_DISTANCE_VALUE_OFFSET + 8)
#define EXIF_IFD0E_USER_COMMENT_VALUE_OFFSET		(EXIF_IFD0E_FOCAL_LENGTH_VALUE_OFFSET + 8)

typedef __packed struct _EXIF_IFD0E_VALUE
{
	u32		exposureTimeValue[2];
	u32		fNumberValue[2];
	u8		dateTimeOriginalValue[EXIF_IFD0E_DATE_TIME_ORIGINAL_VALUE_COUNT];
	u8		dateTimeDigitizedValue[EXIF_IFD0E_DATE_TIME_DIGITIZED_VALUE_COUNT];
	u32		compressedBitsPerPixelValue[2];
	s32		shutterSpeedValueValue[2];
	u32		apertureValueValue[2];
	s32		brightnessValueValue[2];
	s32		exposureBiasValueValue[2];
	u32		maxApertureValueValue[2];
	u32		subjectDistanceValue[2];
	u32		focalLengthValue[2];
	u8		userCommentValue[EXIF_IFD0E_USER_COMMENT_VALUE_COUNT];
} EXIF_IFD0E_VALUE;

/*---- IFD1 ----*/
#define EXIF_IFD1_NUM_INTEROP				0x0007
typedef __packed struct _EXIF_IFD1
{
	u16		numInterop;
	EXIF_IFD	compression;
	EXIF_IFD	xResolution;
	EXIF_IFD	yResolution;
	EXIF_IFD	resolutionUnit;
	EXIF_IFD	jpegInterchangeFormat;
	EXIF_IFD	jpegInterchangeFormatLength;
	EXIF_IFD	yCbCrPositioning;
	u32		nextIfdOffset;
} EXIF_IFD1;

#define EXIF_IFD1_X_RESOLUTION_VALUE_OFFSET		(sizeof(EXIF_TIFF_HEADER) + sizeof(EXIF_IFD0) + sizeof(EXIF_IFD0_VALUE) + sizeof(EXIF_IFD0E) + sizeof(EXIF_IFD0E_VALUE) + sizeof(EXIF_IFD1))
#define EXIF_IFD1_Y_RESOLUTION_VALUE_OFFSET		(EXIF_IFD1_X_RESOLUTION_VALUE_OFFSET + 8)

typedef __packed struct _EXIF_IFD1_VALUE
{
	u32		xResolutionValue[2];
	u32		yResolutionValue[2];
} EXIF_IFD1_VALUE;

/*---- IFD offset ----*/

#define EXIF_IFD0_EXIF_IFD_POINTER			(sizeof(EXIF_TIFF_HEADER) + sizeof(EXIF_IFD0) + sizeof(EXIF_IFD0_VALUE))
#define EXIF_IFD0_NEXT_IFD_OFFSET			(EXIF_IFD0_EXIF_IFD_POINTER + sizeof(EXIF_IFD0E) + sizeof(EXIF_IFD0E_VALUE))
#define EXIF_IFD0E_NEXT_IFD_OFFSET			0x00000000
#define EXIF_IFD1_NEXT_IFD_OFFSET			0x00000000
#define EXIF_IFD1_JPEG_INTERCHANGE_FORMAT 		(EXIF_IFD0_NEXT_IFD_OFFSET + sizeof(EXIF_IFD1) + sizeof(EXIF_IFD1_VALUE))

#endif
