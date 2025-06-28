/*
 *	File:		RpExtern.h
 *
 *	Contains:	RomPager Web server external definitions 
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
 *      10/04/02    amp     update for new RomPagerSecure
 * * * * Release 4.12  * * *
 * * * * Release 4.03  * * *
 *		10/16/01	rhb		include RpsProto.h if RomPagerSecure 
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		02/21/01	bva		move dictionary includes to AsEngine.h
 *		01/23/01	rhb		include RpSpProt.h if Soft Pages is enabled
 *		12/18/00	pjr		add RomPagerServer conditional
 * * * * Release 3.0 * * * *
 *		02/06/99	bva		most includes moved to AsExtern.h
 *		01/24/99	pjr		added AsChkDef.h, AsObjct.h
 *		01/15/99	pjr		rework
 *		01/11/99	pjr		only include dictionary headers for RomPagerServer
 *		01/10/99	pjr		add AsConfig.h and McConfig.h
 * * * * Release 2.2 * * * *
 *		11/09/98	bva		added RpStdLib.h
 *		07/18/98	bva		added RpTarget.h
 *		07/01/98	pjr		include RpCgi.h if SlavePager
 * * * * Release 2.1 * * * *
 *		04/04/98	bva		added PrConfig.h
 *		04/02/98	bva		added RmConfig.h, RpCheck.h
 *		01/13/98	rhb		added PrProtos.h
 * * * * Release 2.0 * * * *
 *		11/03/97	rhb		added RpIpp.h
 *		10/28/97	bva		added RpCgi.h
 * * * * Release 1.6 * * * *
 *		03/03/97	rhb		added RpUsrDct.h
 *		01/24/97	rhb		added RpMimes.h
 * * * * Release 1.5 * * * *
 *		11/01/96	bva		created from RpPages.h
 * * * * Release 1.4 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_RP_EXTERN_
#define	_RP_EXTERN_

#include "AsConfig.h"

#if RomPagerServer

#include "AsTypes.h"
#include "AsStdLib.h"
#include "AsMimes.h"
#include "AsConvrt.h"

#include "RpConfig.h"
#include "RpPgFunc.h"
#include "RpPages.h"
#include "RpCallBk.h"

#if RomPagerIpp
#include "RpIpp.h"
#endif

#if RomPagerSoftPages
#include "RpSpProt.h"
#endif

#endif /* RomPagerServer */

#endif /* _RP_EXTERN_ */
