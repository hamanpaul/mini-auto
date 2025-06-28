/*
 *	File:		RpUser.c
 *
 *	Contains:	These routines are used for user customization of the RomPager
 *				server for a given operating environment
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
 *		02/03/03	rhb		support enums for access through Variable Access
 * * * * Release 4.20  * * *
 *		10/04/02    amp     change realm3 and realm4 to use TLS
 * * * * Release 4.12  * * *
 * * * * Release 4.10  * * *
 *		06/08/02	bva		remove RomPagerSlaveIdentity
 * * * * Release 4.07  * * *
 *		02/27/02	bva		change ifdef for AsSetVarAccessItemList call
 *		02/11/02	rhb		set up default RomPagerSecure access realm
 *		02/04/02	pjr		only call RpInitUserData for RomPagerServer
 * * * * Release 4.06  * * *
 * * * * Release 4.04  * * *
 *		11/26/01	pjr		fix conditional
 * * * * Release 4.00  * * *
 *		06/19/01	pjr		initialize the variable access item list for RomCli
 *		04/19/01	bva		add RpHttpPutComplete
 *		02/16/01	rhb		rename gRpMasterVariableItemList to
 *								gAsMasterVariableItemList
 *		02/09/01	rhb		Variable Access Lists are arrays
 *		12/15/00	pjr		always call AsSetVarAccessItemList for RomPagerServer
 *		12/12/00	pjr		add AsSetVarAccessItemList call
 *		10/06/00	pjr		rework IPP printer setup
 *		09/06/00	pjr		change default user configuration
 *		08/31/00	pjr		change initialization for the new security model
 *		08/14/00	rhb		gRpMasterObjectList's extern is here, not RpPages.h
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.10  * * *
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 *		12/31/98	pjr		move date and time routines to RpDate.c
 *		12/23/98	bva		add documentation to RpBuildDateString
 * * * * Release 2.2 * * * *
 *		11/14/98	bva		RpGetRomTimeInSeconds -> RpGetMonthDayYearInSeconds
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *							remove RpFree, RpAlloc routines
 *		10/20/98	bva		add realm initialization for PUT support
 *		07/18/98	bva		add time support for target OS environments
 *		06/17/98	bva		add initialization of slave identity
 * * * * Release 2.1 * * * *
 *		04/20/98	bva		moved include of time.h
 *		04/16/98	bva		use memset in RpAlloc
 *		02/16/98	rhb		compile RpAlloc and RpFree on RpUseRpAlloc
 *		02/13/98	rhb		change parameter to RpCatUnsigned32ToString
 * * * * Release 2.0 * * * *
 *		12/08/97	pjr		change RpBuildDateString to generate a 2 digit
 *							day number.  some servers do not parse a single
 *							digit day number in a date string properly.
 *		09/29/97	pjr		rework RpInitializeBox for External Password and
 *							cases where less than 8 realms are used.
 *		08/07/97	bva		add RmSetMailGlobals call to RpInitializeBox
 *		07/12/97	bva		move config consistency checks to RpConfig.h
 *							add IPP realm initialization
 * * * * Release 1.6 * * * *
 *		04/20/97	pjr		add digest authentication support
 *		04/04/97	bva		cleanup warnings
 *		03/20/97	bva		add RpUserServerPushExit
 *		12/12/96	rhb		use Jan 1, 1901 as start for date calculations
 *								and do date calculations better
 * * * * Release 1.5 * * * *
 *		09/24/96	rhb		support dynamically allocated engine data
 *		09/24/96	bva		add conditional compile flags for security
 * * * * Release 1.4 * * * *
 * * * * Release 1.3 * * * *
 *		07/03/96	bva		add dynamic box name support
 *		06/21/96	bva		cleaned up Allegro test code
 *		06/13/96	bva		added CRLF to end of RpBuildDateString to save
 *							calls in BuildDateHeaders
 * * * * Release 1.2 * * * *
 *		06/01/96	bva		added RomPagerUseStandardTime and internal
 *								date/time routines
 * * * * Release 1.1 * * * *
 * * * * Release 1.0 * * * *
 *		01/10/96	bva		created
 *
 *	To Do:
 */

#include "AsEngine.h"

#if 0   /*Elsa modify for web*/
#ifndef WIN32
#include <stack/inc/st_sys.h>
#endif
#endif

#if RomPagerServer
extern rpObjectDescPtrPtr	gRpMasterObjectList[];
#endif

#if AsVariableAccess
extern asVarAccessItemPtr	gAsMasterVariableItemList[];
#endif


#if RomPagerServer || RomCli
/*
	RpInitializeBox

	This routine is called once at startup to initialize any box specific
	variables.
*/

extern void dlk_GetDeviceType(char *); // add by Olivia Liou, 10/30/2001, 10/30/2001
#define  GET_NULL   '\0'
void RpInitializeBox(rpDataPtr theDataPtr) {

#if 0   /*Elsa modify for web*/
#ifndef WIN32
char                         UserN[MIB_CST_USER_NAME_LEN+1];
char                         PassW[MIB_CST_USER_NAME_LEN+1];
char                         DUser[MIB_CST_USER_NAME_LEN+1];
	char    DPass[MIB_CST_USER_NAME_LEN+1];
	int 	usercount;
ST_USER_ACCOUNT_T  account_item ;

// ---- Add by Olivia Liou 10/25/2001 , for DEVICE_NAME------------------
char						deviceType[128];

	dlk_GetDeviceType(deviceType);
// ----- Add over -------------------------------------------------------

#endif /*nWIN32 */
#else
char						deviceType[128];
#endif

#if RomPagerServer
	/*
		Set up the item list for pages and images.
	*/
	RpSetRomObjectList(theDataPtr, gRpMasterObjectList);
#endif

#if AsVariableAccess
	/*
		Set up the item list for variable access.
	*/
	AsSetVarAccessItemList(theDataPtr, gAsMasterVariableItemList);
#endif

#if RomPagerServer
#if RomPagerDynamicBoxName
	#error "Need initialization of theDataPtr->fBoxNameText"
#else
	RP_STRCPY(theDataPtr->fBoxNameText, kRpBoxName);
#endif
#endif	/* RomPagerServer */

#if RomPagerServer && RomPagerSecurity
	/*
		Initialize the security table.
	*/

#if (kRpNumberOfRealms > kMaxNumberOfRealms)
	#error "kRpNumberOfRealms is too high"
#endif

	/*
		Set Realm Locking off.
	*/
	RpSetRealmLocking(theDataPtr, False);

/*******************************************************************************
	Realm set up - set up the realm names and security levels.
*******************************************************************************/
	/*
		Realm 1.
	*/
	RpSetRealmName(theDataPtr, 0, deviceType);
	RpSetSecurityLevel(theDataPtr, 0, eRpSecurity_PasswordOnly);

	/*
		Realm 2.
	*/
#if (kRpNumberOfRealms > 1)
	RpSetRealmName(theDataPtr, 1, "Realm2");
#if RomPagerSecurityDigest
	RpSetSecurityLevel(theDataPtr, 1, eRpSecurity_DigestPasswordOnly);
#elif RomPagerSecure
	RpSetSecurityLevel(theDataPtr, 1, eRpSecurity_SecureSocketPasswordOnly);
#else
	RpSetSecurityLevel(theDataPtr, 1, eRpSecurity_PasswordOnly);
#endif
#endif

	/*
		Realm 3.
	*/
#if (kRpNumberOfRealms > 2)
	RpSetRealmName(theDataPtr, 2, "Realm3");
#if RomPagerSecure
    RpSetSecurityLevel(theDataPtr, 2, eRpSecurity_SecureSocketOnly);
#else
	RpSetSecurityLevel(theDataPtr, 2, eRpSecurity_PasswordOnly);
#endif	/* RomPagerSecure */
#endif

	/*
		Realm 4.
	*/
#if (kRpNumberOfRealms > 3)
	RpSetRealmName(theDataPtr, 3, "Realm4");
#if RomPagerSecure
    RpSetSecurityLevel(theDataPtr, 3, eRpSecurity_SecureSocketPasswordOnly);
#else
	RpSetSecurityLevel(theDataPtr, 3, eRpSecurity_PasswordOnly);
#endif	/* RomPagerSecure */
#endif

	/*
		Realm 5.
	*/
#if (kRpNumberOfRealms > 4)
	RpSetRealmName(theDataPtr, 4, "Realm5");
#if RomPagerSecure
	RpSetSecurityLevel(theDataPtr, 4, eRpSecurity_SecureSocketOnly);
#else
	RpSetSecurityLevel(theDataPtr, 4, eRpSecurity_PasswordOnly);
#endif
#endif

	/*
		Realm 6.
	*/
#if (kRpNumberOfRealms > 5)
	RpSetRealmName(theDataPtr, 5, "Realm6");
	RpSetSecurityLevel(theDataPtr, 5, eRpSecurity_PasswordOnly);
#endif

	/*
		Realm 7.
	*/
#if (kRpNumberOfRealms > 6)
	RpSetRealmName(theDataPtr, 6, "Realm7");
	RpSetSecurityLevel(theDataPtr, 6, eRpSecurity_PasswordOnly);
#endif

	/*
		Realm 8.
	*/
#if (kRpNumberOfRealms > 7)
	RpSetRealmName(theDataPtr, 7, "Realm8");
	RpSetSecurityLevel(theDataPtr, 7, eRpSecurity_PasswordOnly);
#endif

/*******************************************************************************
	End of Realm set up.
*******************************************************************************/

//--- add by jesson 91.8.15

#ifdef WIN32
#if !RomPagerExternalPassword

/*******************************************************************************

	User set up - set up the parameters for each user.  These are:
		user_name, password, realm access code, IP Address, session timeout.

	NOTE:	Even though we have room for 8 users in the default RomPager
			configuration, only configure 6 users.  This way, the customer
			has the opportunity to add a couple of users (using the demo pages)
			before running out of user entrys.

*******************************************************************************/

	/*
		Username:			User
		Password:			Password
		Realm Access:		Realm1
		IP address:			0 (accept any IP address)
		Session Timeout:	0 (in seconds, 0 = use server default timeout
	*/
	RpSetUserAttributes(theDataPtr, "User", "Password",
						kRpPageAccess_Realm1|kRpPageAccess_Realm7|kRpPageAccess_Realm8, 0, 0);
//						kRpPageAccess_Realm1, 0, 0);

	/*
		User 2.
	*/
#if (kRpNumberOfUsers > 1)
	RpSetUserAttributes(theDataPtr, "User2", "Password2",
						kRpPageAccess_Realm2, 0, 0);
#endif

	/*
		User 3.
	*/
#if (kRpNumberOfUsers > 2)
	RpSetUserAttributes(theDataPtr, "User3", "Password3",
						kRpPageAccess_Realm3, 0, 0);
#endif

	/*
		User 4.
	*/
#if (kRpNumberOfUsers > 3)
#if RomPagerSecure
	RpSetUserAttributes(theDataPtr, "RomPagerSecure", "TopSecret",
						kRpPageAccess_Realm4, 0, 0);
#else
	RpSetUserAttributes(theDataPtr, "Fourth User", "Password4",
						kRpPageAccess_Realm4, 0, 0);
#endif	/* RomPagerSecure */
#endif

	/*
		User 5.
	*/
#if (kRpNumberOfUsers > 4)
	RpSetUserAttributes(theDataPtr, "User5", "Password5",
						kRpPageAccess_Realm5, 0, 0);
#endif

	/*
		User 6.
	*/
#if (kRpNumberOfUsers > 5)
	RpSetUserAttributes(theDataPtr, "User6", "Password6",
						kRpPageAccess_Realm6, 0, 0);
#endif

#if (kRpNumberOfUsers > 6 ) // add by WindChen, 3/22/2002
    RpSetUserAttributes(theDataPtr, "User7", "Password7",
                        kRpPageAccess_Realm7, 0, 0);
#endif

#if (kRpNumberOfUsers > 7 ) // add by WindChen, 3/22/2002
    RpSetUserAttributes(theDataPtr, "User8", "Password8",
                        kRpPageAccess_Realm8, 0, 0);
#endif

/*******************************************************************************
	End of User set up.
*******************************************************************************/

#endif	/* !RomPagerExternalPassword */

#else /* if WIN32*/
#if 0   /*Elsa modify for web*/



	RpSetRealmName(theDataPtr, 5, deviceType);
	RpSetRealmName(theDataPtr, 6, deviceType);
	RpSetRealmName(theDataPtr, 7, deviceType);

	memset( &(account_item), 0, sizeof( ST_USER_ACCOUNT_T ) ) ;
    	strcpy (DUser, "" );
    	strcpy (DPass, "" );
    	strcpy (UserN, "" );
    	strcpy (PassW, "" );
    	account_item.user_name_len = 0 ;            /* key */
    	account_item.user_name[0] = GET_NULL ;  /* key */

    	//for null user
    	if(ST_User_Table_Is_Empty())
    		RpSetUserAttributes(theDataPtr, UserN, PassW,
                        		kRpPageAccess_Realm8, 0, theDataPtr->fPasswordSessionTimeout);



#if 1
	for(usercount=0;  usercount< kRpNumberOfUsers; usercount++)
	{

		if (ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item))//get user
		{

			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;

			if( account_item.privilege == EPM_CPU_NORMAL_USER) //user
				RpSetUserAttributes(theDataPtr, UserN, PassW,
                        		kRpPageAccess_Realm7, 0, theDataPtr->fPasswordSessionTimeout);
			else if( account_item.privilege == EPM_CPU_MEDIUM_USER ) //user+
				RpSetUserAttributes(theDataPtr, UserN, PassW,
                        		kRpPageAccess_Realm6, 0, theDataPtr->fPasswordSessionTimeout);

			else	//super user
				RpSetUserAttributes(theDataPtr, UserN, PassW,
                        		kRpPageAccess_Realm8, 0, theDataPtr->fPasswordSessionTimeout);

		}
		else	//don't get next user
			break;


	}//end for



#else /* if 1 */


#if !RomPagerExternalPassword
	/* << 1998/05/28 Added by Alien */
    memset( &(account_item), 0, sizeof( ST_USER_ACCOUNT_T ) ) ;
    strcpy (DUser, "" );
    strcpy (DPass, "" );
    strcpy (UserN, "" );
    strcpy (PassW, "" );
    account_item.user_name_len = 0 ;            /* key */
    account_item.user_name[0] = GET_NULL ;  /* key */
    if (ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item))
	{
		//printf("[Get1]");
		memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
		memcpy( DUser, account_item.user_name, account_item.user_name_len ) ;
		*(UserN+account_item.user_name_len) = GET_NULL ;
		*(DUser+account_item.user_name_len) = GET_NULL ;
		memcpy( PassW, account_item.password, account_item.password_len ) ;
		memcpy( DPass, account_item.password, account_item.password_len ) ;
		*(PassW+account_item.password_len) = GET_NULL ;
		*(DPass+account_item.password_len) = GET_NULL ;
		/* 1998/05/28 Added by WSH >>*/
		/* <<1998/05/28 Changed by WSH */
		//RpSetUserName(theDataPtr,  0, UserN);
		//RpSetPassword(theDataPtr,  0, PassW);
		/* 1998/05/28 Changed by WSH >>*/
		RpSetUserAttributes(theDataPtr, UserN, PassW,
                        kRpPageAccess_Realm1, 0, 0);


	}
	else
	{
		RpSetRealmName(theDataPtr, 0, deviceType);
		RpSetUserAttributes(theDataPtr, UserN, PassW,
                        kRpPageAccess_Realm1, 0, 0);
/*
		RpSetUserName(theDataPtr,  0, UserN);
		RpSetPassword(theDataPtr,  0, PassW);*/

	}
	RpSetSecurityLevel(theDataPtr,  0, eRpSecurity_PasswordOnly);
#endif

#if (kRpNumberOfRealms > 1)
	/* << 1998/05/29 Added by WSH */
	if (ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item))
	{
		memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
		*(UserN+account_item.user_name_len) = GET_NULL ;
		memcpy( PassW, account_item.password, account_item.password_len ) ;
		*(PassW+account_item.password_len) = GET_NULL ;
		RpSetRealmName(theDataPtr, 1, deviceType);
		/*RpSetUserName(theDataPtr,  1, UserN);
		RpSetPassword(theDataPtr,  1, PassW);*/
		RpSetUserAttributes(theDataPtr, UserN, PassW,
                        kRpPageAccess_Realm2, 0, 0);
	}
	else
	{
		RpSetRealmName(theDataPtr, 1, deviceType);
		/*
		RpSetUserName(theDataPtr,  1, DUser);
		RpSetPassword(theDataPtr,  1, DPass);*/
		RpSetUserAttributes(theDataPtr, DUser, DPass,
                        kRpPageAccess_Realm2, 0, 0);

	}
	RpSetSecurityLevel(theDataPtr,  1, eRpSecurity_PasswordOnly);
	/* 1998/05/29 Added by WSH >>*/
#endif

#if (kRpNumberOfRealms > 2)
	/* << 1998/05/29 Added by WSH */
	if (ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item))
	{
		memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
		*(UserN+account_item.user_name_len) = GET_NULL ;
		memcpy( PassW, account_item.password, account_item.password_len ) ;
		*(PassW+account_item.password_len) = GET_NULL ;
		RpSetRealmName(theDataPtr, 2, deviceType);
		/*
		RpSetUserName(theDataPtr,  2, UserN);
		RpSetPassword(theDataPtr,  2, PassW);*/
		RpSetUserAttributes(theDataPtr, UserN, PassW,
                        kRpPageAccess_Realm3, 0, 0);
	}
	else
	{
		RpSetRealmName(theDataPtr, 2, deviceType);
		/*
		RpSetUserName(theDataPtr,  2, DUser);
		RpSetPassword(theDataPtr,  2, DPass);*/
		RpSetUserAttributes(theDataPtr, DUser, DPass,
                        kRpPageAccess_Realm3, 0, 0);
	}
	/* 1998/05/29 Added by WSH >>*/
	RpSetSecurityLevel(theDataPtr,  2, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 3)
	if (ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item))
	{
		memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
		*(UserN+account_item.user_name_len) = GET_NULL ;
		memcpy( PassW, account_item.password, account_item.password_len ) ;
		*(PassW+account_item.password_len) = GET_NULL ;
		RpSetRealmName(theDataPtr, 3, deviceType);
	/*	RpSetUserName(theDataPtr,  3, UserN);
		RpSetPassword(theDataPtr,  3, PassW);*/
		RpSetUserAttributes(theDataPtr, UserN, PassW,
                        kRpPageAccess_Realm4, 0, 0);
	}
	else
	{
		RpSetRealmName(theDataPtr, 3, deviceType);
		/*
		RpSetUserName(theDataPtr,  3, DUser);
		RpSetPassword(theDataPtr,  3, DPass);*/
		RpSetUserAttributes(theDataPtr, DUser, DPass,
                        kRpPageAccess_Realm4, 0, 0);
	}
	RpSetSecurityLevel(theDataPtr,  3, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 4)
	if (ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item))
	{
		memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
		*(UserN+account_item.user_name_len) = GET_NULL ;
		memcpy( PassW, account_item.password, account_item.password_len ) ;
		*(PassW+account_item.password_len) = GET_NULL ;
		RpSetRealmName(theDataPtr, 4, deviceType);
		/*
		RpSetUserName(theDataPtr,  4, UserN);
		RpSetPassword(theDataPtr,  4, PassW);*/
			RpSetUserAttributes(theDataPtr, UserN, PassW,
                        kRpPageAccess_Realm5, 0, 0);
	}
	else
	{
		RpSetRealmName(theDataPtr, 4, deviceType);
		/*
		RpSetUserName(theDataPtr,  4, DUser);
		RpSetPassword(theDataPtr,  4, DPass);*/
		RpSetUserAttributes(theDataPtr, DUser, DPass,
                        kRpPageAccess_Realm5, 0, 0);
	}
	RpSetSecurityLevel(theDataPtr,  4, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 5)
	if (ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item))
	{
		memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
		*(UserN+account_item.user_name_len) = GET_NULL ;
		memcpy( PassW, account_item.password, account_item.password_len ) ;
		*(PassW+account_item.password_len) = GET_NULL ;
		RpSetRealmName(theDataPtr, 5, deviceType);
		/*
		RpSetUserName(theDataPtr,  5, UserN);
		RpSetPassword(theDataPtr,  5, PassW);*/
		RpSetUserAttributes(theDataPtr, UserN, PassW,
                        kRpPageAccess_Realm6, 0, 0);
	}
	else
	{
		RpSetRealmName(theDataPtr, 5, deviceType);
		/*
		RpSetUserName(theDataPtr,  5, DUser);
		RpSetPassword(theDataPtr,  5, DPass);*/

		RpSetUserAttributes(theDataPtr, DUser, DPass,
                        kRpPageAccess_Realm6, 0, 0);
	}
	RpSetSecurityLevel(theDataPtr,  5, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 6)
	if (ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item))
	{
		memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
		*(UserN+account_item.user_name_len) = GET_NULL ;
		memcpy( PassW, account_item.password, account_item.password_len ) ;
		*(PassW+account_item.password_len) = GET_NULL ;
		RpSetRealmName(theDataPtr, 6, deviceType);
		/*
		RpSetUserName(theDataPtr,  6, UserN);
		RpSetPassword(theDataPtr,  6, PassW);*/
		RpSetUserAttributes(theDataPtr, UserN, PassW,
                        kRpPageAccess_Realm7, 0, 0);
	}
	else
	{
		RpSetRealmName(theDataPtr, 6, deviceType);
		/*
		RpSetUserName(theDataPtr,  6, DUser);
		RpSetPassword(theDataPtr,  6, DPass);*/

		RpSetUserAttributes(theDataPtr, DUser, DPass,
                        kRpPageAccess_Realm7, 0, 0);
	}
	RpSetSecurityLevel(theDataPtr,  6, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 7)
	if (ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item))
	{
		memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
		*(UserN+account_item.user_name_len) = GET_NULL ;
		memcpy( PassW, account_item.password, account_item.password_len ) ;
		*(PassW+account_item.password_len) = GET_NULL ;
		RpSetRealmName(theDataPtr, 7, deviceType);
		/*
		RpSetUserName(theDataPtr,  7, UserN);
		RpSetPassword(theDataPtr,  7, PassW);*/
		RpSetUserAttributes(theDataPtr, UserN, PassW,
                        kRpPageAccess_Realm8, 0, 0);
	}
	else
	{
		RpSetRealmName(theDataPtr, 7, deviceType);
		/*
		RpSetUserName(theDataPtr,  7, DUser);
		RpSetPassword(theDataPtr,  7, DPass);*/

		RpSetUserAttributes(theDataPtr, DUser, DPass,
                        kRpPageAccess_Realm8, 0, 0);
	}
	RpSetSecurityLevel(theDataPtr,  7, eRpSecurity_PasswordOnly);
#endif

// Mandy modified on 2000/09/25
#if (kRpNumberOfRealms > 8)
	/* << 1998/05/28 Added by WSH */
	account_item.user_name_len = 0 ;            /* key */
	account_item.user_name[0] = GET_NULL ;  /* key */

	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_MEDIUM_USER)||(account_item.privilege == EPM_CPU_SUPER_USER)) /* EPM_CPU_MEDIUM_USER (user+) */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetRealmName(theDataPtr, 8, deviceType);
			RpSetUserName(theDataPtr,  8, UserN);
			RpSetPassword(theDataPtr,  8, PassW);
		}
        else
        {
            strcpy (UserN, "#@123@#" );
            strcpy (PassW, "#@123@#" );
            RpSetRealmName(theDataPtr, 8, deviceType);
            RpSetUserName(theDataPtr,  8, UserN);
            RpSetPassword(theDataPtr,  8, PassW);
			while (TRUE == ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*)
                   &account_item))
            {
				if ((account_item.privilege == EPM_CPU_MEDIUM_USER)) /* EPM_CPU_MEDIUM_USER (user+) */
                {
					memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
                    *(UserN+account_item.user_name_len) = GET_NULL ;
                    memcpy( PassW, account_item.password, account_item.password_len ) ;
                    *(PassW+account_item.password_len) = GET_NULL ;
                    RpSetRealmName(theDataPtr, 8, deviceType);
                    RpSetUserName(theDataPtr,  8, UserN);
                    RpSetPassword(theDataPtr,  8, PassW);
                    break;
                }
                else
                {
                    strcpy (UserN, "@#123@#" );
                    strcpy (PassW, "@#123@#" );
                    RpSetRealmName(theDataPtr, 8, deviceType);
                    RpSetUserName(theDataPtr,  8, UserN);
                    RpSetPassword(theDataPtr,  8, PassW);
				}
			}
		}
	}
	else
	{
		RpSetRealmName(theDataPtr, 8, deviceType);
		RpSetUserName(theDataPtr,  8, UserN);
		RpSetPassword(theDataPtr,  8, PassW);
	}
	RpSetSecurityLevel(theDataPtr,  8, eRpSecurity_PasswordOnly);
	/* 1998/05/28 Added by WSH >>*/
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 9)
	/* user+ */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_MEDIUM_USER)) /* EPM_CPU_MEDIUM_USER (user+) */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetRealmName(theDataPtr, 9, deviceType);
			RpSetUserName(theDataPtr,  9, UserN);
			RpSetPassword(theDataPtr,  9, PassW);
		}
		else
		{
			RpSetRealmName(theDataPtr, 9, deviceType);
			RpSetUserName(theDataPtr,  9, UserN);
			RpSetPassword(theDataPtr,  9, PassW);
		}
	}
	else
	{
		RpSetRealmName(theDataPtr, 9, deviceType);
		RpSetUserName(theDataPtr,  9, UserN);
		RpSetPassword(theDataPtr,  9, PassW);
	}
	RpSetSecurityLevel(theDataPtr,  9, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 10)
	/* user+ */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_MEDIUM_USER)) /* EPM_CPU_MEDIUM_USER (user+) */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetRealmName(theDataPtr, 10, deviceType);
			RpSetUserName(theDataPtr,  10, UserN);
			RpSetPassword(theDataPtr,  10, PassW);
		}
		else
		{
			RpSetRealmName(theDataPtr, 10, deviceType);
			RpSetUserName(theDataPtr,  10, UserN);
			RpSetPassword(theDataPtr,  10, PassW);
		}
	}
	else
	{
		RpSetRealmName(theDataPtr, 10, deviceType);
		RpSetUserName(theDataPtr,  10, UserN);
		RpSetPassword(theDataPtr,  10, PassW);
	}
	RpSetSecurityLevel(theDataPtr,  10, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 11)
	/* user+ */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_MEDIUM_USER)) /* EPM_CPU_MEDIUM_USER (user+) */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetRealmName(theDataPtr, 11, deviceType);
			RpSetUserName(theDataPtr,  11, UserN);
			RpSetPassword(theDataPtr,  11, PassW);
		}
		else
		{
			RpSetRealmName(theDataPtr, 11, deviceType);
			RpSetUserName(theDataPtr,  11, UserN);
			RpSetPassword(theDataPtr,  11, PassW);
		}
	}
	else
	{
		RpSetRealmName(theDataPtr, 11, deviceType);
		RpSetUserName(theDataPtr,  11, UserN);
		RpSetPassword(theDataPtr,  11, PassW);
	}
	RpSetSecurityLevel(theDataPtr,  11, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 12)
	/* user+ */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_MEDIUM_USER)) /* EPM_CPU_MEDIUM_USER (user+) */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetRealmName(theDataPtr, 12, deviceType);
			RpSetUserName(theDataPtr,  12, UserN);
			RpSetPassword(theDataPtr,  12, PassW);
		}
		else
		{
			RpSetRealmName(theDataPtr, 12, deviceType);
			RpSetUserName(theDataPtr,  12, UserN);
			RpSetPassword(theDataPtr,  12, PassW);
		}
	}
	else
	{
		RpSetRealmName(theDataPtr, 12, deviceType);
		RpSetUserName(theDataPtr,  12, UserN);
		RpSetPassword(theDataPtr,  12, PassW);
	}
	RpSetSecurityLevel(theDataPtr,  12, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 13)
	/* user+ */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_MEDIUM_USER)) /* EPM_CPU_MEDIUM_USER (user+) */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetRealmName(theDataPtr, 13, deviceType);
			RpSetUserName(theDataPtr,  13, UserN);
			RpSetPassword(theDataPtr,  13, PassW);
		}
		else
		{
			RpSetRealmName(theDataPtr, 13, deviceType);
			RpSetUserName(theDataPtr,  13, UserN);
			RpSetPassword(theDataPtr,  13, PassW);
		}
	}
	else
	{
		RpSetRealmName(theDataPtr, 13, deviceType);
		RpSetUserName(theDataPtr,  13, UserN);
		RpSetPassword(theDataPtr,  13, PassW);
	}
	RpSetSecurityLevel(theDataPtr,  13, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 14)
	/* user+ */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_MEDIUM_USER)) /* EPM_CPU_MEDIUM_USER (user+) */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetRealmName(theDataPtr, 14, deviceType);
			RpSetUserName(theDataPtr,  14, UserN);
			RpSetPassword(theDataPtr,  14, PassW);
		}
		else
		{
			RpSetRealmName(theDataPtr, 14, deviceType);
			RpSetUserName(theDataPtr,  14, UserN);
			RpSetPassword(theDataPtr,  14, PassW);
		}
	}
	else
	{
		RpSetRealmName(theDataPtr, 14, deviceType);
		RpSetUserName(theDataPtr,  14, UserN);
		RpSetPassword(theDataPtr,  14, PassW);
	}
	RpSetSecurityLevel(theDataPtr,  14, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 15)
	/* user+ */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_MEDIUM_USER)) /* EPM_CPU_MEDIUM_USER (user+) */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetRealmName(theDataPtr, 15, deviceType);
			RpSetUserName(theDataPtr,  15, UserN);
			RpSetPassword(theDataPtr,  15, PassW);
		}
		else
		{
			RpSetRealmName(theDataPtr, 15, deviceType);
			RpSetUserName(theDataPtr,  15, UserN);
			RpSetPassword(theDataPtr,  15, PassW);
		}
	}
	else
	{
		RpSetRealmName(theDataPtr, 15, deviceType);
		RpSetUserName(theDataPtr,  15, UserN);
		RpSetPassword(theDataPtr,  15, PassW);
	}
	RpSetSecurityLevel(theDataPtr,  15, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 16)
	account_item.user_name_len = 0 ;            /* key */
	account_item.user_name[0] = GET_NULL ;  /* key */

	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_SUPER_USER)) /* Super user */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetUserName(theDataPtr,  16, UserN);
			RpSetPassword(theDataPtr,  16, PassW);
		}
        else
        {
            strcpy (UserN, "@#123@#" );
            strcpy (PassW, "@#123@#" );
            RpSetUserName(theDataPtr,  16, UserN);
            RpSetPassword(theDataPtr,  16, PassW);
#if 0
			while (TRUE == ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item))
            {
				if ((account_item.privilege == EPM_CPU_SUPER_USER)) /* Super user */
                {
					memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
                    *(UserN+account_item.user_name_len) = GET_NULL ;
                    memcpy( PassW, account_item.password, account_item.password_len ) ;
                    *(PassW+account_item.password_len) = GET_NULL ;
                    RpSetUserName(theDataPtr,  16, UserN);
                    RpSetPassword(theDataPtr,  16, PassW);
                    break;
                }
                else
                {
                    strcpy (UserN, "@#123@#" );
                    strcpy (PassW, "@#123@#" );
                    RpSetUserName(theDataPtr,  16, UserN);
                    RpSetPassword(theDataPtr,  16, PassW);
				}
			}
#endif
		}
	}
	else
	{
		RpSetUserName(theDataPtr,  16, UserN);
		RpSetPassword(theDataPtr,  16, PassW);
	}
	RpSetRealmName(theDataPtr, 16, deviceType);
	RpSetSecurityLevel(theDataPtr,  16, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 17)
	/* super user */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_SUPER_USER)) /* Super user */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetUserName(theDataPtr,  17, UserN);
			RpSetPassword(theDataPtr,  17, PassW);
		}
		else
		{
			RpSetUserName(theDataPtr,  17, UserN);
			RpSetPassword(theDataPtr,  17, PassW);
		}
	}
	else
	{
		RpSetUserName(theDataPtr,  17, UserN);
		RpSetPassword(theDataPtr,  17, PassW);
	}
	RpSetRealmName(theDataPtr, 17, deviceType);
	RpSetSecurityLevel(theDataPtr,  17, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 18)
	/* super user */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_SUPER_USER)) /* Super user */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetUserName(theDataPtr,  18, UserN);
			RpSetPassword(theDataPtr,  18, PassW);
		}
		else
		{
			RpSetUserName(theDataPtr,  18, UserN);
			RpSetPassword(theDataPtr,  18, PassW);
		}
	}
	else
	{
		RpSetUserName(theDataPtr,  18, UserN);
		RpSetPassword(theDataPtr,  18, PassW);
	}
	RpSetRealmName(theDataPtr, 18, deviceType);
	RpSetSecurityLevel(theDataPtr,  18, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 19)
	/* super user */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_SUPER_USER)) /* Super user */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetUserName(theDataPtr,  19, UserN);
			RpSetPassword(theDataPtr,  19, PassW);
		}
		else
		{
			RpSetUserName(theDataPtr,  19, UserN);
			RpSetPassword(theDataPtr,  19, PassW);
		}
	}
	else
	{
		RpSetUserName(theDataPtr,  19, UserN);
		RpSetPassword(theDataPtr,  19, PassW);
	}
	RpSetRealmName(theDataPtr, 19, deviceType);
	RpSetSecurityLevel(theDataPtr,  19, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 20)
	/* super user */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_SUPER_USER)) /* Super user */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetUserName(theDataPtr,  20, UserN);
			RpSetPassword(theDataPtr,  20, PassW);
		}
		else
		{
			RpSetUserName(theDataPtr,  20, UserN);
			RpSetPassword(theDataPtr,  20, PassW);
		}
	}
	else
	{
		RpSetUserName(theDataPtr,  20, UserN);
		RpSetPassword(theDataPtr,  20, PassW);
	}
	RpSetRealmName(theDataPtr, 20, deviceType);
	RpSetSecurityLevel(theDataPtr,  20, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 21)
	/* super user */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_SUPER_USER)) /* Super user */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetUserName(theDataPtr,  21, UserN);
			RpSetPassword(theDataPtr,  21, PassW);
		}
		else
		{
			RpSetUserName(theDataPtr,  21, UserN);
			RpSetPassword(theDataPtr,  21, PassW);
		}
	}
	else
	{
		RpSetUserName(theDataPtr,  21, UserN);
		RpSetPassword(theDataPtr,  21, PassW);
	}
	RpSetRealmName(theDataPtr, 21, deviceType);
	RpSetSecurityLevel(theDataPtr,  21, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 22)
	/* super user */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_SUPER_USER)) /* Super user */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetUserName(theDataPtr,  22, UserN);
			RpSetPassword(theDataPtr,  22, PassW);
		}
		else
		{
			RpSetUserName(theDataPtr,  22, UserN);
			RpSetPassword(theDataPtr,  22, PassW);
		}
	}
	else
	{
		RpSetUserName(theDataPtr,  22, UserN);
		RpSetPassword(theDataPtr,  22, PassW);
	}
	RpSetRealmName(theDataPtr, 22, deviceType);
	RpSetSecurityLevel(theDataPtr,  22, eRpSecurity_PasswordOnly);
#endif

// Mandy added on 2000/09/25
#if (kRpNumberOfRealms > 23)
	/* super user */
	if (TRUE==ST_User_Get_Next_Item((ST_USER_ACCOUNT_T*) &account_item) )
	{
		if ((account_item.privilege == EPM_CPU_SUPER_USER)) /* Super user */
		{
			memcpy( UserN, account_item.user_name, account_item.user_name_len ) ;
			*(UserN+account_item.user_name_len) = GET_NULL ;
			memcpy( PassW, account_item.password, account_item.password_len ) ;
			*(PassW+account_item.password_len) = GET_NULL ;
			RpSetUserName(theDataPtr,  23, UserN);
			RpSetPassword(theDataPtr,  23, PassW);
		}
		else
		{
			RpSetUserName(theDataPtr,  23, UserN);
			RpSetPassword(theDataPtr,  23, PassW);
		}
	}
	else
	{
		RpSetUserName(theDataPtr,  23, UserN);
		RpSetPassword(theDataPtr,  23, PassW);
	}
	RpSetRealmName(theDataPtr, 23, deviceType);
	RpSetSecurityLevel(theDataPtr,  23, eRpSecurity_PasswordOnly);
#endif

#if (kRpNumberOfRealms > 24)
   RpSetRealmName(theDataPtr, 24, deviceType);
   RpSetUserName(theDataPtr,  24, "User25");
   RpSetPassword(theDataPtr,  24, "Password25");
#endif

#if (kRpNumberOfRealms > 25)
   RpSetRealmName(theDataPtr, 25, deviceType);
   RpSetUserName(theDataPtr,  25, "User26");
   RpSetPassword(theDataPtr,  25, "Password26");
#endif

#if (kRpNumberOfRealms > 26)
   RpSetRealmName(theDataPtr, 26, deviceType);
   RpSetUserName(theDataPtr,  26, "User27");
   RpSetPassword(theDataPtr,  26, "Password27");
#endif

#if (kRpNumberOfRealms > 27)
   RpSetRealmName(theDataPtr, 27, deviceType);
   RpSetUserName(theDataPtr,  27, "User28");
   RpSetPassword(theDataPtr,  27, "Password28");
#endif

#if (kRpNumberOfRealms > 28)
   RpSetRealmName(theDataPtr, 28, deviceType);
   RpSetUserName(theDataPtr,  28, "User29");
   RpSetPassword(theDataPtr,  28, "Password29");
#endif

#if (kRpNumberOfRealms > 29)
   RpSetRealmName(theDataPtr, 29, deviceType);
   RpSetUserName(theDataPtr,  29, "User30");
   RpSetPassword(theDataPtr,  29, "Password30");
#endif

#if (kRpNumberOfRealms > 30)
   RpSetRealmName(theDataPtr, 30, deviceType);
   RpSetUserName(theDataPtr,  30, "User31");
   RpSetPassword(theDataPtr,  30, "Password31");
#endif

#if (kRpNumberOfRealms > 31)
   RpSetRealmName(theDataPtr, 31, deviceType);
   RpSetUserName(theDataPtr,  31, "User32");
   RpSetPassword(theDataPtr,  31, "Password32");
#endif


#endif /* if 1 */
#endif

#endif /*WIN32 */
   //end add by jesson 91.8.15


#if RomPagerIpp
/*******************************************************************************
	Set up IPP printer name(s) and realm access code(s) here.
*******************************************************************************/

#if kIppNumberOfPrinters > 0
	RpSetIppPrinterName(theDataPtr, 0, kRpPageAccess_Unprotected, "ipp");
#endif
#if kIppNumberOfPrinters > 1
	RpSetIppPrinterName(theDataPtr, 1, kRpPageAccess_Unprotected, "ipp2");
#endif
#if kIppNumberOfPrinters > 2
	RpSetIppPrinterName(theDataPtr, 2, kRpPageAccess_Unprotected, "ipp3");
#endif

#endif	/* RomPagerIpp */


#if RomPagerPutMethod
	/*
		Set the realms that can send PUT commands.
	*/
	RpSetPutAccess(theDataPtr, kRpPageAccess_Unprotected);
#endif	/* RomPagerPutMethod */


#endif	/* RomPagerServer && RomPagerSecurity */


#if RomPagerServer && RomPagerDynamicGlobals
	/*
		Allocate memory required for the users data.
	*/
	RpInitUserData(theDataPtr);
#endif

	return;
}


#if RomPagerServerPush
/*
	RpUserServerPushExit

	This routine is called once per second during the server push
	wait period.  The device can call the server push call backs
	here, or perform other status checking.
*/
void RpUserServerPushExit(void *theTaskDataPtr) {

	return;
}

#endif	/* RomPagerServerPush */

#if RomPagerPutMethod
/*
	RpHttpPutComplete

	This routine is called when a HTTP PUT command completes. The
	path of the PUT URL is provided in thePathPtr.
*/
void RpHttpPutComplete(void *theTaskDataPtr, char *thePathPtr) {

	return;
}

#endif

#endif	/* RomPagerServer || RomCli */
