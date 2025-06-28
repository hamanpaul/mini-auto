/*
 *	File:		AsExtern.h
 *
 *	Contains:	RomPager product family external definitions 
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
 *		09/22/03	bva		move RomXML check to RpUpSoap.h
 *		08/27/03	rhb		rework VaConfig.h and its conditionals
 *		07/08/03	bva		rework AsVariableAccess ifdefs
 *		06/30/03	amp		add resource locking
 *		06/26/03	nam		add prototypes for floating point conversions
 *		06/23/03	amp		add Media Server support
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 *		03/17/03	amp		add Media Renderer support
 * * * * Release 4.20  * * *
 *		12/17/02	bva		include AsVarAcc.h for RomXml
 *		12/08/02	amp		add RomCliSecure
 *		11/09/02	rhb		include AsTypes.h before AsConvrt.h
 *		10/13/02	bva		add AsTskMsg.h
 *		10/04/02	amp		support new RomPagerSecure
 * * * * Release 4.10  * * *
 *		06/12/02	bva		add RomUpnpAdvanced check for RomXml
 * * * * Release 4.07  * * *
 * * * * Release 4.05  * * *
 *		01/05/02	bva		rearrange RomTime includes
 * * * * Release 4.04  * * *
 * * * * Release 4.01  * * *
 *		08/29/01	rhb		include RxProtos.h before RxMetObj.h
 * * * * Release 4.00  * * *
 *		06/29/01	amp		add RomUpnpControl
 *		06/29/01	amp		add UPnp SOAP support; move UPnP includes
 *		05/22/01	amp		add C++ compatibility
 *		05/07/01	dts		add RomTime support
 *		02/20/01	rhb		remove RxObject.h
 *		02/13/01	bva		add RcProtos.h
 *		02/12/01	rhb		include AsVarAcc.h
 *		02/09/01	bva/dts	add RomConsole support
 *		12/18/00	pjr		add AsConvrt.h
 *		09/01/00	bva/dts	add RomTelnet/RomCli support
 *		07/31/00	bva		add RomUpnp support
 *		05/22/00	pjr		move XML includes up, before Web Client includes
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 *		01/13/00	amp		add RomMailerBasic and RomPopBasic files
 * * * * Release 3.10  * * *
 * * * * Release 3.05  * * *
 *		08/23/99	rhb		add RomXML files
 *		06/14/99	bva		remove SlavePager references
 * * * * Release 3.04  * * *
 * * * * Release 3.0 * * * *
 *		02/26/99	bva		change connection setup definitions
 *		02/14/99	bva		moved definitions from RsConfig.h
 *		02/06/99	bva		created from RpExtern.h
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_AS_EXTERN_
#define	_AS_EXTERN_

#ifdef __cplusplus
extern "C" {
#endif

#include "AsTarget.h"
#include "AsConfig.h"
#include "VaConfig.h"
#include "AsTypes.h"
#include "AsStdLib.h"
#include "AsMimes.h"

#if AllegroMultiTasking
//#include "AsTskMsg.h"
#endif

#include "Stcp.h"

#if RomDns || RomPlug || RomPlugControl || RomTime
#include "Sudp.h"
#endif

#include "AsConvrt.h"

#if RomXml
#include "RxConfig.h"
#include "RxProtos.h"
#include "RxMetObj.h"
#endif

#if AsUseFloatingPoint
double		AsStringToFloat(char *theStringPtr);
Unsigned16	AsFloatToString(double theFloatValue, char *theStringPtr);
#endif

#if AsVariableAccess || RomPagerServer || RxConversions
#include "AsVarAcc.h"
#endif

#if AsVariableAccess || RomPagerServer 
#include "RpDict.h"
#include "RpUsrDct.h"
#endif

#if AsResourceLocks
#include "AsLock.h"
#endif

#if RomPagerServer
#include "RpExtern.h"
#elif RomPagerBasic || RomMailer
#include "RpObject.h"
#endif

#if RomPagerBasic
#define RomPagerUserExit			1
#define RomPagerQueryIndex			0
#include "RsConfig.h"
#include "RsCallBk.h"
#endif

#if RomMailer
#include "RmConfig.h"
#include "RmProtos.h"
#endif

#if RomMailerBasic
#include "ScConfig.h"
#include "ScProtos.h"
#endif

#if RomPop
#include "PrConfig.h"
#include "PrProtos.h"
#endif

#if RomPopBasic
#include "PcConfig.h"
#include "PcProtos.h"
#endif

#if RomPlug
#include "RuConfig.h"
#if RomPlugAdvanced
	#include "RpUpSoap.h"
#endif	/* RomPlugAdvanced */
#include "RuProtos.h"
#if RomPlugAdvanced
#if RuMediaRenderer
	#include "MrConfig.h"
	#include "MrProtos.h"
#endif	/* RuMediaRenderer */
#if RuMediaServer
	#include "MsConfig.h"
	#include "MsProtos.h"
#endif	/* RuMediaServer */
#endif	/* RomPlugAdvanced */
#endif	/* RomPlug */

#if RomPlugControl
#include "CpConfig.h"
#include "CpProtos.h"
#endif

#if RomWebClient
#include "WcConfig.h"
#include "WcProtos.h"
#endif

#if RomDns
#include "RdConfig.h"
#include "RdProtos.h"
#endif

#if RomTime
#include "TmConfig.h"
#include "TmProtos.h"
#endif

#if RomPagerUserExit
#include "RpCgi.h"
#endif

#if RomTelnet
#include "TnConfig.h"
#endif

#if RomCliSecure
#include "ShConfig.h"
#include "ShProtos.h"
#endif

#if RomConsole
#include "CsConfig.h"
#endif

#if RomCli
#include "RcConfig.h"
#include "RcProtos.h"
#include "RcParse.h"
#endif

#if RomPagerSecure || RomWebClientSecure
#if 0
#include "A1Config.h"
#include "A1Protos.h"
#endif
#endif	/* RomPagerSecure || RomWebClientSecure */

#include "AsChkDef.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AS_EXTERN_ */
