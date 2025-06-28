/*
 *	File:		RpData.c
 *
 *	Contains:	Contains the data for RomPager error messages and system
 *				phrase dictionary
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
 * * * * Release 4.30  * * *
 *		09/03/03	pjr		set unused fFieldMaxLength to 0 in display items
 * * * * Release 4.20  * * *
 * * * * Release 4.07  * * *
 *		03/22/02	pjr/rhb	add C_S_Error22, prevent crashes caused by error 
 *							reporting of bad URL or form items that contain
 *							unexpected extended ASCII characters
 * * * * Release 4.06  * * *
 * * * * Release 4.00  * * *
 *		02/14/01	rhb		rename rpItemError and eRpItemError_* to 
 *								asItemError and eAsItemError_*
 *		02/06/01	rhb		add  BGCOLOR="# tag to system dictionary Pretag case
 *		08/31/00	pjr		add gRpSslRequiredPage page
 *		08/23/00	rhb		use lower case for system dictionary phrases
 *		06/13/00	bva		add  BGCOLOR="# tag to system dictionary
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.10  * * *
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 *		01/06/99	pjr		move gMethodsAllowed to RpHttp.c, month and day
 *							tables to RpDate.c
 * * * * Release 2.2 * * * *
 *		09/21/98	pjr		in GetFSErrorString, use fFileSystemError from the
 *							connection block instead of the HTTP request block.
 *		08/31/98	bva		change <CR><LF> definitions
 *		08/12/98	rhb		move MIME related data to RpMimes.c
 *		08/10/98	bva		made gRpStringToMimeTypeTable conditional
 *		07/23/98	pjr		fix compiler warnings
 *		07/06/98	bva		fix warnings for gRpStringToMimeTypeTable
 *		06/10/98	bva		add PUT to gMethodsAllowed
 *		05/27/98	bva		add kTypeXmlObject to gMimeTypes
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 *		12/1/97		bva		add kTypeIppObject to gMimeTypes
 *		12/08/97	pjr		change Tues to Tue to match RFC's and HTTP spec.
 *		11/22/97	bva		rework gRpInputTooLargePage
 *		11/17/97	bva		add <DIV> tags to system dictionary
 *		11/12/97	pjr		add gRpInputTooLargePage and related items.
 *		11/10/97	rhb		add errors for 64 bit integers
 *		09/25/97	pjr		reduce the number of verbose file system errors and
 *							only include them if RomPagerFileUpload is enabled.
 *		09/24/97	pjr		change gRpPhrase \223 to eliminate extra LF in HTML 
 *							output and reduce data size.
 *		09/04/97	pjr		add gRpStringToMimeTypeTable
 *		08/29/97	bva		add RpHtmlAlignmentUsesPreTag
 *		08/18/97	bva		rework copyright page, make compressed applets conditional
 *		08/13/97	bva		add compressed applet to gMimeTypes
 *		07/14/97	bva		add OPTIONS and TRACE support
 *		07/02/97	pjr		add unexpected multipart form data page.
 *		06/25/97	pjr		add file system error handling
 * * * * Release 1.6 * * * *
 *		04/18/97	pjr		add Security Digest feature
 *		04/14/97	bva		add newlines to close tags in dictionary
 *		04/04/97	bva		gRpMonthTable format changed to match gRpDayName
 *		03/10/97	bva		add kTypePngImage to gMimeTypes
 *		03/03/97	bva		rework error pages for user dictionary
 *		02/28/97	bva		add frame definitions
 *		12/12/96	rhb		add gRpMonthDaysLeapYear
 *		12/11/96	bva		fix gRpMonthTable, gRpMonthDays for RpBuildDateString
 *		12/04/96	bva		add gMimeTypes
 * * * * Release 1.5 * * * *
 *		11/20/96	rhb		fix more compiler warnings
 *		11/13/96	rhb		fix compiler warnings
 *		11/04/96	bva		move dates and phrase dictionary
 *		10/22/96	bva		use rpObjectType definitions,
 *								move error page data from RpForm
 *		10/20/96	bva		use phrase dictionary
 *		09/24/96	rhb		support dynamically allocated engine data 
 * * * * Release 1.4 * * * *
 *		08/17/96	bva		fix URLs for error pages
 * * * * Release 1.3 * * * *
 *		07/05/96	bva		pages and forms messages unified
 *		06/27/96	bva		add referer to gRpAccessNotAllowed
 * * * * Release 1.2 * * * *
 * * * * Release 1.0 * * * *
 *		04/09/96	bva		added gRpFormAccessNotAllowed
 *		01/01/96	bva		modified Page Not Found items	
 *		12/14/95	bva		created		
 *
 *	To Do:
 */
 
#include "AsEngine.h"


#if RomPagerServer

static const char *	GetItemErrorString(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static char *		GetItemNameString(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static char *		GetItemValueString(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static char *		GetErrorPath(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static char *		GetBoxNameText(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);


/* **************************** */
/*           Error Pages        */
/* **************************** */

static char gHttpNoObjectFoundText1[] =	C_oHTML_oHEAD_oTITLE
										C_S_ObjectNotFound
										C_xTITLE_xHEAD_oBODY C_oH1
										C_S_ObjectNotFound
										C_xH1 
										C_S_RequestedUrl;
static char gHttpNoObjectFoundText2[] =	C_S_WasNotFound;
static char gHttpErrorClose1[] =		"." C_oP C_S_ReturnTo;
static char gHttpErrorClose2[] =		C_S_LastPage;
static char gHttpErrorClose3[] =		C_oP C_xBODY_xHTML;

static rpTextDisplayItem gHttpNoObjectFoundDisplayText = {
	(void *) &GetErrorPath,		/* Address for get */
	eRpVarType_Complex,			/* Get access -- get item from address */
	eRpTextType_ASCII_Extended,
	0
};


static rpTextDisplayItem gBoxNameTextItem = {
	(void *) &GetBoxNameText,	/* Address for get */
	eRpVarType_Complex,			/* Get access -- get item from address */
	eRpTextType_ASCII_Extended,
	0
};


static rpItem gHttpNoObjectFoundItems[] = {
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpNoObjectFoundText1 },
	{ eRpItemType_DisplayText,			(void *)&gHttpNoObjectFoundDisplayText },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpNoObjectFoundText2 },
	{ eRpItemType_DisplayText,			(void *)&gBoxNameTextItem },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose1 },
	{ eRpItemType_HtmlReferer,			(void *)&gHttpErrorClose2 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose3 },
	{ eRpItemType_LastItemInList }
};


rpObjectDescription gRpHttpNoObjectFoundPage = { 
	"/RpError/NoObjectFound",
	gHttpNoObjectFoundItems, 
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
};

static char gProtectedObjectText1[] =	C_oHTML_oHEAD_oTITLE
										C_S_ProtectedObject
										C_xTITLE_xHEAD_oBODY C_oH1
										C_S_ProtectedObject
										C_xH1 
										C_S_ThisObjectOnThe;
static char gProtectedObjectText2[] =	C_S_IsProtected;

extern char *html_SetBuffer(char *);
extern char *html_AppendBufferFormat(char *format, ...);
char *JumpTo_MyProtectedObjectProcess_Page_JavaScript_Text(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr)
{
	char *sp;
	rpDataPtr		theDataPtr = (rpDataPtr)theTaskDataPtr;
	rpHttpRequestPtr	theRequestPtr= theDataPtr->fCurrentHttpRequestPtr;
    Unsigned32 theRemoteAddress = theDataPtr->fCurrentConnectionPtr->fIpRemote;

	sp=html_SetBuffer(0);
	html_AppendBufferFormat("<script>\n");
	html_AppendBufferFormat("location.replace('/html/ProtectedObject.html')");
	html_AppendBufferFormat("</script>\n");
	return sp;
}

static rpTextDisplayItem gProtectedObjectProcess_Text = {
	(void*)JumpTo_MyProtectedObjectProcess_Page_JavaScript_Text,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	255
};
static rpItem gProtectedObjectItems[] = {
    { eRpItemType_DisplayText,   &gProtectedObjectProcess_Text },
	{ eRpItemType_DataZeroTerminated,	(void *)&gProtectedObjectText1 },
	{ eRpItemType_DisplayText,			(void *)&gBoxNameTextItem },
	{ eRpItemType_DataZeroTerminated,	(void *)&gProtectedObjectText2 },
//	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose1 },
//	{ eRpItemType_HtmlReferer,			(void *)&gHttpErrorClose2 },
//	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose3 },
	{ eRpItemType_LastItemInList }
};

rpObjectDescription gRpAccessNotAllowedPage = { 
	"/RpError/Protected",
	gProtectedObjectItems,
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
};


#if RomPagerSecurityDigest
static char gProtectedObjectText3[] =	" by Digest Access Authentication, which"
										" is not supported by your browser";

static rpItem gDigestUnsupportedItems[] = {
	{ eRpItemType_DataZeroTerminated,	(void *)&gProtectedObjectText1 },
	{ eRpItemType_DisplayText,			(void *)&gBoxNameTextItem },
	{ eRpItemType_DataZeroTerminated,	(void *)&gProtectedObjectText2 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gProtectedObjectText3 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose1 },
	{ eRpItemType_HtmlReferer,			(void *)&gHttpErrorClose2 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose3 },
	{ eRpItemType_LastItemInList }
};

rpObjectDescription gRpDigestUnsupportedPage = { 
	"/RpError/NoDigest",
	gDigestUnsupportedItems,
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
};
#endif	/* RomPagerSecurityDigest */

#if RomPagerSecure
static char gProtectedObjectText4[] = " and requires a secure socket connection";

static rpItem gSslRequiredItems[] = {
	{ eRpItemType_DataZeroTerminated,	(void *)&gProtectedObjectText1 },
	{ eRpItemType_DisplayText,			(void *)&gBoxNameTextItem },
	{ eRpItemType_DataZeroTerminated,	(void *)&gProtectedObjectText2 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gProtectedObjectText4 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose1 },
	{ eRpItemType_HtmlReferer,			(void *)&gHttpErrorClose2 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose3 },
	{ eRpItemType_LastItemInList }
};

rpObjectDescription gRpSslRequiredPage = { 
	"/RpError/SslRequired",
	gSslRequiredItems,
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
};
#endif	/* RomPagerSecure */

static char gServerBusyText1[] =		C_oHTML_oHEAD_oTITLE
										C_S_ServerBusy
										C_xTITLE_xHEAD_oBODY C_oH1
										C_S_ServerBusy
										C_xH1 
										C_S_Busy1;
static char gServerBusyText2[] =		C_S_Busy2;

static rpItem gServerBusyItems[] = {
	{ eRpItemType_DataZeroTerminated,	(void *)&gServerBusyText1 },
	{ eRpItemType_DisplayText,			(void *)&gBoxNameTextItem },
	{ eRpItemType_DataZeroTerminated,	(void *)&gServerBusyText2 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose1 },
	{ eRpItemType_HtmlReferer,			(void *)&gHttpErrorClose2 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose3 },
	{ eRpItemType_LastItemInList }
};

rpObjectDescription gRpServerBusyPage = { 
	"/RpError/ServerBusy",
	gServerBusyItems,
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
};

#if 0
static char gListTestText[] =	"<h2>Multiple Choices:</h2>"
     							"<ul>"
     							"<li><a href=Allegro.1>HTML, English version</a>"
     							"<li><a href=Allegro.2>HTML, French version</a>"
								"</ul>";

static rpItem gListTestItems[] = {
	{ eRpItemType_DataZeroTerminated,	&gListTestText },
	{ eRpItemType_LastItemInList }
};

rpObjectDescription gRpListTest = { 
	"/ListTest",
	gListTestItems,
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
	
};
#endif

static char gAllegroCopyrightText[] =	C_oHTML_oHEAD_oTITLE
										"Allegro Copyright"
										C_xTITLE_xHEAD_oBODY
										"RomPager Advanced Version "
										kVersion
										C_oBR
										"(C) 1995 - 2003 Allegro Software "
										"Development Corporation"
										C_xBODY_xHTML;

static rpItem gAllegroCopyrightItems[] = {
	{ eRpItemType_DataZeroTerminated,	&gAllegroCopyrightText },
	{ eRpItemType_LastItemInList }
};

rpObjectDescription gRpAllegroCopyrightPage = { 
	"/Allegro",
	gAllegroCopyrightItems,
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
	
};

static const char *gItemErrorDescriptionPtr[] = {
	C_S_Error1,
	C_S_Error2,
	C_S_Error3,
	C_S_Error4,
	C_S_Error5,
	C_S_Error6,
	C_S_Error7,
	C_S_Error8,
	C_S_Error9,
	C_S_Error10,
	C_S_Error11,
	C_S_Error12,
	C_S_Error13,
	C_S_Error14,
	C_S_Error15,
	C_S_Error16,
	C_S_Error17,
	C_S_Error18,
	C_S_Error19,
	C_S_Error20,
	C_S_Error21
};


static char gItemErrorPageText1[] =	C_oHTML_oHEAD_oTITLE
									C_S_EntryError
									C_xTITLE_xHEAD_oBODY
									C_oH2 C_S_ErrorDetected C_xH2 C_oP;

static rpTextDisplayItem gItemErrorPageItem = {
	(void *) &GetItemErrorString,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	0
};

static char gItemErrorPageText2[] =	C_oP C_S_HtmlItem C_S_Name;

static rpTextDisplayItem gItemNamePageItem = {
	(void *) &GetItemNameString,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	0
};

static char gItemErrorPageText3[] =	C_oP C_S_HtmlItem C_S_Value;

static rpTextDisplayItem gItemValuePageItem = {
	(void *) &GetItemValueString,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	0
};

static rpItem gItemErrorPageItems[] = {
	{ eRpItemType_DataZeroTerminated,	&gItemErrorPageText1},
	{ eRpItemType_DisplayText,			&gItemErrorPageItem},
	{ eRpItemType_DataZeroTerminated,	&gItemErrorPageText2},
	{ eRpItemType_DisplayText,			&gItemNamePageItem}, 
	{ eRpItemType_DataZeroTerminated,	&gItemErrorPageText3},
	{ eRpItemType_DisplayText,			&gItemValuePageItem}, 
	{ eRpItemType_DataZeroTerminated,	&gHttpErrorClose3},
	{ eRpItemType_LastItemInList }
};

rpObjectDescription gRpItemErrorPage = {
	"/RpError/ErrorItem",
	gItemErrorPageItems,
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
};


static const char *GetItemErrorString(void *theTaskDataPtr, char *theNamePtr, 
		Signed16Ptr theIndexValuesPtr) {

	rpDataPtr			theDataPtr;
	rpHttpRequestPtr	theRequestPtr;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	if (theRequestPtr->fItemError == eAsItemError_UserError) {
		return theRequestPtr->fUserErrorPtr;
	}
	else {
		return gItemErrorDescriptionPtr[theRequestPtr->fItemError - 1];
	}
}


#if RomPagerFileSystem
static char gFSErrorObjectText1[] =	C_oHTML_oHEAD_oTITLE
										C_S_ServerError
										C_xTITLE_xHEAD_oBODY C_oH1
										C_S_ServerError
										C_xH1 
										C_S_FSErrorDetected;


#if RomPagerFileUpload && RomPagerDebug

static const char *GetFSErrorString(void *theTaskDataPtr, char *theNamePtr, 
		Signed16Ptr theIndexValuesPtr);

static const char *GetFSErrorString(void *theTaskDataPtr, char *theNamePtr, 
		Signed16Ptr theIndexValuesPtr) {
	rpDataPtr			theDataPtr;
	rpConnectionPtr		theConnectionPtr;
	char * 				theFileSystemErrorString;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;

	switch (theConnectionPtr->fFileSystemError) {

		case eRpFileAlreadyExists:
			theFileSystemErrorString = C_S_DupFilename;
			break;

		case eRpFileNoRoom:
			theFileSystemErrorString = C_S_DiskFull;
			break;

		default:
		case eRpFileSystemError:
			theFileSystemErrorString = C_S_GeneralError;
			break;
	}

	return theFileSystemErrorString;
}

static rpTextDisplayItem gFSErrorPageItem = {
	(void *) &GetFSErrorString,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	0
};

static rpItem gFileSystemErrorItems[] = {
	{ eRpItemType_DataZeroTerminated,	&gFSErrorObjectText1 },
	{ eRpItemType_DisplayText,			&gBoxNameTextItem },
	{ eRpItemType_DisplayText,			&gFSErrorPageItem},
	{ eRpItemType_DataZeroTerminated,	&gHttpErrorClose1 },
	{ eRpItemType_HtmlReferer,			&gHttpErrorClose2 },
	{ eRpItemType_DataZeroTerminated,	&gHttpErrorClose3 },
	{ eRpItemType_LastItemInList }
};

#else	/* RomPagerFileUpload && !RomPagerDebug */

static rpItem gFileSystemErrorItems[] = {
	{ eRpItemType_DataZeroTerminated,	&gFSErrorObjectText1 },
	{ eRpItemType_DisplayText,			&gBoxNameTextItem },
	{ eRpItemType_DataZeroTerminated,	&gHttpErrorClose1 },
	{ eRpItemType_HtmlReferer,			&gHttpErrorClose2 },
	{ eRpItemType_DataZeroTerminated,	&gHttpErrorClose3 },
	{ eRpItemType_LastItemInList }
};

#endif	/* RomPagerFileUpload && RomPagerDebug */


rpObjectDescription gRpFileSystemErrorPage = { 
	"/RpError/FileSystem",
	gFileSystemErrorItems,
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
};

#endif	/* RomPagerFileSystem */


#if RomPagerFileUpload
static char gUnexpectedMpText1[] =		C_oHTML_oHEAD_oTITLE
										C_S_UnexpectedMp
										C_xTITLE_xHEAD_oBODY
										C_S_UnexpectedMp;

static rpItem gUnexpectedMpItems[] = {
	{ eRpItemType_DataZeroTerminated,	(void *)&gUnexpectedMpText1 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose1 },
	{ eRpItemType_HtmlReferer,			(void *)&gHttpErrorClose2 },
	{ eRpItemType_DataZeroTerminated,	(void *)&gHttpErrorClose3 },
	{ eRpItemType_LastItemInList }
};

rpObjectDescription gRpUnexpectedMultipart = { 
	"/RpError/UnexpectedMp",
	gUnexpectedMpItems,
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
};
#endif	/* RomPagerFileUpload */

#if RomPagerDebug || RomPagerHttpOneDotOne
static char gInputTooLargeText1[] =		C_oHTML_oHEAD_oTITLE
										"Request Too Large"
										C_xTITLE_xHEAD_oBODY C_oH1
										"Request Too Large"
										C_xH1 "The \"POST\" request is too "
										"large for the internal work buffer:"
										C_oBR C_oBLOCKQUOTE "The internal "
										"work buffer size is ";
static char gInputTooLargeText5[] =		" bytes." C_oBR "The \"POST\" request size is ";
static char gInputTooLargeText6[] =		" bytes.";
static char gInputTooLargeClose1[] =	C_xBLOCKQUOTE C_oP C_S_ReturnTo;

static Signed32	GetWorkBufferSize(void *theTaskDataPtr, char *theNamePtr,
									Signed16Ptr theIndexValuesPtr);

static Signed32	GetPostRequestSize(void *theTaskDataPtr, char *theNamePtr,
									Signed16Ptr theIndexValuesPtr);

static rpTextDisplayItem gWorkBufferSizeItem = {
	(void *) &GetWorkBufferSize,	/* Address for get */
	eRpVarType_Complex,				/* Get access -- get item from address */
	eRpTextType_Signed32,
	0
};

static rpTextDisplayItem gPostRequestSizeItem = {
	(void *) &GetPostRequestSize,	/* Address for get */
	eRpVarType_Complex,				/* Get access -- get item from address */
	eRpTextType_Signed32,
	0
};

static rpItem gInputTooLargeItems[] = {
	{ eRpItemType_DataZeroTerminated,	&gInputTooLargeText1 },
	{ eRpItemType_DisplayText,			&gWorkBufferSizeItem},
	{ eRpItemType_DataZeroTerminated,	&gInputTooLargeText5 },
	{ eRpItemType_DisplayText,			&gPostRequestSizeItem},
	{ eRpItemType_DataZeroTerminated,	&gInputTooLargeText6 },
	{ eRpItemType_DataZeroTerminated,	&gInputTooLargeClose1 },
	{ eRpItemType_HtmlReferer,			&gHttpErrorClose2 },
	{ eRpItemType_DataZeroTerminated,	&gHttpErrorClose3 },
	{ eRpItemType_LastItemInList }
};

rpObjectDescription gRpInputTooLargePage = { 
	"/RpError/InputTooLarge",
	gInputTooLargeItems,
	(rpObjectExtensionPtr) 0,
	0, 
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml, 
	eRpObjectTypeDynamic
};

static Signed32 GetWorkBufferSize(void *theTaskDataPtr,
									char *theNamePtr, 
									Signed16Ptr theIndexValuesPtr) {
	return kHttpWorkSize;
}

static Signed32 GetPostRequestSize(void *theTaskDataPtr,
									char *theNamePtr,
									Signed16Ptr theIndexValuesPtr) {
	rpDataPtr	theDataPtr = (rpDataPtr) theTaskDataPtr;
	return theDataPtr->fCurrentHttpRequestPtr->fPostRequestLength;
}
#endif	/* RomPagerDebug || RomPagerHttpOneDotOne */


static char *GetErrorPath(void *theTaskDataPtr, char *theNamePtr, 
		Signed16Ptr theIndexValuesPtr) {
	rpDataPtr			theDataPtr;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	return theDataPtr->fCurrentHttpRequestPtr->fErrorPath;
}

static char *GetBoxNameText(void *theTaskDataPtr, char *theNamePtr, 
		Signed16Ptr theIndexValuesPtr) {
	return ((rpDataPtr) theTaskDataPtr)->fBoxNameText;
}

static char *GetItemNameString(void *theTaskDataPtr, char *theNamePtr, 
		Signed16Ptr theIndexValuesPtr) {
	rpDataPtr			theDataPtr;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	return theDataPtr->fCurrentHttpRequestPtr->fCurrentItemName;
}

static char *GetItemValueString(void *theTaskDataPtr, char *theNamePtr, 
		Signed16Ptr theIndexValuesPtr) {
	rpDataPtr			theDataPtr;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	return theDataPtr->fCurrentHttpRequestPtr->fCurrentItemValue;
}

asItemError RpGetConversionErrorCode(void *theTaskDataPtr) {
	rpDataPtr			theDataPtr;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	return theDataPtr->fCurrentHttpRequestPtr->fItemError;
}

/*
	This routine allows the user application to set a custom error message.

	Note that this error message will be run through the the RomPager
	SendDataOut routines that expand the message using the dictionaries,
	so it must not contain invalid byte codes that could cause RomPager
	to bus error or crash.
*/

void RpSetUserErrorMessage(void *theTaskDataPtr, 
								char *theMessagePtr) {
	rpDataPtr			theDataPtr;
	rpHttpRequestPtr	theRequestPtr;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = eAsItemError_UserError;
	theRequestPtr->fUserErrorPtr = theMessagePtr;

	return;
}


/*
	Phrase dictionary used in RpHtml.c
*/

#if RpHtmlAlignmentUsesPreTag

const char *gRpPhrases[] = {
	 "<p>&nbsp;</p>",							/* \200 */
	 C_xTD C_oP C_xTR C_xTABLE,					/* \201 */
	 C_xTD C_oP C_xTR,							/* \202 */
	 "<tr" C_ALIGN_LEFT,						/* \203 */
	 "<tr" C_ALIGN_CENTER,						/* \204 */
	 "<tr" C_ALIGN_RIGHT,						/* \205 */
	 "<th" C_ALIGN_LEFT,						/* \206 */
	 "<th" C_ALIGN_CENTER,						/* \207 */
	 "<th" C_ALIGN_RIGHT,						/* \210 */
	 "<td" C_ALIGN_LEFT,						/* \211 */
	 "<td" C_ALIGN_CENTER,						/* \212 */
	 "<td" C_ALIGN_RIGHT,						/* \213 */
	 C_oTD C_oBR C_xTD,							/* \214 */
	 "<table" C_CELLPADDING,					/* \215 */
	 "<table" C_CELLSPACING,					/* \216 */
	 "<table" C_BORDER,							/* \217 */
	 C_oTD C_oANCHOR_HREF,						/* \220 */
	 C_xANCHOR C_xTD,							/* \221 */
	 C_oHTML "\n" C_oHEAD "\n" C_oTITLE,		/* \222 */
	 C_xTITLE C_xHEAD C_oBODY ">\n",			/* \223 */
	 "\n" C_xBODY C_xHTML,						/* \224 */
	 "</form>",									/* \225 */
	 "<center>",								/* \226 */
	 "</center>",								/* \227 */
	 "<blockquote>",							/* \230 */
	 "\n</blockquote>",							/* \231 */
	 "<a href=\"",								/* \232 */
	 "</a>",									/* \233 */
	 "<img src=\"",								/* \234 */
	 "<h1>",									/* \235 */
	 "</h1>",									/* \236 */
	 "<h2>",									/* \237 */
	 "</h2>",									/* \240 */
	 "<h3>",									/* \241 */
	 "</h3>",									/* \242 */
	 "<h4>",									/* \243 */
	 "</h4>",									/* \244 */
	 "<p>",										/* \245 */
	 "</p>",									/* \246 */
	 "<hr>",									/* \247 */
	 "&nbsp;",									/* \250 */
	 "<br>",									/* \251 */
	 "<table>",									/* \252 */
	 "</table>",								/* \253 */
	 "<tr>",									/* \254 */
	 "</tr>",									/* \255 */
	 "<th>",									/* \256 */
	 "</th>",									/* \257 */
	 "<td>",									/* \260 */
	 "</td>",									/* \261 */
	 "<code>",									/* \262 */
	 "</code>",									/* \263 */
	 "<font size=",								/* \264 */
	 "<b>",										/* \265 */
	 "</b>",									/* \266 */
	 "<i>",										/* \267 */
	 "</i>",									/* \270 */
	 "<html>",									/* \271 */
	 "</html>",									/* \272 */
	 "<head>",									/* \273 */
	 "</head>",									/* \274 */
	 "<meta",									/* \275 */
	 "<body",									/* \276 */
	 "</body>",									/* \277 */
	 "<title>",									/* \300 */
	 "</title>",								/* \301 */
	 " width=",									/* \302 */
	 " height=",								/* \303 */
	 C_ALIGN "top",								/* \304 */
	 C_ALIGN "middle",							/* \305 */
	 C_ALIGN "bottom",							/* \306 */
	 C_ALIGN "left",							/* \307 */
	 C_ALIGN "center",							/* \310 */
	 C_ALIGN "right",							/* \311 */
	 " valign=top",								/* \312 */
	 " valign=middle",							/* \313 */
	 " valign=bottom",							/* \314 */
	 " cellpadding=",							/* \315 */
	 " border=",								/* \316 */
	 " cellspacing=",							/* \317 */
	 " colspan=",								/* \320 */
	 " rowspan=",								/* \321 */
	 " name=",									/* \322 */
	 " content=",								/* \323 */
	 " alt=\"",									/* \324 */
	 "<td colspan=",							/* \325 */
	 "</font>",									/* \326 */
	 "<frameset>",								/* \327 */
	 "</frameset>",								/* \330 */
	 "<frame scrolling=",						/* \331 */
	 " align=",									/* \332 */
	 "&quot;",									/* \333 */
	 "<div>",									/* \334 */
	 "<div",									/* \335 */
	 "</div>",									/* \336 */
	 " bgcolor=\"#",							/* \337 */
};

#else

const char *gRpPhrases[] = {
	 "<p>&nbsp;</p>\n",							/* \200 */
	 C_xTD C_oP C_xTR C_xTABLE "\n",			/* \201 */
	 C_xTD C_oP C_xTR "\n",						/* \202 */
	 "<tr" C_ALIGN "left",						/* \203 */
	 "<tr" C_ALIGN "center",					/* \204 */
	 "<tr" C_ALIGN "right",						/* \205 */
	 "<th" C_ALIGN "left",						/* \206 */
	 "<th" C_ALIGN "center",					/* \207 */
	 "<th" C_ALIGN "right",						/* \210 */
	 "<td" C_ALIGN "left",						/* \211 */
	 "<td" C_ALIGN "center",					/* \212 */
	 "<td" C_ALIGN "right",						/* \213 */
	 C_oTD C_oBR C_xTD,							/* \214 */
	 "<table" C_CELLPADDING,					/* \215 */
	 "<table" C_CELLSPACING,					/* \216 */
	 "<table" C_BORDER,							/* \217 */
	 C_oTD C_oANCHOR_HREF,						/* \220 */
	 C_xANCHOR C_xTD "\n",						/* \221 */
	 C_oHTML "\n" C_oHEAD "\n" C_oTITLE,		/* \222 */
	 C_xTITLE C_xHEAD C_oBODY ">\n",			/* \223 */
	 "\n" C_xBODY C_xHTML,						/* \224 */
	 "\n</form>\n",								/* \225 */
	 "<center>",								/* \226 */
	 "</center>\n",								/* \227 */
	 "<blockquote>\n",							/* \230 */
	 "\n</blockquote>\n",						/* \231 */
	 "<a href=\"",								/* \232 */
	 "</a>",									/* \233 */
	 "<img src=\"",								/* \234 */
	 "<h1>",									/* \235 */
	 "</h1>\n",									/* \236 */
	 "<h2>",									/* \237 */
	 "</h2>\n",									/* \240 */
	 "<h3>",									/* \241 */
	 "</h3>\n",									/* \242 */
	 "<h4>",									/* \243 */
	 "</h4>\n",									/* \244 */
	 "<p>\n",									/* \245 */
	 "</p>\n",									/* \246 */
	 "<hr>\n",									/* \247 */
	 "&nbsp;",									/* \250 */
	 "<br>\n",									/* \251 */
	 "<table>",									/* \252 */
	 "</table>\n",								/* \253 */
	 "<tr>",									/* \254 */
	 "</tr>\n",									/* \255 */
	 "<th>",									/* \256 */
	 "</th>\n",									/* \257 */
	 "<td>",									/* \260 */
	 "</td>\n",									/* \261 */
	 "<code>\n",								/* \262 */
	 "\n</code>\n",								/* \263 */
	 "<font size=",								/* \264 */
	 "<b>",										/* \265 */
	 "</b>",									/* \266 */
	 "<i>",										/* \267 */
	 "</i>",									/* \270 */
	 "<html>",									/* \271 */
	 "</html>\n",								/* \272 */
	 "<head>",									/* \273 */
	 "</head>\n",								/* \274 */
	 "<meta",									/* \275 */
	 "<body",									/* \276 */
	 "</body>\n",								/* \277 */
	 "<title>",									/* \300 */
	 "</title>\n",								/* \301 */
	 " width=",									/* \302 */
	 " height=",								/* \303 */
	 C_ALIGN "top",								/* \304 */
	 C_ALIGN "middle",							/* \305 */
	 C_ALIGN "bottom",							/* \306 */
	 C_ALIGN "left",							/* \307 */
	 C_ALIGN "center",							/* \310 */
	 C_ALIGN "right",							/* \311 */
	 " valign=top",								/* \312 */
	 " valign=middle",							/* \313 */
	 " valign=bottom",							/* \314 */
	 " cellpadding=",							/* \315 */
	 " border=",								/* \316 */
	 " cellspacing=",							/* \317 */
	 " colspan=",								/* \320 */
	 " rowspan=",								/* \321 */
	 " name=",									/* \322 */
	 " content=",								/* \323 */
	 " alt=\"",									/* \324 */
	 "<td colspan=",							/* \325 */
	 "</font>",									/* \326 */
	 "<frameset>",								/* \327 */
	 "</frameset>\n",							/* \330 */
	 "<frame scrolling=",						/* \331 */
	 " align=",									/* \332 */
	 "&quot;",									/* \333 */
	 "<div>",									/* \334 */
	 "<div",									/* \335 */
	 "</div>\n",								/* \336 */
	 " bgcolor=\"#",							/* \337 */
};

#endif	/* RpHtmlAlignmentUsesPreTag */

#endif /* RomPagerServer */
