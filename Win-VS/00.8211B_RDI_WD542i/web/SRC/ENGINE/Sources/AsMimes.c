/*
 *	File:		AsMimes.c
 *
 *	Contains:	Contains the data for RomPager MIME tables and conversions
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
 *		11/24/02	rhb		fix RpConvertFileExtensionToMimeType for .class
 *		10/04/02	amp		add eRpDataTypeCertificate and .der files
 * * * * Release 4.12  * * *
 *		09/26/02	rhb		fix extension handling overflow
 *		09/23/02	rhb		fix warning
 * * * * Release 4.11  * * *
 * * * * Release 4.10  * * *
 *		04/05/02	rhb		prevent extension handling overflow
 * * * * Release 4.07  * * *
 *		01/15/02	rhb		recognize .xsl as a file extension
 * * * * Release 4.06  * * *
 * * * * Release 4.00  * * *
 *		05/30/01	bva		enable RpStringToMimeType for IPP
 *		03/06/01	bva		add eRpDataTypeWmls, eRpDataTypeWbmp
 *		02/14/00	amp		fix RpConvertFileExtensionToMimeType
 *		01/30/00	bva		make RpConvertFileExtensionToMimeType case insensitive
 *		08/08/00	bva		fix warning
 *		06/20/00	amp		add eRpDataTypeMdn, eRpDataTypeDsn
 *		06/20/00	rhb		add eRpDataTypeJs
 *		05/25/00	rhb		Support SSL/TLS
 *		05/13/00	bva		add RpConvertMimeTypeToFileExtension
 *		04/24/00	rhb		add entries in gRpStringToMimeTypeTable for 
 *								kTypeIppObject and kTypeMultipartForm
 *		02/22/00	rhb		make MimeExtensionConversion non-static
 *		02/09/00	bva		use AsEngine.h
 *		01/31/00	bva		add eRpDataTypeXbmp, kTypeXbmpObject
 *		01/31/00	amp		add RomPopBasic conditionals
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 *		01/10/00	amp		add .txt conversion support
 *		01/07/00	bva		change RpConvertFileExtensionToMimeType to set
 *							unidentified extension types to generic binary,
 *							add .bin conversion support
 *		01/07/00	bva		add eRpDataTypePdf, kTypePdfObject
 * * * * Release 3.06  * * *
 *		12/16/99	bva		add eRpDataTypeWml, kTypeWmlObject
 * * * * Release 3.05  * * *
 *		08/31/99	rhb		add eRpDataTypeSwf, kTypeSwfObject
 * * * * Release 3.04  * * *
 * * * * Release 3.0 * * * *
 *		04/02/99	rhb		convert the .htc extension to an HTML MIME type
 *		01/31/99	bva		add eRpDataTypeWav, kTypeWavObject
 *		01/22/99	pjr		RpMimes.c -> AsMimes.c
 *		01/14/99	pjr		enable gMimeTypes for RomWebClient
 *		01/05/99	bva		add types to gMimeExtensionConversions
 *		12/30/98	bva		add kTypeNormalForm to gMimeTypes
 *		12/24/98	pjr		add RomPagerBasic conditional
 *		12/14/98	rhb		remove special Soft Page Items
 *		12/08/98	rhb		eliminate warning	
 * * * * Release 2.2 * * * *
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		11/03/98	pjr		enable RpStringToMimeType for RomPagerPutMethod
 *		10/23/98	pjr		add RpStringToMimeType
 *		10/16/98	pjr		conditionialize	gMimeTypes
 *		09/24/98	bva		add eRpDataTypeCss support
 *		08/12/98	rhb		created	from RpData.c and sfil.c	
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */
 
#include "AsEngine.h"

#if RomPagerServer || RomPagerBasic || RomMailer || RomWebClient
/*
	Mime Type table used to provide conversion from the
	RomPager internal MIME types to external string definitions.
	
	The strings in this table need to be in the same 
	order as the rpDataType definitions in AsMimes.h.
*/

const char *gMimeTypes[] = {
	kTypeHtml,
	kTypeGifImage,
	kTypeApplet,
	kTypeText,
	kTypeJpegImage,
	kTypePictImage,
	kTypeTiffImage,
	kTypePngImage,
	kTypeNormalForm,
	kTypeIppObject,
	kTypeCssObject,
	kTypeXmlObject,
	kTypeWavObject,
	kTypeSwfObject,
	kTypeWmlObject,
	kTypeWmlsObject,
	kTypeWbmpObject,
	kTypePdfObject,
	kTypeXbmpObject,
	kTypeMdnReceipt,
	kTypeDsnReceipt,
#if RomPagerSecure
	kTypeAsSecurityObject,			
#endif
	kTypeJsObject
};
#endif	/* RomPagerServer || RomPagerBasic || RomMailer || RomWebClient */


#if RomPagerFileUpload || RomPagerIpp || RomPagerRemoteHost || RomPop || RomPopBasic || RomWebClient || RomPagerPutMethod

/*
	Table for converting MIME type strings to RomPager's MIME type enum.
*/

rpStringToMimeTypeTable gRpStringToMimeTypeTable[] = {
	{ kTypeApplet,			eRpDataTypeApplet },
	{ kTypeGifImage,		eRpDataTypeImageGif },
	{ kTypeHtml,			eRpDataTypeHtml },
	{ kTypeJpegImage,		eRpDataTypeImageJpeg },
	{ kTypePictImage,		eRpDataTypeImagePict },
	{ kTypePictImageAlt,	eRpDataTypeImagePict },
	{ kTypePictImageAlt2,	eRpDataTypeImagePict },
	{ kTypePngImage,		eRpDataTypeImagePng },
	{ kTypePngImageAlt,		eRpDataTypeImagePng },
	{ kTypeText,			eRpDataTypeText },
	{ kTypeTiffImage,		eRpDataTypeImageTiff },
	{ kTypeCssObject,		eRpDataTypeCss },
	{ kTypeXmlObject,		eRpDataTypeXml },
	{ kTypeWavObject,		eRpDataTypeWav },
	{ kTypeSwfObject,		eRpDataTypeSwf },
	{ kTypeWmlObject,		eRpDataTypeWml },
	{ kTypeWmlsObject,		eRpDataTypeWmls },
	{ kTypeWbmpObject,		eRpDataTypeWbmp },
	{ kTypePdfObject,		eRpDataTypePdf },
	{ kTypeXbmpObject,		eRpDataTypeXbmp },
	{ kTypeMdnReceipt,		eRpDataTypeMdn },
	{ kTypeDsnReceipt,		eRpDataTypeDsn },
	{ kTypeJsObject,		eRpDataTypeJs },
	{ kTypeIppObject,		eRpDataTypeIpp },
	{ kTypeMultipartForm,	eRpDataTypeFormMultipart },
#if RomPagerSecure
	{ kTypeAsSecurityObject,	eRpDataTypeCertificate },
#endif
	{ (char *) 0,			(rpDataType) 0 }
};


rpDataType RpStringToMimeType(char * theTypeStringPtr) {
	rpStringToMimeTypeTablePtr	theTablePtr;

	theTablePtr = gRpStringToMimeTypeTable;

	while (theTablePtr->fString != 0) {
		if (RP_STRCMP(theTypeStringPtr, theTablePtr->fString) == 0) {
			return theTablePtr->fMimeType;
		}
		theTablePtr++;
	}

	return eRpDataTypeOther;
}

#endif	/* RomPagerFileUpload || RomPagerIpp || RomPagerRemoteHost || RomPop || RomPopBasic || RomPagerPutMethod */


#if RomPagerFileSystem

MimeExtensionConversion	gMimeExtensionConversions[] = {
	{ ".jar",	eRpDataTypeApplet   },
	{ ".class",	eRpDataTypeApplet   },
	{ ".html",	eRpDataTypeHtml     },
	{ ".htm",	eRpDataTypeHtml     },
	{ ".htc",	eRpDataTypeHtml     },
	{ ".gif",	eRpDataTypeImageGif },
	{ ".tif",   eRpDataTypeImageTiff },
	{ ".jpg",   eRpDataTypeImageJpeg },
	{ ".jpeg",  eRpDataTypeImageJpeg },
	{ ".pict",  eRpDataTypeImagePict },
	{ ".pct",   eRpDataTypeImagePict },
	{ ".frm",   eRpDataTypeForm },
	{ ".xml",	eRpDataTypeXml },
	{ ".xsl",	eRpDataTypeXml },
	{ ".css",	eRpDataTypeCss },
	{ ".wav",	eRpDataTypeWav },
	{ ".swf",	eRpDataTypeSwf },
	{ ".wml",	eRpDataTypeWml },
	{ ".wmls",	eRpDataTypeWmls },
	{ ".wbmp",	eRpDataTypeWbmp },
	{ ".txt",	eRpDataTypeText },
	{ ".bin",	eRpDataTypeApplet },
	{ ".pdf",	eRpDataTypePdf },
	{ ".xbm",	eRpDataTypeXbmp },
	{ ".js",	eRpDataTypeJs },
#if RomPagerSecure
	{ ".der",	eRpDataTypeCertificate },
#endif
	{ 0 }
};


rpDataType RpConvertFileExtensionToMimeType(char *theFileNamePtr) {
	char *						theBufferPtr;
	char						theChar;
	MimeExtensionConversionPtr	theConversionPtr;
	char						theExtension[7];
	char *						theExtensionPtr;
	rpDataType					theMimeType;

	/*
		Look for an extension at the end of the file name.
	*/
	theConversionPtr = gMimeExtensionConversions;
	theExtensionPtr = theFileNamePtr + RP_STRLEN(theFileNamePtr);
	while (theExtensionPtr > theFileNamePtr && *theExtensionPtr != kAscii_Dot) {
		theExtensionPtr -= 1;
	}
	if (RP_STRLEN(theExtensionPtr) > (sizeof(theExtension) - 1)) {
		/*
			The extension is longer than any we know of, since the
			character array has room for ".xxxx<0>".
		*/
		theFileNamePtr = theExtensionPtr;
	}
	else if (theExtensionPtr > theFileNamePtr) {		
		/*
			We found an extension, so convert it to lower case.
		*/
		theBufferPtr = theExtension;
		*theBufferPtr++ = *theExtensionPtr++;

		while (*theExtensionPtr != '\0') {
			theChar = *theExtensionPtr++;
			if (theChar >= kAscii_A && theChar <= kAscii_Z) {
				/*
					Convert character to lower case.
				*/
				*theBufferPtr++ = theChar ^ kAscii_Space;
			}
			else {
				/*
					Character is already lower case. Just copy it.
				*/
				*theBufferPtr++ = theChar;
			}
		}
		*theBufferPtr = '\0';
				
		/*
			Now try to match it against the strings in 
			gMimeExtensionConversions.
		*/
		theExtensionPtr = theExtension;
		while ((theConversionPtr->fExtensionPtr != (char *) 0) &&
				RP_STRCMP(theConversionPtr->fExtensionPtr, theExtensionPtr)) {
			theConversionPtr += 1;
		}
	}
	/*
		If we didn't find an extension or the extension wasn't in the
		conversion table, set the type to the generic binary type.
		Otherwise, set the type to the entry matching the extension.
	*/
	if (theExtensionPtr == theFileNamePtr || 
			theConversionPtr->fExtensionPtr == (char *) 0 ) {
		theMimeType = eRpDataTypeApplet;
	}
	else {
		theMimeType = theConversionPtr->fMimeType;
	}
	return theMimeType;
}

char * RpConvertMimeTypeToFileExtension(rpDataType theMimeType) {
	MimeExtensionConversionPtr	theConversionPtr;

	theConversionPtr = gMimeExtensionConversions;
	while (theConversionPtr->fExtensionPtr != (char *) 0 && 
			theConversionPtr->fMimeType != theMimeType) {
		theConversionPtr += 1;
	}

	return theConversionPtr->fExtensionPtr;
}

#endif	/* RomPagerFileSystem */

