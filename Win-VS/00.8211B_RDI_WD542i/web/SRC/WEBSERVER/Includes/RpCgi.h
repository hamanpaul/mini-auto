/*
 *	File:		RpCgi.h
 *
 *	Contains:	Embedded Web Server definitions used by for User Exit processing
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
 *		12/18/02	bva		bump fConnectionId size
 *		12/03/02	rhb		add eRpCgiHttpIntServerErr
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *      07/25/02    bva     add fSoapActionPtr to rpCgi
 * * * * Release 4.10  * * *
 * * * * Release 4.00  * * *
 *		05/23/01	rhb		add fIsTlsFlag to rpCgi
 *		05/22/01	rhb		add includes
 *		11/07/00	pjr		RpUserExitInit is now passed theTaskDataPtr
 * * * * Release 3.0 * * * *
 *		02/23/99	pjr		pass RpExternalCgi an rpCgiPtr instead of void *
 *							fUserData -> fUserDataPtr
 * * * * Release 2.2 * * * *
 *		10/14/98	bva		add security fields to rpCgi structure
 *		07/01/98	bva		make fHostIndex Unsigned16
 *		07/01/98	pjr		add fHostIndex to the rpCgi structure
 *		06/26/98	pjr		add RpUserExitInit and RpUserExitDeInit prototypes
 *							and include RpError.h
 * * * * Release 2.1 * * * *
 *		04/02/98	bva		add rpCgiHttpResponse types, fObjectDate 
 *		03/26/98	pjr		add fDataType
 * * * * Release 2.0 * * * *
 *		10/28/97	bva		added to project
 * * * * Release 1.6 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef _RPCGI_
#define _RPCGI_

#include "AsError.h"
#include "AsTypes.h"
#include "AsMimes.h"

/* 
 	Cgi HTTP requests
*/
typedef enum {							
	eRpCgiHttpGet = 1,		/*	Cgi request is HTTP GET 			*/
	eRpCgiHttpHead,			/*	Cgi request is HTTP HEAD 			*/
	eRpCgiHttpPost			/*	Cgi request is HTTP POST			*/
} rpCgiHttpRequest;


/* 
 	Cgi call responses
*/
typedef enum {							
	eRpCgiPending,			/*	Cgi processing incomplete 						*/
	eRpCgiBufferComplete,	/*	Cgi buffer ready - there will be more buffers  	*/
	eRpCgiLastBuffer		/*	Cgi buffer ready - no more buffers				*/
} rpCgiResponse;

/* 
 	Cgi HTTP responses
*/
typedef enum {							
	eRpCgiHttpOk,			/*	Cgi returns HTTP 200 Ok 				*/
	eRpCgiHttpOkStatic,		/*	Cgi returns HTTP 200 Ok - Static Object	*/
	eRpCgiHttpRedirect,		/*	Cgi returns HTTP 302 Moved Temp 		*/
	eRpCgiHttpNotModified,	/*	Cgi returns HTTP 304 Not Modified 		*/
	eRpCgiHttpUnauthorized,	/*	Cgi returns HTTP 401 Unauthorized 		*/
	eRpCgiHttpNotFound,		/*	Cgi returns HTTP 404 Not Found				*/
	eRpCgiHttpIntServerErr	/*	Cgi returns HTTP 500 Internal Server Error	*/
} rpCgiHttpResponse;

/*
	rpCgi structure
*/
typedef struct {
	/*
		Request fields
	*/
	Unsigned16 			fConnectionId;
	rpCgiHttpRequest	fHttpRequest;
	char *				fPathPtr;		/*	URL 					*/
	char *				fHostPtr;		/*	Host:					*/
	char *				fRefererPtr;	/*	Referer:				*/
	char *				fAgentPtr;		/*	User-Agent:				*/
	char *				fLanguagePtr;	/*	Content-Language:		*/
#if RomPagerCaptureSoapAction
	char *				fSoapActionPtr;	/*	Soap-Action:			*/
#endif
	Unsigned32 			fBrowserDate;	/*	Date:	(internal)		*/
	char *				fArgumentBufferPtr;
	Signed32 			fArgumentBufferLength;
	char *				fUserNamePtr;	/*	Username from Authorization	*/
	char *				fPasswordPtr;	/*	Password from Authorization	*/
#if RomPagerSecure
	Boolean				fIsTlsFlag;		/*	Request is TLS			*/
#endif
	/*
		Request/Response fields
	*/
	void *	 			fUserDataPtr;	/*	Arbitrary User Data		*/
	/*
		Response fields
	*/
	rpCgiResponse		fResponseState;
	rpCgiHttpResponse	fHttpResponse;
	rpDataType			fDataType;
	char *				fResponseBufferPtr;
	Signed32			fResponseBufferLength;
	Unsigned32 			fObjectDate;	/*	Object Date	(internal)	*/
	Unsigned16 			fHostIndex;
} rpCgi, *rpCgiPtr;


/*
	Routines to initialize and de-initialize User Exit resources.
*/
extern RpErrorCode		RpUserExitInit(void *theTaskDataPtr);
extern RpErrorCode		RpUserExitDeInit(void);

/*
	The External User Interface call
*/
extern void RpExternalCgi(void *theTaskDataPtr, rpCgiPtr theCgiPtr);

/*

	The RomPager engine will pass control to an external CGI routine
	by issuing the RpExternalCgi call when it determines that the URL 
	needs to be handled externally.  It passes a condensed form of the 
	information in the HTTP request to the external CGI routine using 
	the rpCgi control block, and looks for responses from the CGI routine
	in the same structure.   
	
	The arguments passed to the CGI routine include the number of the 
	TCP connection which is passed in fConnectionId.  The HTTP request 
	type (GET, HEAD, or POST) is passed as an enum in fHttpRequest.  
	The URL is passed in fPathPtr and the contents of various HTTP 
	headers are passed in fHostPtr, fRefererPtr, fAgentPtr, and 
	fLanguagePtr.  The contents of the Date header are passed in 
	fBrowserDate after being converted to the RomPager internal format 
	of seconds since 1/1/1901.  The fArgumentBufferPtr and 
	fArgumentBufferLength fields point to any query arguments appended 
	to a GET request, or the object body of a POST request.  If the
	browser has provided authentication information, it will be passed in
	the fUserNamePtr and fPasswordPtr fields.
	
	The fResponseState field is used by the CGI routine to signal it's
	processing state.  Since some CGI processes may need to run 
	asynchronously, they can signal this using the eRpCgiPending state
	and the RomPager engine will issue another RpExternalCgi call at a 
	later time to gather the response.  If the eRpCgiLastBuffer state
	is returned, then the engine knows the CGI process is complete and
	will send the response back to the browser client.  If the 
	eRpCgiBufferComplete state is returned, the engine will send the 
	response back to the browser client and issue another RpExternalCgi
	call to gather more of the response.  
	
	The fHttpResponse, fDataType and fObjectDate fields are used to tell 
	the engine which HTTP headers to prepare for the response.  The normal
	response will be eRpCgiHttpOk for a dynamically prepared object and
	eRpCgiHttpOkStatic for a static object such as a GIF or JPEG image. 
	The eRpCgiHttpRedirect response is used after processing a form to
	tell the browser which page to retrieve next. The eRpCgiHttpNotModified
	response is used for requests for a static object that have been 
	previously filled.  This response can be used to save sending the
	object to the browser again. The eRpCgiHttpNotFound response is used
	to notify the browser that the CGI routine was not able to handle this
	request. The eRpCgiHttpUnauthorized response is used to tell the
	browser that the provided User and Password fields are invalid.
	
	The values for fDataType are the MIME types specified in AsMimes.h. The 
	value of fObjectDate is a date in internal RomPager format if the object 
	is a static object.  The fResponseBufferPtr and fResponseBufferLength 
	point to the HTML response buffer prepared by the external CGI routine.  
	If the fHttpResponse field is eRpCgiHttpRedirect, then the external CGI 
	routine needs to provide the URL to redirect to in the response buffer.
	If the fHttpResponse field is eRpCgiHttpUnauthorized, then the external CGI 
	routine needs to provide the realm name in the response buffer.
	
*/

#endif /* RPCGI */
