/*
 *	File:		AsCheck.h
 *
 *	Contains:	Consistency checks for configuration parameters
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
 *		09/22/03	bva		reworked RomPlug checks
 *		09/19/03	pjr		add RpRhLocalCredentials check
 *		09/18/03	amp		add RSA key length check
 *      08/25/03    amp     update Media Renderer and Media Server checks
 *      06/16/03    amp     add Media Server check
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 * * * * Release 4.20  * * *
 *      02/13/03    bva     update RomPagerSecure check
 *      12/18/02    amp     add RomCliSecure checks
 *		11/07/02	pjr		change kWcWorkBufferLength check
 *		11/06/02	pjr		add WcPipelining check
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *		07/18/02	pjr		add check for kWcMaxCacheFiles > 9999
 *		07/01/02	pjr		add WcCaching checks
 * * * * Release 4.10  * * *
 *		06/12/02	bva		add RomCliInitialPlayback check
 * * * * Release 4.07  * * *
 * * * * Release 4.04  * * *
 *		11/28/01	pjr		clean up
 * * * * Release 4.03  * * *
 *		10/16/01	amp		add RomUpnp checks
 *		10/09/01	amp		allow !RomPagerCalendarTime with RomPagerSecure
 * * * * Release 4.02  * * *
 * * * * Release 4.01  * * *
 *		08/25/01	pjr		add more RomCli checks
 * * * * Release 4.00  * * *
 *		07/28/01	pjr		add kRcMaxLineBufferLength check
 *		07/25/01	pjr		add more RomCli checks
 *		07/09/01	pjr		add kRcNumberOfCliSessions check
 *		04/19/01	bva		add RomCli check
 *		12/11/00	pjr		add more RomPagerQueryIndex checks
 *		07/25/00	amp		Support SSL/TLS
 *		05/19/00	pjr		add kWcWorkBufferLength check
 *		02/14/00	pjr		digest security requires HTTP 1.1
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 *		01/07/00	bva		remove Keep-Alive check
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 *		02/26/99	bva		change connection setup definitions
 *		02/06/99	bva		RpCheck.h -> AsCheck.h
 *		01/10/99	pjr		make common for all RomPager products
 * * * * Release 2.2 * * * *
 *		11/09/98	bva		remove RpUseRpAlloc
 *		10/12/98	bva		remove IPP check for HTTP 1.1
 * * * * Release 2.1 * * * *
 *		04/02/98	bva		created from RpConfig.h
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_AS_CHECK_
#define	_AS_CHECK_

/*
	The consistency checks.
*/

#if (RomPagerServer && RomPagerBasic)
	#error Only one of RomPagerServer or RomPagerBasic may be enabled
#endif

#if (RomPagerServer || RomPagerBasic)

#if (kStcpNumberOfConnections < \
			(kStcpActiveConnections + kRpNumberOfHttpRequests))
	#error Not enough TCP connections
#endif

#endif	/* RomPagerServer || RomPagerBasic */

#if (RmFileOptions && !RomPagerFileSystem)
	#error Mail attachments and File Message Body require the file system
#endif

#if (RomPop && !RomPagerFileSystem)
	#error RomPop requires the file system
#endif

#if RomPagerServer || RomPagerBasic || RmFileOptions
#if (kRpNumberOfHttpRequests == 0 && !RomPagerDynamicRequestBlocks)
	#error No request control blocks defined.
#endif
#endif

#if RomPagerServer

#if (RpUnknownUrlsAreFiles && (RpUnknownUrlsAreCgi || RpUnknownUrlsAreRemote))
	#error Invalid Unknown URL handler.
#endif

#if (RpUnknownUrlsAreCgi && RpUnknownUrlsAreRemote)
	#error Invalid Unknown URL handler.
#endif

#if (RomPagerDemo && RomPagerDynamicGlobals)
	#error The demo pages require static globals.
#endif

#if (RpHtmlCheckboxDynamic && !RomPagerQueryIndex)
	#error Need RomPagerQueryIndex set if RpHtmlCheckboxDynamic is set
#endif

#if (RpHtmlRadioDynamic && !RomPagerQueryIndex)
	#error Need RomPagerQueryIndex set if RpHtmlRadioDynamic is set
#endif

#if (RpHtmlSelectFixedMulti && !RomPagerQueryIndex)
	#error Need RomPagerQueryIndex set if RpHtmlSelectFixedMulti is set
#endif

#if (RpHtmlSelectFixedSingle && !RomPagerQueryIndex)
	#error Need RomPagerQueryIndex set if RpHtmlSelectFixedSingle is set
#endif

#if (RpHtmlSelectVariable && !RomPagerQueryIndex)
	#error Need RomPagerQueryIndex set if RpHtmlSelectVariable is set
#endif

#if (RpHtmlSelectVarValue && !RomPagerQueryIndex)
	#error Need RomPagerQueryIndex set if RpHtmlSelectVarValue is set
#endif

#if (RpHtmlTextFormDynamic && !RomPagerQueryIndex)
	#error Need RomPagerQueryIndex set if RpHtmlTextFormDynamic is set
#endif

#if (RomPagerFileUpload && !RomPagerForms)
	#error File Uploads require HTML Forms to specify the file to upload
#endif

#if (RomPagerPutMethod && !RomPagerFileSystem)
	#error The HTTP 1.1 PUT method requires the file system
#endif

#if (RomPagerSecurityDigest && !RomPagerHttpOneDotOne)
	#error RomPagerSecurityDigest requires RomPagerHttpOneDotOne
#endif

#if (RpRhLocalCredentials && !RomPagerSecurity)
	#error RpRhLocalCredentials requires RomPagerSecurity
#endif

#endif	/* RomPagerServer */

#if RomWebClient

#if WcHttpCookies
	#if (kWcWorkBufferLength < \
				(kWcMaxCookieNameLength + kWcMaxCookieValueLength + 3))
		#error kWcWorkBufferLength too small
	#endif
#endif

#if WcCaching
	#if !RomPagerFileSystem
		#error WcCaching requires the file system
	#endif
	#if !RomPagerCalendarTime
	#error WcCaching requires RomPagerCalendarTime
	#endif
	#if (kWcMaxCacheFiles > 9999)
		#error kWcMaxCacheFiles must be <= 9999
	#endif
#endif

#if WcPipelining
	#if !WcHttpOneDotOne
		#error WcPipelining requires WcHttpOneDotOne
	#endif
#endif

#endif	/* RomWebClient */

#if RomPagerSecure && !(RomPagerServer || RomPagerBasic)
	#error RomPagerSecure requires a RomPagerServer
#endif

#if RomWebClientSecure && !RomWebClient
	#error RomWebClientSecure requires RomWebClient
#endif

#if (RomPagerSecure || RomWebClientSecure) && \
		(kDeviceCertificateKeyLengthBits > kMaxRSAKeyLengthBits)
#error kDeviceCertificateKeyLengthBits must be <= kMaxRSAKeyLengthBits
#endif

#if RomCli

#if !RomTelnet && !RomConsole && !RomCliSecure
    #error RomCli requires Telnet or Console access or RomCliSecure
#endif

#if (kRcNumberOfCliSessions == 0)
	#error kRcNumberOfCliSessions must be > 0 for RomCli
#endif

#if ((RomCliRecord || RomCliPlayback) && !RomPagerFileSystem)
	#error RomCliRecord and RomCliPlayback require the file system
#endif

#if (RomCliPlayback && (kRcPlaybackFileBufferSize > kRcMaxLineBufferLength))
	#error kRcMaxLineBufferLength must be >= kRcPlaybackFileBufferSize
#endif

#if RomConsole && (kRcNumberOfSerialPortSessions == 0)
	#error kRcNumberOfSerialPortSessions must be > 0 for RomConsole
#endif

#if RomCliInitialPlayback && !RomCliPlayback
	#error RomCliInitialPlayback requires RomCliPlayback
#endif

#if RomCliSecure && (kShNumberOfSessions == 0)
	#error kShNumberOfSessions must be > 0 for RomCliSecure
#endif
#endif	/* RomCli */

#if !RomCli && RomCliSecure
    #error RomCliSecure requires RomCli
#endif

#if (AsServerPollReduction && AllegroMultiTasking)
	#error AsServerPollReduction should only be used in a non-multitasking environment
#endif

#if RomPlugAdvanced

	#if	!RomPagerServer
		#error RomPlugAdvanced needs RomPagerServer
	#endif

	#if RuMediaRenderer
		#if kRxMaxArrayIndices < 3
		#error kRxMaxArrayIndices must be at least 3
		#endif

		#if kRxMaxNamespaces < 3
		#error kRxMaxNamespaces must be at least 3
		#endif

		#if kRxMaxStructureLevels < 4
		#error kRxMaxStructureLevels must be at least 4
		#endif

		#if kHtmlMemorySize < 1400
		#error kHtmlMemorySize must be at least 1400
		#endif
	#endif	/* RuMediaRenderer */

	#if RuMediaServer
		#if  (kMsMaxResourceTransferBlocks + 2 > kWcNumberOfHttpSessions)
			#error Increase kWcNumberOfHttpSessions to kMsMaxResourceTransferBlocks+2
		#endif
		#if !RomPagerDelayedFunctions
			#error Media Server requires RomPagerDelayedFunctions=1
		#endif

		#if kRxMaxArrayIndices < 5
		#error kRxMaxArrayIndices must be at least 5
		#endif

		#if kRxMaxNamespaces < 3
		#error kRxMaxNamespaces must be at least 3
		#endif

		#if kRxMaxStructureLevels < 4
		#error kRxMaxStructureLevels must be at least 4
		#endif
	#endif	/* RuMediaServer */

#endif	/* RomPlugAdvanced */

#if RomPlugControl

	#if	!RomPagerServer
		#error RomPlugControl needs RomPagerServer
	#endif

	/*
		RomPlugControl requires WebClient.
	*/
	#if RomWebClient
		/*
			RomPlugControl requires HTTP 1.1.
		*/
		#if !WcHttpOneDotOne
			#error RomPlugControl needs HTTP 1.1
		#endif

		/*
			RomPlugControl requires WcCancelRequest.
		*/
		#if !WcRequestCancel
			#error RomPlugControl needs WcRequestCancel
		#endif	/* !WcRequestCancel */

		/*
			WebClient must support the number of requests needed
			by RomPlugControl.
		*/
		#if kCpMaxWebRequests > kWcNumberOfHttpRequests
			#error kCpMaxWebRequests too big
		#endif
	#else
		#error RomPlugControl needs RomWebClient
	#endif	/* RomWebClient */
#endif	/* RomPlugControl */

#endif	/* _AS_CHECK_ */
