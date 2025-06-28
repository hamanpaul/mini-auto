/*
 *	File:		RpConfig.h
 *
 *	Contains:	RomPager Configuration Definitions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2003 by Allegro Software Development Corporation
 *	All rights reserved.
 *
 *	This module contains confidential, unpublished, proprietary
 *	source code of Allegro Software Development Corporation.
 *
 *	The copyright notice above does not evidence any actual or intended
 *	publication of such source code.
 *
 *	License is granted for specific uses only under separate
 *	written license by Allegro Software Development Corporation.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 * * * * Release 4.30  * * *
 *		09/19/03	pjr		add RpRhLocalCredentials
 * * * * Release 4.21  * * *
 *		05/07/03	amp		add RpRootObjectIsUserExit
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 * * * * Release 4.20  * * *
 *		02/21/03	pjr		add kRpMaxUserNameLength and kRpMaxPasswordLength
 *      01/28/03    rhb     add RomPagerSoftPageProcessUserExit
 *      10/07/02    bva     add RomPagerMPostMethod
 *		10/04/02	amp		move RomPagerSecureExport to A1Config.h
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *      07/25/02    bva     add RomPagerCaptureSoapAction
 *      06/26/02    rhb     add RpUserExitAggregate
 * * * * Release 4.10  * * *
 *		06/08/02	bva		remove RomPagerSlaveIdentity, RpUserExitMulti
 * * * * Release 4.07  * * *
 *      01/29/02    rhb     add RomPagerSoftPageIndexedString
 * * * * Release 4.06  * * *
 * * * * Release 4.05  * * *
 *		12/12/01	amp		change kHttpWorkSize to 1536 for RomUPNP Advanced
 * * * * Release 4.04  * * *
 * * * * Release 4.03  * * *
 *		10/12/01	rhb		add section for RomPager Secure
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		07/06/01	amp		add kUpnpDevicePrefix to indicate UPnP SOAP request
 *		02/17/01	bva		remove RomPagerPreOneFive
 *		02/16/01	rhb		move kRpCompressionEscape* to AsConfig.h
 *		02/15/01	rhb		move kRpIndexQueryDepth & kRpHexSeparator to AsConfig.h
 *		02/15/01	bva		remove dictionary patching
 *		02/13/01	rhb		move 64-bit typedefs back to AsConfig.h and move
 *								RpCustomVariableAccess, RomPagerSnmpAccess, & 
 *								RomPagerUse64BitIntegers to AsConfig.h (rename
 *								appropriately)
 *		01/05/01	pjr		add RpHtmlBufferDisplay
 *		10/06/00	pjr		rework IPP printer name allocation and setup
 *		08/31/00	pjr		add kRpNumberOfUsers for new security model
 *		08/07/00	rhb		add kRpIndexCharacter
 *		07/25/00	rhb		Support SSL/TLS
 *		06/29/00	pjr		add kIppNumberOfPrinterNames
 *		06/23/00	pjr		add RpRootObjectIsFile and kRpRootFilename
 *		06/12/00	rhb		add RpHtmlFormImage
 * * * * Release 3.10  * * *
 *		02/16/00	bva		fix comments reference
 *		01/13/00	amp		RomPopBasic demo uses RomPagerDelayedFunctions
 * * * * Release 3.06  * * *
 * * * * Release 3.05  * * *
 *		10/20/99	bva		rework SoftPages compile flags
 *		10/15/99	rhb		add user exit functionality for <rpdv>, <INPUT>, 
 *								and <SELECT> tags
 *		06/17/99	bva		add RFC 2616/2617 documentation
 *		06/14/99	bva		move 64-bit typedefs from AsTypes.h
 * * * * Release 3.04  * * *
 * * * * Release 3.0 * * * *
 *		03/27/99	bva		remove RpHtmlSubmit
 *		03/16/99	pjr		rework debug code
 *		02/26/99	bva		change connection setup definitions
 *		02/15/99	bva		add RomPagerDelayedFunctions for RomPop demo
 *		02/06/99	bva		rework documentation
 *		02/04/99	bva		add RomPagerHttpCookies
 *		01/29/99	bva		add RpEtagHeader
 *		01/24/99	pjr		move empty definitions to AsChkDef.h
 *		01/12/99	bva		change kIppPrinterName default
 *		01/10/99	pjr		split into RpConfig.h and AsConfig.h
 *		01/06/99	rhb		fix Soft Page comments
 *		12/29/98	bva		merge RomWebClient
 *		12/28/98	bva		add RpCustomVariableAccess
 *		12/14/98	bva		merge RomPagerSnmpAccess
 *		12/11/98	rhb		add RomPagerSoftPages
 * * * * Release 2.2 * * * *
 *		11/11/98	rhb		add RpFileInsertItem
 *		11/01/98	bva		add empty definitions
 *		07/18/98	bva		add RpHttpFlowDebug
 *		07/06/98	bva		add RomPagerContentDisposition
 *		07/01/98	pjr		add RpUserExitMulti
 *		06/16/98	bva		add RomPagerUserDataAreas, kRpUserDataSize, 
 *								RomPagerSlaveIdentity
 *		06/10/98	bva		add RomPagerPutMethod
 *		06/08/98	bva		add support for multiple Remote Hosts
 * * * * Release 2.1 * * * *
 *		05/23/98	bva		rework compile flags
 *		05/20/98	rhb		add RpHtmlSelectVarValue and RpHtmlVarValueDynamic
 *		04/04/98	bva		moved some info to PrConfig.h
 *		04/02/98	bva		moved some info to RmConfig.h, RpCheck.h
 *		03/20/98	bva		add RpFormFlowDebug 
 *		02/24/98	pjr		change RpPhraseDictPatching flags
 *		02/16/98	rhb		fix up RomPagerDemo and add RpUseRpAlloc
 *		02/13/98	bva		add RpHtmlSubmit 
 *		01/23/98	pjr		add RpPhraseDictPatching and related flags
 *		01/06/98	pjr		add kRpLoggingLevel
 * * * * Release 2.0 * * * *
 *		12/17/97	bva		add RomPagerDemo flags
 *		11/30/97	bva		rearrange
 *		11/10/97	rhb		add RomPagerUse64BitIntegers
 *		11/07/97	pjr		add RomPagerRemoteHost related parameters
 *		10/28/97	bva		add RpNoCache
 *		10/21/97	bva		memory allocation documentation		
 *		10/07/97	pjr		add RomPagerDynamicRequestBlocks
 *		09/24/97	pjr		add kRpCompressionEscapeString
 *		09/23/97	bva		add RpHtmlNamedSubmit 
 *		09/15/97	rhb		add RpHtmlRadioDynamic
 *		09/09/97	bva		add RpHtmlFixedSingleDynamic
 *		08/29/97	bva		add RpHtmlAlignmentUsesPreTag
 *		08/26/97	rhb		add kRpHexShiftRight
 *		08/12/97	pjr		add RomPagerExternalPassword
 *		07/31/97	rhb		add RpHtmlTextAreaBuf 
 *		07/26/97	bva		remove RomPagerContentTypeOnErrors
 *		07/25/97	bva		add kRpHexSeparator 
 *		07/16/97	bva		add RpHtmlTextFormDynamic 
 *		07/14/97	bva		add OPTIONS and TRACE support
 *		07/12/97	bva		add HTTP->IPP interface, and consistency checks
 *		06/25/97	pjr		add file upload feature
 *		06/12/97	bva		add RomPagerUrlState 
 *		06/05/97	bva		add kRpCompressionEscape, remove kRpUserPhraseStart
 * * * * Release 1.6 * * * *
 *		04/18/97	pjr		add Security Digest feature
 *		04/16/97	bva		add RomPagerHttpOneDotOne 
 *		03/05/97	bva		add RpHtmlCheckboxDynamic 
 *		01/31/97	bva		add RpSearchRomFirst
 *		01/20/97	bva		add RomPagerCaptureUserAgent
 *		12/13/96	bva		add RomPagerKeepAlive
 * * * * Release 1.5 * * * *
 *		11/06/96	bva		add RomPagerClientPull, RomPagerUrlExit
 *		10/22/96	bva		add RomPagerDelayedFunctions
 *		10/18/96	bva		consolidate HTTP headers and form items buffers
 *		10/14/96	bva		add RomPagerJavaScript
 *		10/07/96	bva		add RomPagerFullHostName
 *		10/03/96	bva		add conditionals for security and forms handling
 *		09/20/96	rhb		allow more than one HTTP request
 * * * * Release 1.4 * * * *
 * * * * Release 1.3 * * * *
 *		08/02/96	bva		add RomPagerContentTypeOnErrors, 
 *								RomPagerContentLanguage
 *		07/08/96	bva		add kRpIndexQueryDepth
 *		07/03/96	bva		add RomPagerDynamicBoxName
 *		07/02/96	bva		add kRpNumberOfRealms
 *		06/26/96	bva		optimize kHtmlMemorySize
 *		06/23/96	bva		remove RomPagerUriHeaders compile flag - 
 *							functions mandatory
 *							update image mapping comments
 * * * * Release 1.2 * * * *
 *		01/24/96	bva		created
 *
 *	To Do:
 */

#ifndef	_RPCONFIG_
#define	_RPCONFIG_


/* 
	Web Server Memory Allocation

	RomPagerDynamicGlobals, RomPagerDynamicRequestBlocks,
	kRpNumberOfHttpRequests, kHttpWorkSize, kHtmlMemorySize,
	RomPagerUserDataAreas, kRpUserDataSize

	Embedded systems memory allocation strategies are a tradeoff between 
	static allocation which minimizes memory fragmentation and dynamic 
	allocation which minimize wasted memory.  The RomPager scheduler allows 
	three strategies for memory allocation.  These are static allocation, 
	dynamic allocation at engine initialization, and dynamic allocation at 
	request initialization. Fixed global memory requirements are about 3Kb 
	for the base engine and about 5Kb per HTTP request control block.  Each 
	simultaneous HTTP connection uses one HTTP request control block.

	The first allocation strategy is static allocation which identifies all 
	memory needs at compile/link time and allocates the appropriate 
	structures from the static memory pool. To use this strategy, set the 
	RomPagerDynamicGlobals variable in AsConfig.h to 0.  The variable 
	kRpNumberOfHttpRequests defines the number of simultaneous HTTP 
	connections to support.

	The second memory allocation strategy identifies all memory usage at 
	compile/link time, allocates the memory when the engine is initialized, 
	and deallocates the memory when the engine is terminated. To use this 
	strategy, set the variable RomPagerDynamicGlobals in AsConfig.h to 1 
	and use kRpNumberOfHttpRequests to define the number of simultaneous 
	HTTP connections to support.
	
	The third strategy allocates global engine memory and a starting pool of 
	HTTP request control blocks at engine initialization.  Allocation for 
	additional HTTP request control blocks are made as needed from the pool
	of available memory.  Additional request control blocks will be allocated 
	when a TCP/HTTP request has a receive buffer to process and will be freed 
	when there is no more response data to send and the connection is about 
	to be closed.  This third strategy allows the number of simultaneous HTTP 
	requests to be limited by available memory, but increases the possibility 
	of memory fragmentation.  To use this strategy, set the variable 
	RomPagerDynamicGlobals in AsConfig.h to 1 and use kRpNumberOfHttpRequests 
	to define the initial pool of HTTP request control blocks.  Set the 
	variable RomPagerDynamicRequestBlocks to 1 signal that additional request 
	control blocks should be allocated as needed. If kRpNumberOfHttpRequests 
	is set to 0, all request control blocks will be allocated dynamically.  
	
	The HTTP request control block contains one buffer of kHttpWorkSize and 
	two buffers of kHtmlMemorySize. Changing the size of these buffers will 
	thus control the amount of memory required for a request control block.  
	The buffer of kHttpWorkSize is used to prepare HTTP headers for 
	transmission and to receive form data from the browser. If your system 
	has large form entries you may need to increase kHttpWorkSize above the 
	default size of 512. If only small forms are used, you may be able to 
	reduce the size for increased memory savings. The two buffers of 
	kHtmlMemorySize are used to prepare the HTML object responses for 
	transmission to the browser.  A double buffered scheme is used for 
	maximum I/O overlap. The default size of 1450 fits well in an ethernet 
	TCP packet. If smaller sizes are used, you will be able to save on 
	memory usage with a tradeoff that more packets will be sent for larger 
	pages, images, and applets. If the RomPagerSecure package is used, then 
	some room needs to be allowed for encryption headers.

	If the RomPagerUserDataAreas variable is set to 1, a buffer with the
	size of kRpUserDataSize will be allocated for each HTTP request control
	block and the RequestCookie will point at the data area.  This area
	is a scratch pad for user routines and can be useful for page pre-
	processing and form post-processing routines. This variable must be
	set to 1 if the PageBuilder structured access routines are used. 

	If the RomPagerHttpCookies variable is set to 1, an array of character
	strings is reserved for each HTTP request to support the storing and
	retrieval of HTTP Cookies between individual HTTP requests. The size 
	of an individual Cookie is controlled by the kHttpCookieSize value and 
	the number of Cookies to support is determined by the 
	kNumberOfHttpCookies value. 
*/

#if RomPagerDynamicGlobals
	#define RomPagerDynamicRequestBlocks		1
#endif

#define	kRpNumberOfHttpRequests		(2)

#if RomPlugAdvanced
	#define kHttpWorkSize			(1536)
#else
// +++ _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#if 0
	#define kHttpWorkSize			(512)
#else
	#define kHttpWorkSize			(1536)
#endif
// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#endif

#if RomPagerSecure
	#define kHtmlMemorySize			(1425)
#else
// +++ _Alphanetworks_Patch_, 02/03/2004, jacob_shih
// reserved for GM header
#if 1
	#define kHtmlMemorySize			(1450-16)
#else
	#define kHtmlMemorySize			(1450)
#endif
// --- _Alphanetworks_Patch_, 02/03/2004, jacob_shih
#endif

#define RomPagerUserDataAreas		0
#if RomPagerUserDataAreas
	#define kRpUserDataSize			(512)
#endif

#define RomPagerHttpCookies			1		/* 	450 bytes	*/
#if RomPagerHttpCookies
	#define kNumberOfHttpCookies	(10)		
	#define kHttpCookieSize			(40)		
// +++ _Alphanetworks_Patch_, 02/12/2004, jacob_shih
#if defined(_Alpha_RpWebID_)
    #define kRpWebIDLen             (8)
#endif	//	defined(_Alpha_RpWebID_)
// --- _Alphanetworks_Patch_, 02/12/2004, jacob_shih
#endif


/* 
	HTTP Environment
	
	HTTP 1.0 Compliance

	RomPagerMimeTypeChecking, RomPagerFullHostName

	These flags control HTTP specification compliance.  Strict conformance 
	requires that they be set to 1.  For a variety of reasons, most browsers 
	(and all the popular ones) will work if they are set to 0, thus saving 
	code space and processing time.
	
	If RomPagerMimeTypeChecking is defined as 1, the code to check the HTTP 
	Accept statements will be enabled.  Since the popular browsers will
	accept any type and RomPager serves all objects with proper Mime Types, 
	it isn't necessary to process the Accept statements.
	
	If RomPagerFullHostName is defined as 1, the code to generate a host 
	name from the IP address and port will be included.  Since most browsers 
	(Netscape 2.0 or later) pass in the host name with the request, this 
	code is generally not necessary.  The HTTP 1.1 specifications require
	browsers to send in the host name, so over time this code will be
	even less necessary.

	**** **** **** **** **** **** ****

	HTTP 1.0 Options

	RomPagerImageMapping, RomPagerClientPull, RomPagerServerPush,
	RomPagerKeepAlive
	
	The RomPager engine provides support for server-side image mapping as 
	well as client-side image mapping. If RomPagerImageMapping is defined 
	as 1, the code to build server-side image mapping support will be 
	enabled. Image and map structures will add additional memory.  Client-
	side image mapping is specified in the HTML of a page and does not 
	require additional RomPager engine code, although the HTML may be more 
	verbose than the server-side image mapping code.  If you use image 
	mapping, you should analyze your application to determine which 
	approach yields the smallest memory usage.

	Dynamic page refreshing using client pull is a technique that Netscape
	invented and that Microsoft supports.  If RomPagerClientPull is defined 
	as 1, the code to support dynamic page refreshing using client pull will 
	be generated.

	Netscape also invented a dynamic page technique called server push which 
	is more network intensive.  Microsoft does not support this technique in 
	Internet Explorer. If RomPagerServerPush is defined as 1, the code to 
	support Netscape "server push" will be generated.  
	
	Netscape created a "keep alive" technique for persistent connections in 
	a HTTP 1.0 environment.  Microsoft also supports this technique.  The
	"keep alive" persistent connections became the basis for the initial
	definition of HTTP 1.1 persistent connections, but without the use of 
	chunked encoding to signal object length, the technique provides less 
	benefits for dynamically generated pages.  If RomPagerKeepAlive is 
	defined as 1, the code to maintain persistent connections using the 
	Netscape "keep alive" technique will be generated.
	
	**** **** **** **** **** **** ****

	HTTP Browser Headers
	
	RomPagerCaptureUserAgent, RomPagerCaptureLanguage, 
	RomPagerCaptureSoapAction
	
	Browsers send various HTTP headers that may be useful to the management 
	application.  RomPager provides optional capabilities to examine these 
	headers. 
	
	The Accept-Language HTTP header is sent by browsers and may provide some 
	information about what languages a browser user is capable of displaying.  
	This header is sent in different ways by different browsers and is not 
	always a reliable indicator of the user's desires.  In general, if an 
	application will provide multiple language support, it is better to ask
	the user explicitly which language to display the pages in.  If 
	RomPagerCaptureLanguage is defined as 1, the code to capture the 
	Accept-Language HTTP header that the browser sends in will be generated.  
	Since this option adds processing overhead and 256 bytes of RAM for each 
	HTTP request control block, this flag should only be turned on if the 
	RpGetAcceptLanguage callback routine will be used.  
	
	The User-Agent header is sent by browsers and indicates which browser is 
	running.  If your management application is sending different pages to 
	different browsers, you will need to look at this header.  If 
	RomPagerCaptureUserAgent is defined as 1, the code to capture the 
	User-Agent HTTP header will be generated.  Since this option adds 
	processing overhead and 256 bytes of RAM for each HTTP request control 
	block, this flag should only be turned on if the RpGetUserAgent callback 
	routine will be used.
	
	The Soap-Action header is used for dispatching control in the Simple
	Object Access Protocol (SOAP). Applications that are building Web 
	services will want to examine this header. If RomPagerCaptureSoapAction is 
	defined as 1, the code to capture the Soap-Action HTTP header that the 
	Web client sends in will be enabled. Since this option adds processing 
	overhead and 256 bytes of RAM for each HTTP request control block, this 
	flag should only be turned on if the RpGetSoapAction callback routine 
	will be used.  

	
	**** **** **** **** **** **** ****

	HTTP Server Headers
	
	RomPagerContentDisposition

	The server can send various optional HTTP headers that may be useful to 
	the management application.  RomPager provides capabilities to set 
	these headers. 

	The Content-Disposition header may be sent by a server to indicate a 
	filename that the browser should use to save the object.  If this header
	is used in conjunction with a MIME type of "application/octet-stream", then
	the browser should put up a "Save As" dialog without attempting to display
	the object. To enable this capability, set the RomPagerContentDisposition 
	flag to 1 and set the kRpObjFlag_Disposition flag in the object header 
	of the object to be stored on the browser user's disk.
	
	**** **** **** **** **** **** ****

	HTTP 1.1 
	
	RomPagerHttpOneDotOne
	
	The HTTP 1.1 protocol is defined by RFC 2068 and additional drafts that 
	will lead to additional RFCs.  As of RomPager Release 3.00, HTTP 1.1 is 
	supported by Microsoft IE 4.0 and IE 5.0 which are based on RFC 2068 with 
	modifications. The RomPager HTTP 1.1 code has been tested and it 
	interoperates with MSIE 4.0 and MSIE 5.0. Since this feature requires extra 
	code space, and is not supported by all browsers, you may wish to turn this 
	feature off for production builds.

	If RomPagerHttpOneDotOne is defined as 1, the code to build an HTTP 1.1
	server (RFC 2068 + draft modifications) will be generated. HTTP 1.1 can 
	provide additional performance with the use of persistent connections 
	and chunked encoding.  These options add approximately 1400 bytes to the 
	server code.  HTTP 1.1 browsers know how to talk to HTTP 1.0 servers, so 
	this capability may not be a requirement for all applications.  If this 
	option is enabled, the RomPager server will respond with an identification 
	as an HTTP 1.1 server to all requests (1.0 and 1.1). The type of client 
	request will be identified, and the appropriate headers generated.  That 
	is, HTTP 1.0 client requests will receive HTTP 1.0 response headers 
	(which are a subset of HTTP 1.1 headers), and HTTP 1.1 client requests 
	will receive HTTP 1.1 response headers. 
	
	The HTTP 1.1 specifications have compliance requirements and optional
	features that may not be required for interoperability.  If memory is
	tight, these options can be turned off and the RomPager server will
	still interoperate with mainstream production HTTP 1.1 browsers.
	
	**** **** **** **** **** **** ****

	HTTP 1.1 Options
	
	RomPagerPutMethod, RomPagerOptionsMethod, RomPagerTraceMethod
	
	The HTTP 1.1 PUT method allows a HTTP client to upload a file to the
	specified UTL with less overhead than the RFC 1867 method of file 
	upload. Support for the PUT method is enabled by setting the 
	RomPagerPutMethod flag to 1.

	The HTTP 1.1 OPTIONS and TRACE methods are designed to allow browsers
	the ability to receive capabilities and debugging information from
	HTTP servers.  Support for the OPTIONS method is enabled by setting the 
	RomPagerOptionsMethod flag to 1.  Support for the TRACE method is enabled 
	by setting the RomPagerTraceMethod flag to 1.
	
	RpEtagHeader
	
	HTTP 1.1 clients and proxys can use the Etag header for caching along with
	date headers. This may provide more reliable caching, but is unnecessary
	in most environments. Support for Etag cache headers is enabled by setting 
	the RpEtagHeader flag to 1.

	**** **** **** **** **** **** ****

	HTTP 1.1 Compliance
	
	RpAbsoluteUri
	
	To allow for transition to future versions of HTTP, Section 5.1.2 
	requires compliant servers to support absolute URIs even though HTTP 1.1
	clients will not generate these requests. Support for absolute URIs
	is enabled by the RpAbsoluteUri flag.
	 
	RpExpectHeader
	
	RFC 2616 requires support of the Expect header, which is used for a client 
	to signal it wants to receive a 100 Continue response before sending a 
	request object.  This support is required for compliance with RFC 2616 but 
	not with RFC 2068 or for interoperability with MSIE and Navigator. Support 
	for the Expect header and 100 Continue response is enabled by the 
	RpExpectHeader flag.
*/

#define RomPagerMimeTypeChecking		0		/* 	550 bytes	*/ 
#define RomPagerFullHostName			1		/* 	125 bytes	*/	

#define RomPagerImageMapping 			0		/* 	600 bytes	*/
#define RomPagerClientPull				1		/* 	225 bytes	*/
#define RomPagerServerPush				0		/* 	1360 bytes	*/
#define RomPagerKeepAlive				1		/* 	325 bytes	*/

#define RomPagerCaptureLanguage			0		/* 	100 bytes	*/
// +++ _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#if 0
#define RomPagerCaptureUserAgent		0		/* 	100 bytes	*/
#else
#define RomPagerCaptureUserAgent		1		/* 	100 bytes	*/
#endif
// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#define RomPagerCaptureSoapAction		1		/* 	100 bytes	*/

#define RomPagerContentDisposition		1		/* 	100 bytes	*/

#define RomPagerHttpOneDotOne			1		/* 	3000 bytes	*/
#if RomPagerHttpOneDotOne
	#define RomPagerPutMethod			1		/* 	1300 bytes	*/
	#define RomPagerOptionsMethod		0		/* 	150 bytes	*/
	#define RomPagerTraceMethod			0		/*	425 bytes	*/
	#define RomPagerMPostMethod			1		/*	200 bytes	*/
	#define RpEtagHeader				0		/* 	350 bytes	*/
	#define RpAbsoluteUri				0		/* 	200 bytes	*/
	#define RpExpectHeader				0		/* 	250 bytes	*/
#endif


/* 
	Security
	
	RomPagerSecurity, kRpMaxUserNameLength, kRpMaxPasswordLength,
	RomPagerSecurityDigest, kRpNumberOfRealms, kPasswordSessionTimeout,
	RomPagerExternalPassword
	
	If RomPagerSecurity is defined as 1, the code for the basic security
	system will be generated.  This code provides a set of protection realms
	and support for HTTP Basic Authentication.  If all pages are unprotected 
	then RomPagerSecurity should be defined as 0, since the basic security
	features require about 1900 bytes.

	The kRpMaxUserNameLength and kRpMaxPasswordLength parameters define the 
	maximum Username and Password string lengths that will be handled by the 
	server. Any requests received with username and password strings longer 
	than these values will fail authentication.

	If RomPagerSecurityDigest is defined as 1, then additional code to 
	support HTTP Digest Authentication (RFC 2069) will be generated.  The 
	additional security levels of eRpSecurity_StrictDigestAndIpAddress, 
	eRpSecurity_DigestAndIpAddress and eRpSecurity_DigestPasswordOnly become 
	available for a realm.  If Digest Authentication is not used, then 
	RomPagerSecurityDigest should be defined as 0, which will save about 
	5000 bytes.

	kRpNumberOfRealms defines the number of separate security realm 
	structures to allocate. The maximum number is 8.  For Basic 
	Authentication each realm takes up about 800 bytes of server memory.  
	Digest Authentication adds about 64 bytes to each realm and 325 bytes 
	to the HTTP request control block. kRpNumberOfUsers defines the
	maximum number of user entries in the security user database.  Each
	user entry takes about 100 bytes of server memory.  The Web server security 
	database which contains the realms, security levels, usernames and passwords 
	is initialized in the function RpInitializeBox which is located in RpUser.c.
	
	The kPasswordSessionTimeout value defines the length of a password
	session in seconds.  If the session has been idle for a time greater 
	than this value, the browser will be challenged again, even if it has 
	provided authentication credentials with the request.

	If RomPagerExternalPassword is defined as 1, the username, password, 
	realm name and IP address are passed to an external function for 
	checking.  The function is asynchronous and allows for arbitrary delay, 
	so this capability may be used to retrieve authentication information 
	from a source external to the device as well as external to the 
	RomPager engine.

	When External Password checking is used, the external password function 
	is called the first time the browser provides a username/password after 
	being challenged.  Thereafter, as the browser continues to provide the
	username/password it is validated against the previously validated 
	username/password.  If the realm is timed out for inactivity, or if a 
	different browser attempts access, the browser will be challenged,
	and the external password function will again be called.
*/

#define RomPagerSecurity				1
#if RomPagerSecurity
#if 1 //Arthur & Jacob 2004/0206
	#define	kRpMaxUserNameLength		300   //support TACACS(255+1)
	#define	kRpMaxPasswordLength		300   //support TACACS(255+1)
#else
	#define	kRpMaxUserNameLength		32
	#define	kRpMaxPasswordLength		32
#endif
	#define RomPagerSecurityDigest		0
	#define RomPagerExternalPassword	0
	#define kPasswordSessionTimeout		(5*60)	
	#define	kRpNumberOfRealms			8
	#define	kRpNumberOfUsers			8
#endif
	

/*
	Debugging Options
 
	RomPagerDebug, RpPageFlowDebug, RpFormFlowDebug, RpNoCache, 
	RpStructuredAccessDebug
	
	If RomPagerDebug is set, various support routines and error checking
	code for tuning and debugging will be generated.  In production versions, 
	this flag should be turned off.

	If RpHttpFlowDebug is set to 1, then a set of "printf" statements that
	can be used to track the flow of the HTTP processing will be generated 
	in RpHttp.c.  A printf statement will show the path requested, the
	response, and the object buffer being sent.

	If RpPageFlowDebug is set to 1, then a set of "printf" statements that
	can be used to track the flow of HTML page creation will be generated 
	in RpHtml.c.  One printf statement exists for each of the page items
	defined in RpPages.h.  If a page object extension has the 
	kRpObjFlag_DebugFlow flag set, then that page will trigger the printf 
	statements.  This can be useful to identify which incorrectly specified 
	page item is causing the engine to hang.

	If RpFormFlowDebug is set to 1, then a set of "printf" statements that
	can be used to track the flow of forms processing will be generated 
	in RpForm.c.  One printf statement exists for each of the form items
	defined in RpPages.h.  The forms processing trace can be useful to 
	track down engine hangs caused by incorrectly specified form items.

	If RpNoCache is set to 1, then HTTP objects that are normally static
	such a help text pages, graphics and applets will be sent to the 
	browser as dynamic objects so that they are not cached.  This can be
	useful during the debugging/development process.  The flag should be
	turned off for production builds, to support best browser performance. 

	If RpStructuredAccessDebug is set to 1, some checking for overflow of
	the structured access storage areas will be performed.
*/

#if AsDebug
	#define RomPagerDebug				0
	#if RomPagerDebug
		#define RpHttpFlowDebug			0
		#define RpPageFlowDebug			0
		#define RpFormFlowDebug			0
		#define RpNoCache				1
		#define RpStructuredAccessDebug	0
	#endif
#endif


/* 
	RomPagerLogging, kRpLogEvents, kRpLoggingLevel

	If RomPagerLogging is defined as 1, the code to support logging of 
	server HTTP events will be enabled.  kRpLogEvents is the number of
	events to keep track of in the event ring buffer.  kRpLoggingLevel
	defines which events to log, as follows:
		1 - all HTTP events.
		2 - only Normal responses and Forms Processed.
		3 - only Normal Responses.
*/

#define RomPagerLogging			0
#if RomPagerLogging
	#define kRpLogEvents		200
	#define kRpLoggingLevel		3
#endif


/*
	Page Sources
	
	The RomPager HTTP server will serve browsers with HTTP objects (pages,
	graphics, and applets) that originate from four sources.  Objects may be
	stored in ROM if they have been prepared with the Web Application Toolkit.
	They may optionally come from an external file system, user exit routines,
	or a remote Web server.
	
	**** **** **** **** **** **** ****

	File System
		 
	kFileSystemPrefix, RpUnknownUrlsAreFiles, RpRootObjectIsFile,
	kRpRootFilename, RomPagerFileUpload

	If RomPagerFileSystem in AsConfig.h is defined as 1, the code to support
	file system data access functions will be generated.  URLs that have the
	form 'http://<device-address>/XXX/pathname' will be passed directly to
	the file system routines without being looked for in the ROM object list.
	The value of XXX is determined by kFileSystemPrefix which has a default
	value of 'FS'. If RpUnknownUrlsAreFiles is set to 1, URLs that are not
	found in the ROM object list will be passed to the file system routines.

	Normally, the root object resides in ROM and is the first object in the
	first list pointed to by the master object list (see RpSetRomObjectList
	in RpCallBk.h).  The root object can be changed to be a file by setting
	RpRootObjectIsFile to 1 and setting kRpRootFilename to the name of the
	desired root file.

	If RomPagerFileUpload is set to 1, the code to support form based file
	uploading (RFC 1867) will be generated.  This capability can be useful
	for sending flash updates or files to be printed from browsers to the
	device.  HTTP based file transmission can be used in place of TFTP or 
	other mechanisms, and is built into Netscape browsers since Navigator 3.0 
	and Microsoft browsers since Explorer 4.0.  If form based file uploading 
	is not needed, this flag should be set to 0.
	
	If RpFileInsertItem is set to 1, the code to support the insertion of 
	a file into the content of a page using the eRpItemType_File item 
	will be generated.

	**** **** **** **** **** **** ****

	User Exits
		 
	RomPagerUserExit, kUserExitPrefix, RpUnknownUrlsAreCgi,
	RpUserExitAggregate, RpRootObjectIsUserExit
	
	If RomPagerUserExit is defined as 1, the engine will exit to a user
	supplied routine to perform CGI-like functions.  URLs that have the form 
	'http://<device-address>/XXX/pathname' will be passed directly to the 
	user exit function without being looked for in the rom object list.  
	The value of XXX is determined by kUserExitPrefix which has a default 
	value of 'UE'. If RpUnknownUrlsAreCgi is set to 1, URLs that are not 
	found in the rom object list will be passed to the user exit function.

	If RpUserExitAggregate is defined as 1, and the user exit function returns 
	multiple buffers in response to a request, the engine will aggregate these 
	buffers so that the outbound packets are as large as possible. 

	Normally, the root object resides in ROM and is the first object in the
	first list pointed to by the master object list (see RpSetRomObjectList
	in RpCallBk.h).  The root object can be changed to be delivered from
	User Exit by setting RpRootObjectIsUserExit to 1.

	**** **** **** **** **** **** ****

	Remote Host
		 
	RomPagerRemoteHost, kRemoteHostPrefix, RpUnknownUrlsAreRemote,
	RpRhLocalCredentials

	The RomPager Remote Host capability is an optional add-on toolkit
	available under separate license that provides the ability to
	retrieve HTTP objects from a remote Web server.  This code includes
	HTTP client code (to talk to the remote server) and code to redirect
	browser requests.  This capability can be useful for retrieving large
	applets or graphics from a remote host rather than storing them in
	the device.  Browsers will believe that the objects are coming from
	the device, so that Java security issues may be satisfied.
	
	If RomPagerRemoteHost is defined as 1, the code to support remote host
	object retrieval  will be generated.  URLs that have the form 
	'http://<device-address>/XXX/pathname' will be requested directly from 
	the remote host without being looked for in the rom object list.  The 
	value of XXX is determined by kRemoteHostPrefix which has a default 
	value of 'RH'.  If RpUnknownUrlsAreRemote is set to 1, URLs that are not 
	found in the rom object list will be requested from the remote host 
	routines.  The number of simultaneous Remote Host objects that can be 
	retrieved is controlled by the value of kStcpActiveConnections.

	Normally, a security challenge from the Remote Host is passed on to
	the requestor and they are required to enter credentials.  If
	RpRhLocalCredentials is defined as 1, the Remote Host credentials
	are stored locally and used to respond to the security callenge
	from the Remote Host.  In this case, the requestor does not get the
	security challenge.  The Remote Host credentials are set using the
	RpSetRemoteHostAuthInfo routine (see RpCallBk.h).

	**** **** **** **** **** **** ****

	SOAP Requests

	URLs that have the form 
	http://<device-address>/<kUpnpDevicePrefix>/pathname
	will be treated as requests using the Simple Object Access Protocol.
*/

#if RomPagerFileSystem
	#define	kFileSystemPrefix		"FS"		
// +++ _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#if 0
	#define RpUnknownUrlsAreFiles	0
#else
	#define RpUnknownUrlsAreFiles	1
#endif
// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih
	#define RpRootObjectIsFile		0
	#if RpRootObjectIsFile
		#define kRpRootFilename		"index.htm"
	#endif
	#define RomPagerFileUpload		1		/* 	4800 bytes	*/
	#define RpFileInsertItem		1
#endif

// +++ _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#if 0
#define RomPagerUserExit			1		/* 	575 bytes	*/
#else
#define RomPagerUserExit			0		/* 	575 bytes	*/
#endif
// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih
	#if RomPagerUserExit
		#define RpRootObjectIsUserExit	0
	#endif

#if RomPagerUserExit
	#define	kUserExitPrefix			"UE"		
	#define RpUnknownUrlsAreCgi		0
	#define RpUserExitAggregate		0
#endif

// +++ _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#if 0
#define RomPagerRemoteHost			0		/* 	2500 bytes	*/
#else
#define RomPagerRemoteHost			1		/* 	2500 bytes	*/
#endif
// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih

#if RomPagerRemoteHost
// +++ _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#if 0
	#define kRemoteHostWorkBufferSize	(512)
	#define	kRemoteHostPrefix		"RH"		
	#define RpUnknownUrlsAreRemote	1
	#define RpRemoteHostMulti		0
	#define RpRemoteHostCount		20
	#define RpRhLocalCredentials	0
	#if (kStcpActiveConnections < kRpNumberOfHttpRequests)
		#undef kStcpActiveConnections
		#define kStcpActiveConnections	kRpNumberOfHttpRequests
	#endif
#else
	#define kRemoteHostWorkBufferSize	(512)
	#define	kRemoteHostPrefix		"RH"		
	#define RpUnknownUrlsAreRemote	0
	#define RpRemoteHostMulti		1
	#define RpRemoteHostCount		32
	#define RpRhLocalCredentials	1
	#if (kStcpActiveConnections < kRpNumberOfHttpRequests)
		#undef kStcpActiveConnections
		#define kStcpActiveConnections	kRpNumberOfHttpRequests
	#endif
#endif
// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#endif

#define	kUpnpDevicePrefix			"UD"


/*
	Web Application Toolkit
	
	The main source of HTTP objects served by the RomPager engine are 
	objects prepared with the Web Application Toolkit using the PageBuilder 
	compiler to create ROM images.  The various flags below are used to 
	control the runtime size of the code that supports the management 
	application, the object preparation for serving, and handling of 
	submitted forms.  The bulk of the runtime support for the Web 
	Application Toolkit is in the modules RpHtml.c and RpForm.c.

	**** **** **** **** **** **** ****

	RomPagerForms, RpHtmlCheckbox, RpHtmlRadio, RpHtmlTextArea, 
	RpHtmlTextAreaBuf, RpHtmlSelectVariable, RpHtmlSelectVarValue, 
	RpHtmlSelectFixedSingle, RpHtmlSelectFixedMulti, RpHtmlRadioDynamic, 
	RpHtmlCheckboxDynamic, RpHtmlTextFormDynamic, RpHtmlNamedSubmit

	These compiler flags are used to control the code generation for the 
	forms handling routines and HTML generation routines.  If no forms are 
	used at all, RomPagerForms should be set to 0 which will save about 7900 
	bytes. This would be the case if all the pages are display pages with no 
	ability to set variables.  Normally, RomPagerForms will be set to 1.  
	The RpHtmlXXXX flags control code generation for different HTML form 
	elements. After your management application is complete, you can reduce 
	memory usage by disabling the flags for the individual form elements 
	that are unused.

	**** **** **** **** **** **** ****

	RomPagerQueryIndex, kRpIndexCharacter
	
	If RomPagerQueryIndex is defined as 1, the code to support the page 
	elements associated with indexed display and query processing will 
	be enabled.
	
	The kRpIndexCharacter definition is used to separate the index information
	from the rest of an HTML name. 

	**** **** **** **** **** **** ****

	kRpHexShiftRight

	The kRpHexShiftRight is used to determine how to treat hex conversions 
	when there are fewer than expected input characters. If kRpHexShiftRight 
	is defined as 0, the conversion left justifies the input and null fills 
	on the right. If kRpHexShiftRight is defined as 1, the conversion right 
	justifies the input and null fills on the left.

	**** **** **** **** **** **** ****

	RomPagerJavaScript
	
	If RomPagerJavaScript is defined as 1, the data structures for <INPUT> 
	elements include an extra char pointer to the JavaScript code that can
	appear inside these elements.  Extra code will be generated in RpHtml.c
	to send out the JavaScript code.

	**** **** **** **** **** **** ****

	RpHtmlAlignmentUsesPreTag
	
	Some HTML page generating tools use the <PRE> tag for alignment.  Since 
	this tag means that all spaces and newlines are treated exactly, the 
	RomPager internally generated HTML must be aware of this condition. If 
	the RpHtmlAlignmentUsesPreTag is defined as 0, the internally generated 
	HTML will contain newline characters that are used to make the generated 
	HTML more readable. If the RpHtmlAlignmentUsesPreTag is defined as 1, 
	the internally generated HTML will not include extra newline characters 
	that could conflict with the <PRE> tag.
*/

#define RpHtmlBufferDisplay				1		/*  xxx bytes  */

#define RomPagerForms					1		/*  7900 bytes  */			

#if RomPagerForms			
	#define RpHtmlCheckbox				1		/*  250 bytes  */
	#define RpHtmlRadio					1		/*  400 bytes  */	
	#define RpHtmlTextArea				1		/*  275 bytes  */
	#define RpHtmlTextAreaBuf			1		/*  300 bytes  */
	#define RpHtmlNamedSubmit			1		/*  350 bytes  */
	#define RpHtmlFormImage				1		/*	xxx bytes  */
	/*
		These 7 item types require RomPagerQueryIndex.
	*/
	#define RpHtmlSelectVariable		1		/*  600 bytes  */	
	#define RpHtmlSelectVarValue		1		/*  600 bytes  */	
	#define RpHtmlSelectFixedSingle		1		/*  600 bytes  */
	#define RpHtmlSelectFixedMulti		1		/*  675 bytes  */
	#define RpHtmlRadioDynamic			1		/*  325 bytes  */
	#define RpHtmlCheckboxDynamic		1		/*  400 bytes  */
	#define RpHtmlTextFormDynamic		1		/*  100 bytes  */
#endif


#define RomPagerQueryIndex				1		/*  1075 bytes  */
#define kRpIndexCharacter				'?'

#define kRpHexShiftRight 				0		/*  75 bytes  */

#define RomPagerJavaScript				1		/*  525 bytes  */

// +++ _Alphanetworks_Patch_, 10/16/2003, jacob_shih 
// to prevent RomPager generates the newline for some javascript that
// writes the html tags.
#if 0
#define RpHtmlAlignmentUsesPreTag		0
#else
#define RpHtmlAlignmentUsesPreTag		1
#endif
// --- _Alphanetworks_Patch_, 10/16/2003, jacob_shih

/*
	Miscellaneous Engine Functions
 
	RomPagerDynamicBoxName
	
	The name of the device that is used in error message displays defaults
	to "RomPager server" the value of kRpBoxName.  If you want to change
	the device name, there are two approaches.  To change it at compile
	time, change the value of kRpBoxName.  If you have a family of devices
	and want to be able to have the device determine a name at startup
	from configuration data, you need to set the RomPagerDynamicBoxName flag
	to 1 and flesh out the RpInitializeBox routine in RpUser.c with a call
	to retrieve the device name and store it in the RomPager data structure.
	
	RomPagerDelayedFunctions
	
	If RomPagerDelayedFunctions is defined as 1, the code to support 
	delayed page data functions will be generated.  

	RpUserCookies
	
	If RpUserCookies is defined as 1, the code to support saving and 
	retrieving arbitrary user data in the RomPager structures will be 
	generated.  

	RomPagerUrlState
	
	If RomPagerUrlState is defined as 1, the code to allow state
	passing in the URL will be generated.  URLs that have the form 
	'http://<device-address>/XXX/SSSSS/pathname' will be examined to
	see if a state should be stored away. If the 'XXX' matches the
	value set by kUrlState, then the 'SSSSS' is stored in internal
	structures that can be accessed by callback routines.  The rest
	of the URL is passed on for further examination.  

*/

#define RomPagerDynamicBoxName		0
#define kRpBoxName					"RomPager server"

#define RomPagerDelayedFunctions	1		/*  200 bytes  */

#define RpUserCookies				1		/*  125 bytes  */

#define RomPagerUrlState			0		/*  325 bytes  */
#if RomPagerUrlState
	#define	kUrlState				"US"		
#endif


/* 
	Soft Pages
	
	RomPagerSoftPages, RomPagerSoftPageCompression, RomPagerSoftPageFunction, 
	RomPagerSoftPageProcessUserExit, kSpParseBufferSize
	
	If RomPagerSoftPages is defined as 1, the code to support the parsing 
	of HTML to process the Soft Page tags will be compiled. 
	
	Soft Pages can be compressed with the PageBuilder compiler using the System 
	and User Phrase Dictionaries. If they are, the code to expand them should 
	be enabled by defining RomPagerSoftPageCompression as 1. 
	
    If the RomPagerSoftPageIndexedString flag is set, a user exit routine is
    used to provide the value string for HTML form items and the <rpisNN> tag.
    The SpGetIndexedString() function will be called by the SoftPages code
    which passes the integer (NN) and must be provided by the user.

	If RomPagerSoftPageFunction is defined as 1, the code to support the <rpdf> 
	tag in a Soft Page is supported and the supporting user function, 
	RpSoftPageFunction(), must be provided. 
	
	If RomPagerSoftPageProcessUserExit is defined as 1, the code to enable Soft 
	Pages from User Exit page is enabled. 
	
	The HTML parsing code requires that the complete page be read into a buffer 
	and kSpParseBufferSize must be large enough to contain the largest Soft 
	Page defined in a device.
*/

#define RomPagerSoftPages					0
#if RomPagerSoftPages
	#define RomPagerSoftPageCompression		1
    #define RomPagerSoftPageIndexedString   1
	#define RomPagerSoftPageFunction		1
	#define RomPagerSoftPageProcessUserExit	1
	#define kSpParseBufferSize			 2000
#endif


/* 
	Internet Printing Protocol (IPP)
	
	RomPagerIpp, kIppNumberOfPrinters
	
	If RomPagerIpp is defined as 1, the code to support Internet Printing 
	Protocol requests will be compiled. IPP requests are passed through
	the HTTP engine to an IPP parser. IPP support requires HTTP 1.1 support.

	The IPP printer name is the name that identifies the printer in the URL.
	In other words, URLs that need to be passed to the IPP parser should
	have the form "<hostname>/xxxxx/yyyyy" where "xxxxx" is the printer
	name and "yyyyy" is an optional job id.

	There may be one or more printer names.  The number of printer names
	is determined by the kIppNumberOfPrinters value.

	IPP printer names and security access codes are set up using the
	RpSetIppPrinterName callback routine at runtime.  This routine
	should be called at device initialization time from the
	RpInitializeBox routine in RpUser.c.
*/

#define RomPagerIpp					0		/*  950 bytes  */
#if RomPagerIpp
	#define	kIppNumberOfPrinters	1
#endif


/* 
	RomPager Demonstration Pages
	
	RomPagerDemo, RomPagerJavaAppletDemo, RpDictionariesDemo, 
	RomPagerChassisDemo
	
	The RomPager package ships with a number of demonstration pages
	that show various capabilities of the product.  In a production
	environment, these variables should all be defined as 0.  
	
	If RomPagerDemo is defined as 1, a number of other settings will
	be overridden so that the demo pages will work.
	
	If RomPagerJavaAppletDemo is defined as 1, the pages and classes that 
	demonstrate the RomPager Java Graphlets will be included. The RomPager 
	Java Graphlets are available under separate license as a toolkit that 
	includes the full Java source code, documentation, compiled byte codes 
	as well as the RomPager internal format.

	If RpDictionariesDemo is defined as 1, the demo routines for the
	international phrase dictionaries will be included.  

	If RomPagerChassisDemo is defined as 1, the demo routines and pages
	for a graphical Lan Switch chassis will be included.  
*/

#define RomPagerDemo				1
#define RomPagerJavaAppletDemo		1
#define RpDictionariesDemo			1
#define RomPagerChassisDemo			1


#if RomPagerDemo
	/*
		The demo pages need the following flags to be set
	*/
	#undef RomPagerImageMapping
	#define RomPagerImageMapping		1
	#undef RomPagerClientPull
	#define RomPagerClientPull			1
	#undef RomPagerServerPush
	#define RomPagerServerPush			1
	#undef RomPagerSecurity
	#define RomPagerSecurity			1
	#undef RomPagerExternalPassword
	#define RomPagerExternalPassword	0
	#undef kRpNumberOfRealms
	#define kRpNumberOfRealms			8
	#undef kRpNumberOfUsers
	#define kRpNumberOfUsers			8
	#undef kPasswordSessionTimeout
	#define kPasswordSessionTimeout		(5*60)
	#if RomPop || RomPopBasic || RomWebClient || RomDns
		#undef RomPagerDelayedFunctions
		#define RomPagerDelayedFunctions	1
	#endif
#endif


#endif	/* _RPCONFIG_ */
