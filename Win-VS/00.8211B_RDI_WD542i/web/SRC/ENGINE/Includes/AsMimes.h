/*
 *	File:		AsMimes.h
 *
 *	Contains:	MIME types for RomPager product family
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2003 by Allegro Software Development Corporation
 *  All rights reserved.
 *
 *  This module contains confidential, unpublished, proprietary 
 *  source code of Allegro Software Development Corporation.
 *
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 *
 *  License is granted for specific uses only under separate 
 *  written license by Allegro Software Development Corporation.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 * * * * Release 4.20  * * *
 *		10/04/02	amp		add eRpDataTypeCertificate
 * * * * Release 4.12  * * *
 * * * * Release 4.00  * * *
 *		03/06/01	bva		add eRpDataTypeWmls, kTypeWmlsObject,
 *							eRpDataTypeWbmp, kTypeWbmpObject
 *		02/12/01	amp		add eRpDataTypeMessagePartial
 *		07/25/00	rhb		Support SSL/TLS
 *		06/20/00	rhb		add eRpDataTypeJs
 *		06/20/00	amp		add eRpDataTypeMdn, eRpDataTypeDsn
 *		05/13/00	bva		add RpConvertMimeTypeToFileExtension
 *		02/22/00	rhb		make MimeExtensionConversion non-static
 *		01/31/00	bva		add eRpDataTypeXbmp, kTypeXbmpObject
 *		01/07/00	bva		add eRpDataTypePdf, kTypePdfObject
 * * * * Release 3.06  * * *
 *		12/16/99	bva		add eRpDataTypeWml, kTypeWmlObject
 * * * * Release 3.05  * * *
 *		08/31/99	rhb		add eRpDataTypeSwf, kTypeSwfObject
 * * * * Release 3.04  * * *
 * * * * Release 3.0 * * * *
 *		02/06/99	bva		RpMimes.h -> AsMimes.h
 *		01/31/99	bva		add eRpDataTypeWav, kTypeWavObject
 *		12/30/98	bva		move eRpDataTypeForm definition
 *		12/14/98	rhb		remove eRpDataTypeSoftHtml
 * * * * Release 2.2 * * * *
 *		10/23/98	pjr		add RpStringToMimeType prototype, gMimeTypes
 *		10/21/98	bva		move rpStringToMimeTypeTable
 *		09/24/98	bva		add eRpDataTypeCss
 *		08/31/98	bva		remove <CR><LF> from kTypeHttpTrace
 *		08/12/98	rhb		do file extension -> MIME type conversions
 *		07/19/98	rhb		add eRpDataTypeSoftHtml
 *		07/06/98	bva		add Mime Type strings from RomPager.h
 *		05/27/98	bva		add eRpDataTypeXml
 * * * * Release 2.1 * * * *
 *		03/19/98	rhb		add eRpDataTypeMessageRfc822
 *		01/22/98	bva		add kMaxMimeTypeLength
 * * * * Release 2.0 * * * *
 *		12/11/97	bva		add eRpDataTypeIpp
 *		11/19/97	rhb		add eRpDataTypePageRedirect
 *		05/26/97	bva		add eRpDataTypeFormMultipart
 * * * * Release 1.6 * * * *
 *		03/10/97	bva		add eRpDataTypeImagePng
 *		01/23/97	rhb		created
 * * * * Release 1.5 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_AS_MIMES_
#define	_AS_MIMES_


/* 
	Mime Data Type Definitions 
*/
typedef enum {
	eRpDataTypeHtml,
	eRpDataTypeImageGif,
	eRpDataTypeApplet,
	eRpDataTypeText,
	eRpDataTypeImageJpeg,
	eRpDataTypeImagePict,
	eRpDataTypeImageTiff,			
	eRpDataTypeImagePng,			
	eRpDataTypeForm,
	eRpDataTypeIpp,			
	eRpDataTypeCss,			
	eRpDataTypeXml,			
	eRpDataTypeWav,			
	eRpDataTypeSwf,			
	eRpDataTypeWml,			
	eRpDataTypeWmls,			
	eRpDataTypeWbmp,			
	eRpDataTypePdf,			
	eRpDataTypeXbmp,			
	eRpDataTypeMdn,
	eRpDataTypeDsn,
#if RomPagerSecure || RomWebClientSecure
	eRpDataTypeCertificate,			
#endif
	eRpDataTypeJs,			
	
/*	
	The types prior to here are used to index into the string array 
	to serve up the matching Mime Type string.  Any additional types
	should be inserted before here, and the appropriate string inserted
	in the Mime Type table in AsMimes.c.  The types following here are 
	used internally by the engine for various signalling.
*/
	eRpDataTypeAnyImage,
	eRpDataTypeMap,
	eRpDataTypeFormGet,
	eRpDataTypeFormMultipart,
	eRpDataTypePageRedirect,
	eRpDataTypeMultiPart,
	eRpDataTypeMultiPartAlternative,
	eRpDataTypeMultiPartIgnore,
	eRpDataTypeMessagePartial,
	eRpDataTypeMessageRfc822,
	eRpDataTypeNone,
	eRpDataTypeOther,
	eRpDataTypeAll
} rpDataType;

/*
	The kMaxMimeTypeLength definition is used for storage allocation in
	various protocol structures.
*/

#define kMaxMimeTypeLength	50

/*
	The string definitions for various Mime Types.
*/

#define kTypeApplet						"application/octet-stream"
#define kTypeHtml						"text/html"
#define kTypeText						"text/plain"
#define kTypeXmlObject					"text/xml"
#define kTypeCssObject					"text/css"
#define kTypeGifImage					"image/gif"
#define kTypeJpegImage					"image/jpeg"
#define kTypePictImage					"image/pict"
#define kTypePictImageAlt				"image/x-pict"
#define kTypePictImageAlt2				"image/x-macpict"
#define kTypePngImage					"image/png"
#define kTypePngImageAlt				"image/x-png"
#define kTypeTiffImage					"image/tiff"
#define kTypeHttpTrace					"message/http"
#define kTypeServerPush					"multipart/x-mixed-replace;boundary="
#define kTypeNormalForm					"application/x-www-form-urlencoded"
#define kTypeMultipartForm				"multipart/form-data"
#define kTypeIppObject					"application/ipp"
#define kTypeWavObject					"audio/wav"
#define kTypeSwfObject					"application/x-shockwave-flash"
#define kTypeWmlObject					"text/vnd.wap.wml"
#define kTypeWmlsObject					"text/vnd.wap.wmlscript"
#define kTypeWbmpObject					"image/vnd.wap.wbmp"
#define kTypePdfObject					"application/pdf"
#define kTypeXbmpObject					"image/x-xbitmap"
#define kTypeMdnReceipt					"message/disposition-notification"
#define kTypeDsnReceipt					"message/delivery-status"
#define kTypeJsObject					"application/x-javascript"
#if RomPagerSecure || RomWebClientSecure
#define kTypeAsSecurityObject			"application/x-x509-ca-cert"
#endif


/*
	This structure is used to convert file name extensions to MIME types
*/

typedef struct {
	char *			fExtensionPtr;
	rpDataType		fMimeType;
} MimeExtensionConversion, *MimeExtensionConversionPtr;


/*
	This structure is used to convert MIME strings to MIME types
*/
typedef struct {
	char *			fString; 
	rpDataType		fMimeType; 
} rpStringToMimeTypeTable, *rpStringToMimeTypeTablePtr;


/*
	The conversion tables
*/

extern const char *				gMimeTypes[];
extern rpStringToMimeTypeTable	gRpStringToMimeTypeTable[];

#if RomPagerFileSystem
extern	MimeExtensionConversion	gMimeExtensionConversions[];
#endif


/*
	The prototypes
*/

#if RomPagerFileSystem
extern rpDataType	RpConvertFileExtensionToMimeType(char *theExtensionPtr);
extern char * 		RpConvertMimeTypeToFileExtension(rpDataType theDataType);
#endif	/* RomPagerFileSystem */
extern rpDataType	RpStringToMimeType(char * theTypeStringPtr);

#endif

