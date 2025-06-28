/*
 *	File:		RpAccess.c
 *
 *	Contains:	RomPager routines for HTTP access control
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	?1995-2003 by Allegro Software Development Corporation
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
 *		09/03/03	pjr		add RpGetUserCount
 * * * * Release 4.21  * * *
 *		03/19/03	pjr		if RomPagerExternalPassword, delete the user entry
 *							when a session is reset
 *		03/18/03	pjr		when starting an HTTP session, clear the challenge
 *							timers for all realms in the user's access code
 *		03/14/03	pjr		fix compile warnings
 *		03/04/03	pjr		set fChallengeTimeout from variable (not constant)
 * * * * Release 4.20  * * *
 *		02/21/03	pjr		change max password length parameter, fix username
 *							and password usage in RpCheckUserExitAuthorization
 *		12/18/02	bva		bump theConnection size for RpCheckAuthorization
 *		12/17/02	bva		increase timer sizes for various calls
 *		12/12/02	pjr		use a timer instead of an object pointer to keep
 *							track of challenges
 *		12/11/02	bva		fix warning
 *		09/24/02	pjr		add RpGetUserIndex
 *		09/20/02	pjr		set fAuthenticatedUserPtr in Start Session
 *		09/04/02	pjr		add user exit security routines
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *		07/15/02	rhb/bva	fix External Password compiler error
 * * * * Release 4.10  * * *
 *		04/18/02	rhb		fix compiler warnings
 * * * * Release 4.07  * * *
 * * * * Release 4.00  * * *
 *		08/07/01	pjr		increase user index to an Unsigned16, fix bug in
 *							RpSetPasswordCookie
 *		07/11/01	rhb		clear fSessionCloseFuncPtr in ResetSession
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		10/06/00	pjr		make GetLeastSecureRealm global and enable it for
 *							RomPagerIpp.  remove RpSetIppAccess (obsolete)
 *		09/08/00	pjr		fix compile errors and warnings
 *		08/31/00	pjr		initial version of new security model
 * * * * Release 3.01  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"
#ifndef WIN32
#include "general.h"
#endif

#if RomPagerServer && RomPagerSecurity

static Boolean			CheckAuthorization2(rpDataPtr theDataPtr,
							rpUserPtr theUserPtr);

static rpPasswordState	CheckRealmAuth(rpDataPtr theDataPtr,
							rpUserPtr theUserPtr,
							rpAccess theAccessCode,
							rpAccess theAuthRealms);

static Boolean			CheckRealmDisabled(rpDataPtr theDataPtr,
							rpAccess theAccessCode);

static rpPasswordState	CheckStartSession(rpDataPtr theDataPtr,
							rpUserPtr theUserPtr,
							rpAccess theAuthRealms);

static rpAccess			CheckUserAuth(rpDataPtr theDataPtr,
							rpUserPtr theUserPtr);

static void				ClearRealmLocks(rpDataPtr theDataPtr,
							rpUserPtr theUserPtr);

static void				DeleteUser(rpDataPtr theDataPtr,
							rpUserPtr theUserPtr);

static rpUserPtr		FindUser(rpDataPtr theDataPtr,
							char * theUsername);

static rpAccess			GetAuthRealms(rpDataPtr theDataPtr,
							rpUserPtr theUserPtr,
							Unsigned32 theIpAddress);

static rpPasswordState	LockRealms(rpDataPtr theDataPtr,
							rpUserPtr theUserPtr,
							rpAccess theRealmLockCode);

static void				ResetSession(rpDataPtr theDataPtr,
							rpUserPtr theUserPtr);

static void				SetupChallenge(rpDataPtr theDataPtr,
							rpAccess theAccessCode,
							rpPasswordState thePasswordState);

static rpPasswordState	StartSession(rpDataPtr theDataPtr,
							rpUserPtr theUserPtr,
							rpAccess theAuthRealms);

#if RomPagerSecurityDigest
static void				CalculateDigest(rpHttpRequestPtr theRequestPtr,
							char * theUsername,
							char * thePassword,
							char * theNonce,
							char * theDigest);

static Boolean			CheckDigestAuth(rpHttpRequestPtr theRequestPtr,
							rpUserPtr theUserPtr,
							Unsigned32 theRemoteAddress);

static void				ClearNonces(rpDataPtr theDataPtr,
							rpAccess theAccessCode);
#endif	/* RomPagerSecurityDigest */


/*****************************************************************************
	Global routines that are internal to RomPager
*****************************************************************************/

int RpWebID_Check_Login_Flag(char *l_RpWebID);
int RpWebID_Update_Browse_Time(char *l_RpWebID);
int	RpWebID_Add_Account_Waiting_For_Login_Challenge(char *l_RpWebID);
int	RpWebID_Set_Already_Challenged(char *l_RpWebID, unsigned long ip, char *username, char *password, int access_right);
int RpWebID_Check_Enable_Admin(char *l_RpWebID);
int Check_UserAccessRight(char *l_RpWebID, unsigned long ip, char *username, char *password, int enable_admin);

extern int sWeb_Main_DeviceModel_Get();
rpPasswordState RpCheckAccess(rpDataPtr theDataPtr, rpAccess theAccessCode) {
	rpAccess			theAuthRealms;
	rpPasswordState		thePasswordState;
	rpHttpRequestPtr	theRequestPtr;
	rpUserPtr			theUserPtr;
	rpConnectionPtr 	theConnectionPtr;

	int gChallenge_login_flag=0;
	int access_right=0;
	int enable_admin=0;

    int deviceType=sWeb_Main_DeviceModel_Get();
    
#if RomPagerSecure
	rpSecurityLevel		theSecurityLevel;
#endif
#if RomPagerExternalPassword || RomPagerSecure
	rpRealmPtr 			theRealmPtr;
#endif

	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	thePasswordState = eRpPasswordAuthorized;
	theAuthRealms = kRpPageAccess_Unprotected;
	theUserPtr = (rpUserPtr) 0;

    if(deviceType == 0)
        RpSetRealmName(theDataPtr, 0, "A1013");
    else
        RpSetRealmName(theDataPtr, 0, "unknow");

// +++ Arthur Chow 2005/04/22
// If use direct url of a protected web page to Rompager, then return eRpHttpNoObjectFound state.
// It means user only can connect to the Rompager from the root page. "http://xxx.xxx.xxx.xxx/"
	if (theRequestPtr->fHttpCookies[0][0]==0)
	{
		if (theAccessCode != kRpPageAccess_Unprotected) {
		    theRequestPtr->fHttpResponseState = eRpHttpNoObjectFound;
		    theRequestPtr->fObjectPtr = &gRpHttpNoObjectFoundPage;
			theRequestPtr->fRealmPtr = theDataPtr->fRealms;
			return eRpPasswordNotAuthorized;
		}
	}
// --- Arthur Chow 2005/04/22

	RpWebID_Update_Browse_Time(theRequestPtr->fHttpCookies[0]);

	/*
		Return authorized if this page is unprotected.
	*/
	if (theAccessCode == kRpPageAccess_Unprotected) {
		return eRpPasswordAuthorized;
	}

	enable_admin=RpWebID_Check_Enable_Admin(theRequestPtr->fHttpCookies[0]);
	if (enable_admin==1)
	{
	    theRequestPtr->fHttpResponseState = eRpHttpNeedBasicAuthorization;
	    theRequestPtr->fObjectPtr = &gRpAccessNotAllowedPage;
		theRequestPtr->fRealmPtr = theDataPtr->fRealms;
   		return eRpPasswordNotAuthorized;
	}

	gChallenge_login_flag=RpWebID_Check_Login_Flag(theRequestPtr->fHttpCookies[0]);
	if (gChallenge_login_flag==0)
	{
		RpWebID_Add_Account_Waiting_For_Login_Challenge(theRequestPtr->fHttpCookies[0]);
	    theRequestPtr->fHttpResponseState = eRpHttpNeedBasicAuthorization;
	    theRequestPtr->fObjectPtr = &gRpAccessNotAllowedPage;
		theRequestPtr->fRealmPtr = theDataPtr->fRealms;
   		return eRpPasswordNotAuthorized;
	}
	else
	if (gChallenge_login_flag==2)
	{
		return eRpPasswordAuthorized;
	}

	access_right = Check_UserAccessRight(theRequestPtr->fHttpCookies[0], theConnectionPtr->fIpRemote, theRequestPtr->fUsername, theRequestPtr->fPassword, enable_admin);
	if (access_right==0) // User challenge fail
	{
	    theRequestPtr->fHttpResponseState = eRpHttpNeedBasicAuthorization;
	    theRequestPtr->fObjectPtr = &gRpAccessNotAllowedPage;
		theRequestPtr->fRealmPtr = theDataPtr->fRealms;
   		return eRpPasswordNotAuthorized;
	}
	else
	{
		RpWebID_Set_Already_Challenged(theRequestPtr->fHttpCookies[0], theConnectionPtr->fIpRemote, theRequestPtr->fUsername, theRequestPtr->fPassword, access_right);
		return eRpPasswordAuthorized;
	}

	/*
		Return authorized if the realm has been disabled.
	*/
	if (CheckRealmDisabled(theDataPtr, theAccessCode)) {
		return eRpPasswordAuthorized;
	}

#if RomPagerSecure
	/*
		See if we need a secure connection.
	*/
	theRealmPtr = RpGetLeastSecureRealm(theDataPtr, theAccessCode);
	theSecurityLevel = theRealmPtr->fSecurityLevel;

	if (theConnectionPtr->fIsTlsFlag) {
		/*
			This is a secure connection.
		*/
		if (theSecurityLevel == eRpSecurity_SecureSocketOnly) {
			/*
				All we need is a secure connection.
			*/
			return eRpPasswordAuthorized;
		}
	}
	else {
		/*
			This is not a secure connection.
		*/
		if (theSecurityLevel & kRpSecuritySSL) {
			/*
				We need a secure connection.  Set up to return the
				secure connection required page.
			*/
			theRequestPtr->fHttpResponseState = eRpHttpForbidden;
			theRequestPtr->fObjectPtr = &gRpSslRequiredPage;
			return eRpPasswordNotAuthorized;
		}
	}
#endif	/* RomPagerSecure */

	/*
		This object is protected and the realm is not disabled.
		We need to authenticate the request.
	*/
	if (*theRequestPtr->fUsername == '\0') {
		/*
			The requestor didn't supply a username.  We need
			to challenge him.
		*/
		thePasswordState = eRpPasswordNotAuthorized;
	}
	else {

		/*
			See if the requesting user is in the user database.
		*/
		theUserPtr = FindUser(theDataPtr, theRequestPtr->fUsername);

		if (theUserPtr == (rpUserPtr) 0) {

#if RomPagerExternalPassword
			/*
				The requesting user isn't in the local user database.
				Check it externally.
			*/
			theRealmPtr = RpGetLeastSecureRealm(theDataPtr, theAccessCode);
			theRequestPtr->fRealmPtr = theRealmPtr;
			theConnectionPtr->fState = eRpConnectionWaitingExternalSecurity;
			thePasswordState = eRpPasswordPending;
			return eRpPasswordPending;
#else
			/*
				The requesting user isn't in the user database.
				We need to challenge him.
			*/
			thePasswordState = eRpPasswordNotAuthorized;
			SetupChallenge(theDataPtr, theAccessCode, thePasswordState);
#endif
		}
		else {
			/*
				The requesting user is in the user database.
				See if the requestor has valid credentials for
				access to the user entry.
			*/
			theAuthRealms = CheckUserAuth(theDataPtr, theUserPtr);

			if (theAuthRealms != kRpPageAccess_Unprotected) {
				thePasswordState = eRpPasswordAuthorized;
			}
		}
	}

	if (thePasswordState == eRpPasswordAuthorized) {
		/*
			The requesting user has credentials for access to
			the user entry in our local users database.

			See if that user entry has access to the object
			being requested.
		*/
		thePasswordState = CheckRealmAuth(theDataPtr,
											theUserPtr,
											theAccessCode,
											theAuthRealms);
	}

	if (thePasswordState != eRpPasswordAuthorized) {
		/*
			We need to set up a challenge.
		*/
		SetupChallenge(theDataPtr, theAccessCode, thePasswordState);
	}

	return thePasswordState;
}


/*
	RpCheckPasswordTimers

	This routine is called once per second to decrement the password
	session timers.  If a timer expires, set the session to 0 so the
	RpCheckAccess routine will force a re-validation.
*/

void RpCheckPasswordTimers(rpDataPtr theDataPtr) {
	Unsigned16				theCount;
	rpSessionCloseFuncPtr	theFunctionPtr;
	rpRealmPtr				theRealmPtr;
	rpUserPtr				theUserPtr;

	theUserPtr = theDataPtr->fUsers;
	theCount = kRpNumberOfUsers;

	while (theCount > 0) {
		if (theUserPtr->fHttpSessionTimer > 0) {
			theUserPtr->fHttpSessionTimer--;
			if (theUserPtr->fHttpSessionTimer == 0) {
				/*
					This HTTP session has timed out.  If there's no
					internal session just reset the session.  If
					there is an internal session just reset the
					HTTP session code.
				*/
				if (theUserPtr->fInternalSessionCode ==
						kRpPageAccess_Unprotected) {
					ResetSession(theDataPtr, theUserPtr);
#if RomPagerExternalPassword
					/*
						Delete the user entry from the local user database
						so we'll call the external password routine next
						time a request is made for this username.
					*/
					DeleteUser(theDataPtr, theUserPtr);
#endif
				}
				else {
					theUserPtr->fHttpSessionCode = kRpPageAccess_Unprotected;
					theFunctionPtr = theUserPtr->fSessionCloseFuncPtr;
					if (theFunctionPtr != (rpSessionCloseFuncPtr) 0) {
						theFunctionPtr(theDataPtr, theUserPtr->fUserCookie);
						theUserPtr->fSessionCloseFuncPtr =
								(rpSessionCloseFuncPtr) 0;
					}
				}
			}
		}
		theUserPtr++;
		theCount--;
	}

	/*
		Cycle through the realms, decrementing the challenge timeouts.
	*/
	theRealmPtr = theDataPtr->fRealms;
	theCount = kRpNumberOfRealms;

	while (theCount > 0) {
		if (theRealmPtr->fChallengeTimeout > 0) {
			theRealmPtr->fChallengeTimeout--;
		}
		theRealmPtr++;
		theCount--;
	}

	return;
}


/*
	Cycle through all the users, ending any sessions and calling
	any session close callback functions.
*/

void RpSecurityDeInit(rpDataPtr theDataPtr) {
	Unsigned16		theIndex;
	rpUserPtr		theUserPtr;

	theUserPtr = theDataPtr->fUsers;

	for (theIndex = 0; theIndex < kRpNumberOfUsers; theIndex++) {
		ResetSession(theDataPtr, theUserPtr);
		theUserPtr++;
	}

	return;
}


/*****************************************************************************
	External Password routines
*****************************************************************************/

#if RomPagerExternalPassword

rpPasswordState RpCheckAccess2(rpDataPtr theDataPtr, rpAccess theAccessCode) {
	rpPasswordState		thePasswordState;
	rpHttpRequestPtr	theRequestPtr;
	rpUserPtr			theUserPtr;
	rpAccess			theAuthRealms;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	thePasswordState = eRpPasswordNotAuthorized;

	theUserPtr = FindUser(theDataPtr, theRequestPtr->fUsername);

	if (theUserPtr != (rpUserPtr) 0) {
		/*
			The requesting user is in the user database.
			See if the requestor has valid credentials for
			access to the user entry.
		*/
		theAuthRealms = CheckUserAuth(theDataPtr, theUserPtr);

		if (theAuthRealms != kRpPageAccess_Unprotected) {
			thePasswordState = eRpPasswordAuthorized;
		}

		if (thePasswordState == eRpPasswordAuthorized) {
			/*
				The requesting user has credentials for access to
				the user entry in our local users database.

				See if that user entry has access to the object
				being requested.
			*/
			thePasswordState = CheckRealmAuth(theDataPtr,
												theUserPtr,
												theAccessCode,
												theAuthRealms);
		}
	}

	if (thePasswordState != eRpPasswordAuthorized) {
		/*
			We need to set up a challenge.
		*/
		SetupChallenge(theDataPtr, theAccessCode, thePasswordState);
	}

	return thePasswordState;
}


void RpHandleExternalSecurity(rpConnectionPtr theConnectionPtr) {
	rpDataPtr			theDataPtr;
	Unsigned32			theIpAddress;
	char				thePassword[kRpMaxPasswordLength];
	rpPasswordState		thePasswordState;
	rpAccess			theRealmAccess;
	rpHttpRequestPtr	theRequestPtr;

	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	theDataPtr = theRequestPtr->fDataPtr;
	RP_STRCPY(thePassword, theRequestPtr->fPassword);
	theIpAddress = 0;
	theRealmAccess = 0;

	thePasswordState = SpwdGetExternalPassword(theDataPtr,
											theConnectionPtr->fIndex,
											theRequestPtr->fUsername,
											thePassword,
											&theIpAddress,
											&theRealmAccess);

	if (thePasswordState != eRpPasswordPending) {
		theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;

		if (thePasswordState == eRpPasswordAuthorized ||
				thePasswordState == eRpPasswordDone) {
			/*
				Add the new user or update the existing user entry.
			*/
			(void) RpSetUserAttributes(theRequestPtr->fDataPtr,
							theRequestPtr->fUsername,
							thePassword,
							theRealmAccess,
							theIpAddress,
							theDataPtr->fPasswordSessionTimeout);
		}
	}

	return;
}

#endif	/* RomPagerExternalPassword */


/*****************************************************************************
	Digest Authentication routines
*****************************************************************************/

#if RomPagerSecurityDigest

/*
	This function will generate a nonce.  It uses: the Time, the
	Server IP Address, the previous nonce (if there was one), and
	the Server Name.  Algorithm:
	nonce = RpMD5(Time:Server IP Address:[previous nonce:]Server Name);
*/

void RpGenerateNonce(rpHttpRequestPtr theRequestPtr, char * theNoncePtr) {
	rpConnectionPtr 	theConnectionPtr;
	Unsigned32			theCurrentTime;
	rpDataPtr			theDataPtr;
	Unsigned8			theNonceDigest[16];
	char				theString[kMaxLineLength];
	char *				theStringPtr;

	theDataPtr = theRequestPtr->fDataPtr;
	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	/*
		Start a string with the Time.
	*/
	theStringPtr = theString;
	theCurrentTime = RpGetSysTimeInSeconds(theDataPtr);
	RpHexToString((unsigned char *)&theCurrentTime, theStringPtr, 4);

	/*
		Time string is 8 characters.
	*/
	theStringPtr += 8;
	*theStringPtr++ = kAscii_Colon;

	/*
		Add the Server IP address to the string.
	*/
	RpHexToString((unsigned char *) &theConnectionPtr->fIpLocal,
					theStringPtr, 4);

	/*
		IP address string is 8 characters.
	*/
	theStringPtr += 8;
	*theStringPtr++ = kAscii_Colon;

	/*
		Make it a C string.
	*/
	*theStringPtr = '\0';

	/*
		If there was a previous nonce, use it in generating the next nonce.
	*/
	if (*theNoncePtr != '\0') {
		RP_STRCAT(theStringPtr, theNoncePtr);
		RP_STRCAT(theStringPtr, kColon);
	}

	/*
		Add the Server Name to the string.
	*/
	RP_STRCAT(theString, theDataPtr->fBoxNameText);

	/*
		Calculate the nonce.
	*/
	RpMD5(theString, theNonceDigest);

	/*
		Convert it to an ASCII string.
	*/
	RpHexToString(theNonceDigest, theNoncePtr, 16);

	return;
}


static Boolean CheckDigestAuth(rpHttpRequestPtr theRequestPtr,
								rpUserPtr theUserPtr,
								Unsigned32 theRemoteAddress) {
	char			theDigest[kDigestStringLength];
	rpRealmPtr		theRealmPtr;
	Boolean			theResult;

	theResult = False;
	theRealmPtr = theRequestPtr->fRealmPtr;

	if (theRealmPtr->fUserLock == theUserPtr) {
		/*
			This request is from the current owner of this realm.
			If the Username and digest do not verify, see if the
			requestor is using a stale nonce.

			If the nonce is stale, use the same nonce again, the
			response will indicate a stale nonce was used.

			If the nonce is not stale, clear the owner of the
			realm and treat this as a new challenge.
		*/
		CalculateDigest(theRequestPtr,
						theUserPtr->fUsername,
						theUserPtr->fPassword,
						theRealmPtr->fNonce,
						theDigest);

		if ((RP_STRCMP(theUserPtr->fUsername, theRequestPtr->fUsername) == 0) &&
			(RP_STRCMP(theDigest, theRequestPtr->fDigest) == 0)) {
			theResult = True;
		}
		else {
			/*
				See if the request's nonce is stale.
			*/
			CalculateDigest(theRequestPtr,
							theUserPtr->fUsername,
							theUserPtr->fPassword,
							theRequestPtr->fNonce,
							theDigest);

			if (RP_STRCMP(theDigest, theRequestPtr->fDigest) == 0) {
				theRequestPtr->fNonceStale = True;
			}
			else {
				/*
					The current owner of this realm has sent in a bad digest
					result and is not using a stale nonce.  Free up this
					realm so a challenge will be sent as if the requestor
					didn't own the realm.  When the challenge is sent, a new
					challenge nonce will be generated.
				*/
				ResetSession(theRequestPtr->fDataPtr, theUserPtr);
#if RomPagerExternalPassword
				/*
					Delete the user entry from the local user database
					so we'll call the external password routine next
					time a request is made for this username.
				*/
				DeleteUser(theRequestPtr->fDataPtr, theUserPtr);
#endif
			}
		}
	}
	else {
		/*
			This request is not from the current owner of this realm.
			It must be a new challenge.
		*/
		CalculateDigest(theRequestPtr,
						theUserPtr->fUsername,
						theUserPtr->fPassword,
						theRealmPtr->fChallengeNonce,
						theDigest);

		if (RP_STRCMP(theDigest, theRequestPtr->fDigest) == 0) {
			theResult = True;
		}
	}

	return theResult;
}


/*
	This routine will calculate the digest for a given nonce.
	The method used to calculate the complete digest is:

		1. theDigest1 = RpMD5(User:Realm:Password);

		2. theDigest2 = RpMD5(Method:URI);

		3. 	Without "qop=auth" header:
				theDigest3 = RpMD5(theDigest1:unq(nonce-value):theDigest2);

			With "qop=auth" header:
				theDigest3 = RpMD5(theDigest1:unq(nonce-value):nc-value:
								unq(cnonce-value):unq(qop-value):theDigest2);
*/

static void CalculateDigest(rpHttpRequestPtr theRequestPtr,
							char * theUsername,
							char * thePassword,
							char * theNonce,
							char * theDigest) {
	Unsigned8			theDigest1[16];
	Unsigned8			theDigest2[16];
	Unsigned8			theDigest3[16];
	char				theString[kMaxLineLength];
	char *				theStringPtr;
	rpRealmPtr 			theRealmPtr = theRequestPtr->fRealmPtr;

	/*
		Do the first step of digest calculation:
			theDigest1 = RpMD5(User:Realm:Password);
	*/
	RP_STRCPY(theString, theUsername);
	RP_STRCAT(theString, kColon);
	RP_STRCAT(theString, theRealmPtr->fRealmName);
	RP_STRCAT(theString, kColon);
	RP_STRCAT(theString, thePassword);

	RpMD5(theString, theDigest1);

	/*
		Create the "Method:URI" string.
	*/
	switch (theRequestPtr->fHttpCommand) {

		case eRpHttpGetCommand:
			RP_STRCPY(theString, kGetColon);
			break;

		case eRpHttpPostCommand:
			RP_STRCPY(theString, kPostColon);
			break;

		case eRpHttpHeadCommand:
			RP_STRCPY(theString, kHeadColon);
			break;

#if RomPagerPutMethod
		case eRpHttpPutCommand:
			RP_STRCPY(theString, kPutColon);
			break;
#endif

#if RomPagerOptionsMethod
		case eRpHttpOptionsCommand:
			RP_STRCPY(theString, kOptionsColon);
			break;
#endif

#if RomPagerTraceMethod
		case eRpHttpTraceCommand:
			RP_STRCPY(theString, kTraceColon);
			break;
#endif

		default:
			break;
	}

	RP_STRCAT(theString, theRequestPtr->fDigestURI);

	/*
		Do the second step of digest calculation:
			theDigest2 = RpMD5(Method:URI);
	*/
	RpMD5(theString, theDigest2);

	/*
		Do the third (and last) step of digest calculation:

		Without "qop=auth" header:
			theDigest3 = RpMD5(theDigest1:unq(nonce-value):theDigest2);

		With "qop=auth" header:
			theDigest3 = RpMD5(theDigest1:unq(nonce-value):nc-value:
								unq(cnonce-value):unq(qop-value):theDigest2);
	*/
	theStringPtr = theString;
	RpHexToString(theDigest1, theStringPtr, 16);
	RP_STRCAT(theString, kColon);
	RP_STRCAT(theStringPtr, theNonce);
	RP_STRCAT(theStringPtr, kColon);

	if (theRequestPtr->fQopAuth) {
		/*
			Do the third (and last) step of digest calculation:
				theDigest3 = RpMD5(theDigest1:nonce:theDigest2);
		*/
		RP_STRCAT(theStringPtr, theRequestPtr->fNonceCount);
		RP_STRCAT(theStringPtr, kColon);
		RP_STRCAT(theStringPtr, theRequestPtr->fCnonce);
		RP_STRCAT(theStringPtr, kColon);
		RP_STRCAT(theStringPtr, kHttpAuth);
		RP_STRCAT(theStringPtr, kColon);
	}

	theStringPtr = theString + RP_STRLEN(theString);
	RpHexToString(theDigest2, theStringPtr, 16);

	RpMD5(theString, theDigest3);

	/*
		Convert it to an ASCII string.
	*/
	RpHexToString(theDigest3, theDigest, 16);

	return;
}


/*
	Clear the nonces for the realms specified by the access code.
*/

static void ClearNonces(rpDataPtr theDataPtr, rpAccess theAccessCode) {
	Unsigned8	theCount;
	rpAccess	theRealmFlag;
	rpRealmPtr	theRealmPtr;

	theRealmPtr = theDataPtr->fRealms;
	theRealmFlag = kRpPageAccess_Realm1;
	theCount = kRpNumberOfRealms;

	while (theCount > 0) {
		if (theAccessCode & theRealmFlag) {
			/*
				This is one of the realms contained in the access code.
			*/
			*theRealmPtr->fNonce = '\0';
		}
		theRealmPtr++;
		theRealmFlag = theRealmFlag << 1;
		theCount--;
	}

	return;
}

#endif	/* RomPagerSecurityDigest */


/*****************************************************************************
	Access control routines
*****************************************************************************/

#if RomPagerPutMethod
void RpSetPutAccess(void *theTaskDataPtr, rpAccess theAccessCode) {

	((rpDataPtr) theTaskDataPtr)->fPutAccess = theAccessCode;
	return;
}
#endif


/*****************************************************************************
	Realm manipulation routines
*****************************************************************************/

void RpSetRealmLocking(void *theTaskDataPtr, Boolean theLockState) {

	((rpDataPtr) theTaskDataPtr)->fRealmLockingFlag = theLockState;

	return;
}


char *RpGetRealmName(void *theTaskDataPtr, unsigned char theIndex) {
	rpRealmPtr	theRealmPtr;

#if RomPagerDebug
	if (theIndex >= kRpNumberOfRealms) {
		RP_PRINTF("Invalid realm index\n");
		return (char *) 0;
	}
#endif

	theRealmPtr = &((rpDataPtr) theTaskDataPtr)->fRealms[theIndex];

	return theRealmPtr->fRealmName;
}


void RpSetRealmName(void *theTaskDataPtr, unsigned char theIndex,
					char *theRealmNamePtr) {
	rpRealmPtr		theRealmPtr;

#if RomPagerDebug
	if (theIndex >= kRpNumberOfRealms) {
		RP_PRINTF("Invalid realm index\n");
		return;
	}
#endif

	theRealmPtr = &((rpDataPtr) theTaskDataPtr)->fRealms[theIndex];
	RP_STRCPY(theRealmPtr->fRealmName, theRealmNamePtr);

	return;
}


rpSecurityLevel	RpGetSecurityLevel(void *theTaskDataPtr,
									unsigned char theIndex) {
	rpRealmPtr	theRealmPtr;

#if RomPagerDebug
	if (theIndex >= kRpNumberOfRealms) {
		RP_PRINTF("Invalid realm index\n");
		return eRpSecurity_Disabled;
	}
#endif

	theRealmPtr = &((rpDataPtr) theTaskDataPtr)->fRealms[theIndex];
	return theRealmPtr->fSecurityLevel;
}


void RpSetSecurityLevel(void *theTaskDataPtr, unsigned char theIndex,
						rpSecurityLevel theSecurityLevel) {
	rpRealmPtr	theRealmPtr;

#if RomPagerDebug
	if (theIndex >= kRpNumberOfRealms) {
		RP_PRINTF("Invalid realm index\n");
		return;
	}
#endif

	theRealmPtr = &((rpDataPtr) theTaskDataPtr)->fRealms[theIndex];
	theRealmPtr->fSecurityLevel = theSecurityLevel;

	return;
}


#if RomPagerExternalPassword || RomPagerIpp || RomPagerSecure

rpRealmPtr RpGetLeastSecureRealm(rpDataPtr theDataPtr,
									rpAccess theAccessCode) {
	Unsigned8		theCount;
	rpAccess		theRealmFlag;
	rpRealmPtr		theLeastSecureRealmPtr;
	rpRealmPtr		theRealmPtr;

	theLeastSecureRealmPtr = (rpRealmPtr) 0;
	theRealmPtr = theDataPtr->fRealms;
	theRealmFlag = kRpPageAccess_Realm1;
	theCount = kRpNumberOfRealms;

	while (theCount > 0) {
		if (theRealmFlag & theAccessCode) {
			/*
				This realm is enabled in theAccessCode.
			*/
			if (theLeastSecureRealmPtr == (rpRealmPtr) 0) {
				theLeastSecureRealmPtr = theRealmPtr;
			}
			else {
				if (theRealmPtr->fSecurityLevel <
						theLeastSecureRealmPtr->fSecurityLevel) {
					theLeastSecureRealmPtr = theRealmPtr;
				}
			}
		}
		theRealmPtr++;
		theRealmFlag = theRealmFlag << 1;
		theCount--;
	}

	return theLeastSecureRealmPtr;
}

#endif	/* RomPagerExternalPassword || RomPagerIpp || RomPagerSecure */


/*****************************************************************************
	Session manipulation routines
*****************************************************************************/

/*
	This routine returns True if any realm specified by the access code
	has an active session.
*/

Boolean RpCheckSession(void *theTaskDataPtr, rpAccess theAccessCode) {
	Unsigned8		theCount;
	rpAccess		theRealmFlag;
	rpRealmPtr		theRealmPtr;

	theRealmPtr = ((rpDataPtr) theTaskDataPtr)->fRealms;
	theRealmFlag = kRpPageAccess_Realm1;
	theCount = kRpNumberOfRealms;

	while (theCount > 0) {
		if ((theAccessCode & theRealmFlag) &&
				theRealmPtr->fUserLock != (rpUserPtr) 0) {
			/*
				This realm is enabled in the access code and
				it has an active session.
			*/
			return True;
		}
		theRealmPtr++;
		theRealmFlag = theRealmFlag << 1;
		theCount--;
	}

	return False;
}


void RpResetCurrentSession(void *theTaskDataPtr,
							Signed16Ptr theIndexValuesPtr) {
	rpHttpRequestPtr	theRequestPtr;
	rpUserPtr			theUserPtr;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;

	if (theRequestPtr != (rpHttpRequestPtr) 0) {
		theUserPtr = theRequestPtr->fAuthenticatedUserPtr;
		ResetSession((rpDataPtr) theTaskDataPtr, theUserPtr);
#if RomPagerExternalPassword
		/*
			Delete the user entry from the local user database
			so we'll call the external password routine next
			time a request is made for this username.
		*/
		DeleteUser((rpDataPtr) theTaskDataPtr, theUserPtr);
#endif
	}

	return;
}


/*
	This routine is used to reset a security sesion.

	Inputs:
		theUsernamePtr		- pointer to a "C" string containing the
							  username
*/

void RpResetUserSession(void *theTaskDataPtr, char *theUsernamePtr) {
	rpUserPtr	theUserPtr;

	theUserPtr = FindUser((rpDataPtr) theTaskDataPtr, theUsernamePtr);
	ResetSession((rpDataPtr) theTaskDataPtr, theUserPtr);
#if RomPagerExternalPassword
	/*
		Delete the user entry from the local user database
		so we'll call the external password routine next
		time a request is made for this username.
	*/
	DeleteUser((rpDataPtr) theTaskDataPtr, theUserPtr);
#endif

	return;
}


void RpSetCurrentSessionCloseFunction(void *theTaskDataPtr,
			rpSessionCloseFuncPtr theFunctionPtr,
			void *theUserCookie) {
	rpHttpRequestPtr		theRequestPtr;
	rpUserPtr				theUserPtr;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;

	if (theRequestPtr != (rpHttpRequestPtr) 0) {
		theUserPtr = theRequestPtr->fAuthenticatedUserPtr;
		if (theUserPtr != (rpUserPtr) 0) {
			theUserPtr->fSessionCloseFuncPtr = theFunctionPtr;
			theUserPtr->fUserCookie = theUserCookie;
		}
	}

	return;
}


/*
	Set a function to be called whenever a new user security session
	is started.
*/

void RpSetSessionStartFunction(void *theTaskDataPtr,
								rpSessionStartFuncPtr theFunctionPtr) {

	((rpDataPtr) theTaskDataPtr)->fSessionStartFuncPtr = theFunctionPtr;

	return;
}


/*
	Set the default password session timeout value for the server.

	This timeout value is used when a user is being added if the
	timeout value passed in to the RpSetUserAttributes routine is 0.
*/

void RpSetServerPasswordTimeout(void *theTaskDataPtr,
								Signed32 theTimeoutSeconds) {

	((rpDataPtr) theTaskDataPtr)->fPasswordSessionTimeout = theTimeoutSeconds;
	return;
}


/*****************************************************************************
	User manipulation routines
*****************************************************************************/

void RpDeleteUser(void *theTaskDataPtr, char *theUsernamePtr) {
	rpUserPtr	theUserPtr;

	theUserPtr = FindUser((rpDataPtr) theTaskDataPtr, theUsernamePtr);
	DeleteUser((rpDataPtr) theTaskDataPtr, theUserPtr);

	return;
}


char *RpGetCurrentUserName(void *theTaskDataPtr) {
	rpHttpRequestPtr	theRequestPtr;
	rpUserPtr			theUserPtr;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;

	if (theRequestPtr != (rpHttpRequestPtr) 0) {
		theUserPtr = theRequestPtr->fAuthenticatedUserPtr;
		if (theUserPtr != (rpUserPtr) 0) {
			return theUserPtr->fUsername;
		}
	}

	/*
		If we got here, we didn't find a currently authenticated user.
	*/
	return (char *) 0;
}


void *RpGetPasswordCookie(void *theTaskDataPtr, char *theUsernamePtr) {
	rpUserPtr	theUserPtr;

	theUserPtr = FindUser((rpDataPtr) theTaskDataPtr, theUsernamePtr);

	if (theUserPtr == (rpUserPtr) 0) {
		return (void *) 0;
	}
	else {
		return theUserPtr->fPasswordCookie;
	}
}


void RpSetPasswordCookie(void *theTaskDataPtr,
							char *theUsernamePtr,
							void *theCookie) {
	rpUserPtr	theUserPtr;

	theUserPtr = FindUser((rpDataPtr) theTaskDataPtr, theUsernamePtr);

	if (theUserPtr != (rpUserPtr) 0) {
		theUserPtr->fPasswordCookie = theCookie;
	}

	return;
}


/*
	This routine returns the total number of users currently in
	the user database.
*/

Signed16 RpGetUserCount(void *theTaskDataPtr) {
	Unsigned16	theIndex;
	Signed16	theUserCount;
	rpUserPtr	theUserPtr;

	theUserPtr = ((rpDataPtr) theTaskDataPtr)->fUsers;
	theUserCount = 0;

	for (theIndex = 0; theIndex < kRpNumberOfUsers; theIndex++) {
		if (*theUserPtr->fUsername != '\0') {
			theUserCount++;
		}
		theUserPtr++;
	}

	return theUserCount;
}


Signed16 RpGetUserIndex(void *theTaskDataPtr, char *theUsernamePtr) {
	Signed16	theUserIndex;
	rpUserPtr	theUserPtr;

	theUserPtr = ((rpDataPtr) theTaskDataPtr)->fUsers;
	theUserIndex = 0;

	while (theUserIndex < kRpNumberOfUsers) {
		if (*theUserPtr->fUsername != '\0' &&
				RP_STRCMP(theUserPtr->fUsername, theUsernamePtr) == 0) {
			return theUserIndex;
		}
		theUserIndex++;
		theUserPtr++;
	}

	return (Signed16) -1;
}


/*
	This routine returns the attributes for the user referred to
	by theUserIndex.  NULL users in the list are skipped.
	If there is no user for the given index, a NULL theUsernamePtr
	is returned.
*/

void RpGetUserAttributes(void *theTaskDataPtr,
							Unsigned16 theUserIndex,
							char **theUsernamePtrPtr,
							char **thePasswordPtrPtr,
							rpAccess *theAccessCodePtr,
							Unsigned32 *theIpAddressPtr,
							Signed32 *theTimeoutSecondsPtr) {
	Unsigned16	theCount;
	rpUserPtr	theUserPtr;

	*theUsernamePtrPtr = (char *) 0;

	if (theUserIndex >= kRpNumberOfUsers) {
		return;
	}

	theUserPtr = ((rpDataPtr) theTaskDataPtr)->fUsers;
	theCount = kRpNumberOfUsers;

	while (theCount > 0) {
		if (*theUserPtr->fUsername != '\0') {
			/*
				We found a user entry in the list.
			*/
			if (theUserIndex == 0) {
				*theUsernamePtrPtr = theUserPtr->fUsername;
				*thePasswordPtrPtr = theUserPtr->fPassword;
				*theAccessCodePtr = theUserPtr->fAccessCode;
				*theIpAddressPtr = theUserPtr->fIpAddress;
				*theTimeoutSecondsPtr = theUserPtr->fSessionTimeout;
				break;
			}
			theUserIndex--;
		}
		theUserPtr++;
		theCount--;
	}

	return;
}


Boolean RpSetUserAttributes(void *theTaskDataPtr,
							char *theUsernamePtr,
							char *thePasswordPtr,
							rpAccess theAccessCode,
							StcpIpAddress theIpAddress,
							Signed32 theTimeoutSeconds) {
	Unsigned16	theCount;
	rpUserPtr	theTempUserPtr;
	rpUserPtr	theUserPtr;
#if RomPagerExternalPassword
	rpUserPtr	theOldestUserPtr;
#endif

	theUserPtr = FindUser((rpDataPtr) theTaskDataPtr, theUsernamePtr);

	if (theUserPtr == (rpUserPtr) 0) {
		/*
			This user is not already in the users list.
			See if we have a free user entry.
		*/
		theTempUserPtr = ((rpDataPtr) theTaskDataPtr)->fUsers;
		theCount = kRpNumberOfUsers;
#if RomPagerExternalPassword
		theOldestUserPtr = theTempUserPtr;
#endif

		while (theCount > 0) {
			if (*theTempUserPtr->fUsername == '\0') {
				/*
					We found an empty user entry.
				*/
				theUserPtr = theTempUserPtr;
				RP_STRCPY(theUserPtr->fUsername, theUsernamePtr);
				break;
			}
#if RomPagerExternalPassword
			else {
				/*
					This user entry is active.  Keep track of the
					least recently used user entry.
				*/
				if (theTempUserPtr->fLastUsedTime <
						theOldestUserPtr->fLastUsedTime) {
					theOldestUserPtr = theTempUserPtr;
				}
			}
#endif
			theTempUserPtr++;
			theCount--;
		}
	}
	else {
		/*
			This user already exists.  If there is an active session
			and the password, IP address, or realm access code is being
			changed, reset the session.
		*/
		if (theUserPtr->fHttpSessionTimer > 0) {
			if ((RP_STRCMP(theUserPtr->fPassword, thePasswordPtr) != 0) ||
					theUserPtr->fIpAddress != theIpAddress ||
					theUserPtr->fAccessCode != theAccessCode) {
				ResetSession((rpDataPtr) theTaskDataPtr, theUserPtr);
			}
		}
	}

#if RomPagerExternalPassword
	if (theUserPtr == (rpUserPtr) 0) {
		/*
			We didn't find a free user entry.  Delete the
			least recently used entry and re-use it.
		*/
		DeleteUser((rpDataPtr) theTaskDataPtr, theOldestUserPtr);
		theUserPtr = theOldestUserPtr;
		RP_STRCPY(theUserPtr->fUsername, theUsernamePtr);
	}
#endif

	if (theUserPtr != (rpUserPtr) 0) {
		RP_STRCPY(theUserPtr->fPassword, thePasswordPtr);
		theUserPtr->fAccessCode = theAccessCode;
		theUserPtr->fIpAddress = theIpAddress;

		if (theTimeoutSeconds == 0) {
			/*
				Use the server's default password session timeout.
			*/
			theUserPtr->fSessionTimeout =
					((rpDataPtr) theTaskDataPtr)->fPasswordSessionTimeout;
		}
		else {
			theUserPtr->fSessionTimeout = theTimeoutSeconds;
		}

		/*
			If we already have a session going for this user, make
			sure the current session timer is not greater than the
			new timeout value.
		*/
		if (theUserPtr->fHttpSessionTimer > theUserPtr->fSessionTimeout) {
			theUserPtr->fHttpSessionTimer = theUserPtr->fSessionTimeout;
		}

#if RomPagerExternalPassword
		theUserPtr->fLastUsedTime = RpGetSysTimeInSeconds(theTaskDataPtr);
#endif

		return True;
	}

	/*
		We didn't find the user in the list and there's no room
		to add another user.
	*/
	return False;
}


/*****************************************************************************
	Internal Security Session (CLI) support routines
*****************************************************************************/

/*
	This routine is used by features other than the Web Server
	(CLI for example) to determine whether a user has valid
	credentials to begin a security session.  It only supports a
	basic security scheme with username, password, and IP address.

	Inputs:
		theConnection		- the index of the connection on which the
							  request is being made
		theUsernamePtr		- pointer to a "C" string containing the
							  username
		thePasswordPtr		- pointer to a "C" string containing the
							  password
		theIpAddress		- the IP address from which the request is
							  being made
		theAccessCodePtr	- pointer to a location for returning the
							  realm access code

	Returns:
		eRpPasswordAuthorized		- the credentials are valid
		eRpPasswordNotAuthorized	- the credentials are invalid
		eRpPasswordPending			- authentication has not yet completed,
									  this routine should be called again,
									  later
		theSessionFlag				- True, there is aready a session
										False, there is no session yet
		*theAccessCodePtr			- the realm access code for which the
									  user has been authenticated
*/

rpPasswordState RpCheckAuthorization(void *theTaskDataPtr,
										Unsigned16 theConnection,
										char *theUsernamePtr,
										char *thePasswordPtr,
										Unsigned32 theIpAddress,
										rpAccess *theAccessCodePtr,
										Boolean *theSessionFlag) {
	rpAccess			theAccessCode;
	rpDataPtr			theDataPtr;
	rpPasswordState		thePasswordState;
	rpUserPtr			theUserPtr;
#if RomPagerExternalPassword
	Unsigned32			theExtIpAddress;
	char				thePassword[kMaxNameLength];
#endif

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	*theAccessCodePtr = kRpPageAccess_Unprotected;
	*theSessionFlag = False;
	theAccessCode = kRpPageAccess_Unprotected;
	thePasswordState = eRpPasswordNotAuthorized;

	theUserPtr = FindUser(theDataPtr, theUsernamePtr);

#if RomPagerExternalPassword
	if (theUserPtr == (rpUserPtr) 0) {
		RP_STRCPY(thePassword, thePasswordPtr);
		theExtIpAddress = 0;

		thePasswordState = SpwdGetExternalPassword(theTaskDataPtr,
													theConnection,
													theUsernamePtr,
													thePassword,
													&theExtIpAddress,
													&theAccessCode);

		if (thePasswordState == eRpPasswordAuthorized ||
				thePasswordState == eRpPasswordDone) {

			(void) RpSetUserAttributes(theTaskDataPtr,
							theUsernamePtr,
							thePassword,
							theAccessCode,
							theExtIpAddress,
							theDataPtr->fPasswordSessionTimeout);

			theUserPtr = FindUser(theDataPtr, theUsernamePtr);
		}
	}
#endif	/* RomPagerExternalPassword */

	if (theUserPtr != (rpUserPtr) 0) {
		if ((RP_STRCMP(theUsernamePtr, theUserPtr->fUsername) == 0) &&
			(RP_STRCMP(thePasswordPtr, theUserPtr->fPassword) == 0)) {

			theAccessCode = GetAuthRealms(theDataPtr,
											theUserPtr,
											theIpAddress);

			if (theAccessCode != kRpPageAccess_Unprotected) {
				thePasswordState = eRpPasswordAuthorized;
			}

			/*
				Find out if there's already a session.
			*/
			if (theUserPtr->fHttpSessionCode != kRpPageAccess_Unprotected) {
				*theSessionFlag = True;
			}
		}
	}

	*theAccessCodePtr = theAccessCode;

	return thePasswordState;
}


/*
	This routine is used to begin a security session once it has been
	determined that a user has valid credentials.

	Inputs:
		theUsernamePtr		- pointer to a "C" string containing the
							  username
		theAccessCode		- realm access code indicating the realm(s) for
							  which the secrurity session is to be started

	Returns:
		True				- a security session hase been started
		False				- a security session has not been started.  this
							  can happen for several reasons:
								1 - there are no realms enabled in the
									access code or it is invalid for
									the requested user
								2 - the username was not recognized
								3 - a realm needed to begin the security
									session is already in use
*/

Boolean RpStartUserSession(void *theTaskDataPtr,
							char *theUsernamePtr,
							rpAccess theAccessCode) {
	rpPasswordState		thePasswordState;
	rpUserPtr			theUserPtr;

	if (theAccessCode != kRpPageAccess_Unprotected) {

		theUserPtr = FindUser((rpDataPtr) theTaskDataPtr, theUsernamePtr);

		if (theUserPtr != (rpUserPtr) 0) {
			thePasswordState = LockRealms((rpDataPtr) theTaskDataPtr,
											theUserPtr, theAccessCode);

			if (thePasswordState == eRpPasswordAuthorized) {
				/*
					We didn't find any realm conflicts.
				*/
				theUserPtr->fInternalSessionCode |= theAccessCode;
#if RomPagerExternalPassword
				theUserPtr->fLastUsedTime =
						RpGetSysTimeInSeconds(theTaskDataPtr);
#endif
				return True;
			}
		}
	}

	return False;
}


#if RomPagerUserExit

rpPasswordState RpCheckUserExitAuthorization(void *theTaskDataPtr,
										Unsigned16 theConnection,
										char *theUsernamePtr,
										char *thePasswordPtr,
										Unsigned32 theIpAddress,
										rpAccess *theAccessCodePtr) {
	rpDataPtr			theDataPtr;
	rpPasswordState		thePasswordState;
	rpHttpRequestPtr	theRequestPtr;
	rpAccess			theAuthRealms;
	rpAccess			theSaveAccessCode;
	rpUserPtr			theUserPtr;
#if RomPagerExternalPassword
	rpConnectionPtr 	theConnectionPtr;
	rpRealmPtr 			theRealmPtr;
#endif

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	thePasswordState = eRpPasswordNotAuthorized;
	theSaveAccessCode = *theAccessCodePtr;
	*theAccessCodePtr = kRpPageAccess_Unprotected;

	/*
		RomPager's internal security routines require the credentials
		to be in the request structure, so copy them in if they aren't
		already there.
	*/
	if (theUsernamePtr != theRequestPtr->fUsername) {
		RpStrLenCpy(theRequestPtr->fUsername,
					theUsernamePtr,
					kRpMaxUserNameLength);
	}
	if (thePasswordPtr != theRequestPtr->fPassword) {
		RpStrLenCpy(theRequestPtr->fPassword,
					thePasswordPtr,
					kRpMaxPasswordLength);
	}

	if (*theUsernamePtr != '\0') {
		/*
			A username was supplied, see if it's in the user database.
		*/
		theUserPtr = FindUser(theDataPtr, theRequestPtr->fUsername);

#if RomPagerExternalPassword
		if (theUserPtr == (rpUserPtr) 0) {
			/*
				The requesting user isn't in the local user database.
				Check it externally.
			*/
			theRealmPtr = RpGetLeastSecureRealm(theDataPtr, theSaveAccessCode);
			theRequestPtr->fRealmPtr = theRealmPtr;
			theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
			theConnectionPtr->fState = eRpConnectionWaitingUserExit;
			RpHandleExternalSecurity(theConnectionPtr);

			if (theConnectionPtr->fState == eRpConnectionNeedsProtocolAction) {
				/*
					The external password routine must have returned
					no longer pending.  See if the requesting User
					has been added to the user database.
				*/
				theUserPtr = FindUser(theDataPtr, theRequestPtr->fUsername);
			}
			else {
				/*
					The external password routine must have returned
					pending, so just return pending and we'll get
					called again later.
				*/
				return eRpPasswordPending;
			}
		}
#endif	/* RomPagerExternalPassword */

		if (theUserPtr != (rpUserPtr) 0) {
			/*
				The requesting user is in the user database.
				See if the requestor has valid credentials for
				access to the user entry.
			*/
			theAuthRealms = CheckUserAuth(theDataPtr, theUserPtr);

			/*
				Return the realm access code for user.
			*/
			*theAccessCodePtr = theAuthRealms;

			if ((theAuthRealms & theSaveAccessCode) !=
					kRpPageAccess_Unprotected) {
				thePasswordState = eRpPasswordAuthorized;
			}
		}
		/*
			else, the requesting user isn't in the local user
			database.  Leave thePasswordState set to
			eRpPasswordNotAuthorized to force a challenge.
		*/
	}

	if (thePasswordState == eRpPasswordNotAuthorized) {
		/*
			We need to set up a challenge.
		*/
		SetupChallenge(theDataPtr, theSaveAccessCode, thePasswordState);
	}

	return thePasswordState;
}


/*
	This routine is used to begin a security session once it has been
	determined that a user has valid credentials.

	Inputs:
		theUsernamePtr		- pointer to a "C" string containing the
							  username
		theAccessCode		- realm access code indicating the realm(s) for
							  which the secrurity session is to be started

	Returns:
		rpPasswordState:
			eRpPasswordAuthorized		- the session has been started
			eRpPasswordNotAuthorized	- the user does not already have an
											active security session and there
											is no challenge pending or the
											user does not have access to the
											realm(s) requested in theAccessCode
			eRpPasswordBusy				- a realm requested in theAccessCode
											is already in use by another user
*/

rpPasswordState RpStartUserExitSession(void *theTaskDataPtr,
										char *theUsernamePtr,
										rpAccess theAccessCode) {
	rpDataPtr			theDataPtr;
	rpPasswordState		thePasswordState;
	rpUserPtr			theUserPtr;

	thePasswordState = eRpPasswordNotAuthorized;
	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theUserPtr = FindUser(theDataPtr, theUsernamePtr);

	if (theUserPtr != (rpUserPtr) 0) {
		thePasswordState = CheckStartSession(theDataPtr, theUserPtr,
												theAccessCode);
		if (thePasswordState == eRpPasswordAuthorized) {
			/*
				There's already a session or there's a pending
				challenge, it's okay to start the session.
			*/
			thePasswordState = StartSession(theDataPtr, theUserPtr,
											theAccessCode);
		}
	}

	if (thePasswordState != eRpPasswordAuthorized) {
		/*
			We need to set up a challenge.
		*/
		SetupChallenge(theDataPtr, theAccessCode, thePasswordState);
	}

	return thePasswordState;
}

#endif	/* RomPagerUserExit */


/*
	Once a username and password have been validated, this
	routine is called to see what realms the user is allowed
	access to, given the IP address and realm security levels.
*/

static rpAccess GetAuthRealms(rpDataPtr theDataPtr, rpUserPtr theUserPtr,
								Unsigned32 theIpAddress) {
	Unsigned8		theCount;
	rpRealmPtr		theRealmPtr;
	rpAccess		theAccessCode;
	rpAccess		theRealmFlag;

	theAccessCode = theUserPtr->fAccessCode;

	if (theUserPtr->fIpAddress != 0 &&
			theIpAddress != theUserPtr->fIpAddress) {
		/*
			The requestor's IP address doesn't match the user entry's
			IP address, so the requestor is only allowed access to the
			realms for the user entry that do not have an IP address
			security level.
		*/
 		theRealmPtr = theDataPtr->fRealms;
		theRealmFlag = kRpPageAccess_Realm1;
		theCount = kRpNumberOfRealms;

		while (theCount > 0) {
			if (theAccessCode & theRealmFlag) {
				if (theRealmPtr->fSecurityLevel & kRpSecurityIpAddress) {
					/*
						This security level requires an IP address.
						Since the requestor didn't supply the
						correct IP address, they are not
						authorized for this realm.
					*/
					theAccessCode &= ~theRealmFlag;
				}
			}
			theRealmPtr++;
			theRealmFlag = theRealmFlag << 1;
			theCount--;
		}
	}
	/*
		else, the requestor's IP address does match the user entry's
		IP address, so the requestor is allowed access to all realms
		for that user entry.
	*/

	return theAccessCode;
}


/*****************************************************************************
	Local (static) support routines
*****************************************************************************/

/*
	This routine is used to validate the username/password from the
	request against the username/password string pointers passed in.
	This routine determines whether to do Basic or Digest authorization.

	Returns:
		True			authorized
		False			not authorized
*/

static Boolean CheckAuthorization2(rpDataPtr theDataPtr,
									rpUserPtr theUserPtr) {
	Boolean				theAuthorizedFlag;
	rpRealmPtr			theRealmPtr;
	rpHttpRequestPtr	theRequestPtr;
	Unsigned32			theRemoteAddress;

	theAuthorizedFlag = False;
	theRemoteAddress = theDataPtr->fCurrentConnectionPtr->fIpRemote;
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theRealmPtr = theRequestPtr->fRealmPtr;

	if (theRealmPtr->fSecurityLevel & kRpSecurityBasic) {
		/*
			Basic authentication.
		*/
		if ((RP_STRCMP(theRequestPtr->fUsername, theUserPtr->fUsername) == 0) &&
			(RP_STRCMP(theRequestPtr->fPassword, theUserPtr->fPassword) == 0)) {
			theAuthorizedFlag = True;
		}
	}
#if RomPagerSecurityDigest
	else if (theRealmPtr->fSecurityLevel & kRpSecurityDigest) {
		/*
			Digest authentication.
		*/
		theAuthorizedFlag = CheckDigestAuth(theRequestPtr, theUserPtr,
											theRemoteAddress);
	}
#endif

	if (theAuthorizedFlag) {
		/*
			The username/password verified, now see
			if we have to check the IP address.
		*/
		if (theRealmPtr->fSecurityLevel & kRpSecurityIpAddress) {
			/*
				Even though the realm's security level requires an
				IP address, if the IP address in the user structure
				is NULL, we'll accept any IP address.
			*/
			if ((theUserPtr->fIpAddress != (Unsigned32) 0) &&
					(theRemoteAddress != theUserPtr->fIpAddress)) {
				theAuthorizedFlag = False;
			}
		}
	}

#if RomPagerSecure
	if (theAuthorizedFlag) {
		/*
			The username/password verified, and the IP address
			matched (if we're checking it), now see if we have
			a secure connection (if one is required).
		*/
		if (theRealmPtr->fSecurityLevel & kRpSecuritySSL) {
			if (!theDataPtr->fCurrentConnectionPtr->fIsTlsFlag) {
				theAuthorizedFlag = False;
			}
		}
	}
#endif

	return theAuthorizedFlag;
}


/*
	This routine is called to check access to a realm once it has been
	determined that the user exists in the user database and the request
	has valid credentials for that user.

	Inputs:
		theUserPtr			- pointer to authorized user entry
		theAccessCode		- realm(s) that the object belongs to
		theAuthRealms		- realm(s) that the user is authorized for

	Returns:
		eRpPasswordAuthorized
							- the user has access to a realm in
								theAccessCode, the realm is available,
								and a session has been started
		eRpPasswordNotAuthorized
							- the user session is not already active
								and there is not an active challenge
		eRpPasswordBusy		- realm locking is enabled and the realm
								needed to start the session is already
								in use
*/

static rpPasswordState CheckRealmAuth(rpDataPtr theDataPtr,
										rpUserPtr theUserPtr,
										rpAccess theAccessCode,
										rpAccess theAuthRealms) {
	rpPasswordState		thePasswordState;

	if (theAccessCode & theAuthRealms) {
		/*
			This user has access to a realm required by the access code.
			Make sure there's already an active session or that
			there's a pending challenge.
		*/
		thePasswordState = CheckStartSession(theDataPtr, theUserPtr,
												theAuthRealms);

		if (thePasswordState != eRpPasswordAuthorized) {
			return thePasswordState;
		}

		thePasswordState = StartSession(theDataPtr, theUserPtr,
										theAuthRealms);
	}
	else {
		thePasswordState = eRpPasswordNotAuthorized;
	}

	return thePasswordState;
}


/*
	This routine returns True if any realm specified by the theAccessCode
	has been disabled or has a blank password.  If so, the calling routine
	will allow access to the object.
*/

static Boolean CheckRealmDisabled(rpDataPtr theDataPtr,
									rpAccess theAccessCode) {
	Unsigned8		theCount;
	rpAccess		theRealmFlag;
	rpRealmPtr 		theRealmPtr;

	theRealmPtr = theDataPtr->fRealms;
	theRealmFlag = kRpPageAccess_Realm1;
	theCount = kRpNumberOfRealms;

	while (theCount > 0) {
		if (theAccessCode & theRealmFlag) {
			if (theRealmPtr->fSecurityLevel == eRpSecurity_Disabled) {
				return True;
			}
		}
		theRealmPtr += 1;
		theRealmFlag = theRealmFlag << 1;
		theCount--;
	}

	return False;
}


/*
	CheckStartSession

	Determine whether or not it's okay to start a session or
	keep an existing session going.

	If a session already exists, it's okay to keep it going.

	If a session doesn't already exist, in order to start one,
	there has to be a challenge pending for the requested object
	in a realm that the validated user has access to.
*/

static rpPasswordState CheckStartSession(rpDataPtr theDataPtr,
											rpUserPtr theUserPtr,
											rpAccess theAuthRealms) {
	Unsigned8			theCount;
	rpRealmPtr 			theRealmPtr;
	rpAccess			theRealmFlag;

	if (theUserPtr->fHttpSessionTimer > 0) {
		/*
			There's already a session, it's okay to keep it going.
		*/
		return eRpPasswordAuthorized;
	}
	else {
		/*
			There's not already an existing session.  Walk through
			the realms that the validated user has access to and
			see if there is a pending challenge for the requested
			object.
		*/
		theRealmPtr = theDataPtr->fRealms;
		theRealmFlag = kRpPageAccess_Realm1;
		theCount = kRpNumberOfRealms;

		while (theCount > 0) {
			if ((theAuthRealms & theRealmFlag) &&
					(theRealmPtr->fChallengeTimeout > 0)) {
				/*
					The user has access to this realm and
					the realm has a pending challenge.
				*/
				return eRpPasswordAuthorized;
			}
			theRealmPtr++;
			theRealmFlag = theRealmFlag << 1;
			theCount--;
		}
	}

	return eRpPasswordNotAuthorized;
}


/*
	Using the user's credentials and the realms' security levels,
	walk through the realms that the user has access to and find
	the ones that the user is authorized for.

	Returns:

		rpAccess		- a realm access code (bit-map) indicating
						  which realm(s) the user is authorized for.
*/

static rpAccess CheckUserAuth(rpDataPtr theDataPtr, rpUserPtr theUserPtr) {
	Unsigned8			theCount;
	rpAccess			theAuthRealms;
	rpAccess			theRealmFlag;
	rpRealmPtr			theRealmPtr;
	rpHttpRequestPtr	theRequestPtr;

	theCount = kRpNumberOfRealms;
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theRealmPtr = theDataPtr->fRealms;
	theRealmFlag = kRpPageAccess_Realm1;
	theAuthRealms = kRpPageAccess_Unprotected;

	while (theCount > 0) {
		if (theRealmFlag & theUserPtr->fAccessCode) {
			/*
				This realm is enabled in the user's access code.
			*/
			theRequestPtr->fRealmPtr = theRealmPtr;

			if (CheckAuthorization2(theDataPtr, theUserPtr) == True) {
				theAuthRealms |= theRealmFlag;
			}
		}
		theRealmPtr++;
		theRealmFlag = theRealmFlag << 1;
		theCount--;
	}

	return theAuthRealms;
}


static void ClearRealmLocks(rpDataPtr theDataPtr, rpUserPtr theUserPtr) {
	Unsigned8	theCount;
	rpRealmPtr	theRealmPtr;

	/*
		Clear any realm locks for this user.
	*/
	theRealmPtr = theDataPtr->fRealms;
	theCount = kRpNumberOfRealms;

	while (theCount > 0) {
		if (theRealmPtr->fUserLock == theUserPtr) {
			theRealmPtr->fUserLock = (rpUserPtr) 0;
		}
		theRealmPtr++;
		theCount--;
	}

	return;
}


static void DeleteUser(rpDataPtr theDataPtr, rpUserPtr theUserPtr) {

	if (theUserPtr != (rpUserPtr) 0) {
		ClearRealmLocks(theDataPtr, theUserPtr);
		RP_MEMSET(theUserPtr, 0, sizeof(rpUser));
	}

	return;
}


static rpUserPtr FindUser(rpDataPtr theDataPtr, char *theUsername) {
	Unsigned16	theCount;
	rpUserPtr	theUserPtr;

	theUserPtr = theDataPtr->fUsers;
	theCount = kRpNumberOfUsers;

	while (theCount > 0) {
		if (*theUserPtr->fUsername != '\0' &&
				RP_STRCMP(theUserPtr->fUsername, theUsername) == 0) {
			return theUserPtr;
		}
		theUserPtr++;
		theCount--;
	}

	return (rpUserPtr) 0;
}


static rpPasswordState LockRealms(rpDataPtr theDataPtr, rpUserPtr theUserPtr,
									rpAccess theRealmLockCode) {
	Unsigned8			theCount;
	rpPasswordState			thePasswordState;
	rpAccess				theRealmFlag;
	rpRealmPtr				theRealmPtr;

	/*
		Don't allow the caller to lock any realms that the user
		doesn't have access to.
	*/
	if ((theUserPtr->fAccessCode & theRealmLockCode) !=
			theRealmLockCode) {
		return eRpPasswordNotAuthorized;
	}

	thePasswordState = eRpPasswordAuthorized;
	theRealmPtr = theDataPtr->fRealms;
	theRealmFlag = kRpPageAccess_Realm1;
	theCount = kRpNumberOfRealms;

	while (theCount > 0) {
		if (theRealmLockCode & theRealmFlag) {
			/*
				This is one of the realms we want to lock.
			*/
			if (theDataPtr->fRealmLockingFlag) {
				/*
					Realm locking is on.  See if this realm
					is already locked.
				*/
				if ((theRealmPtr->fUserLock != (rpUserPtr) 0) &&
						(theRealmPtr->fUserLock != theUserPtr)) {
					/*
						This realm is locked to a different user.
					*/
					thePasswordState = eRpPasswordBusy;
					break;
				}
				else if (theRealmPtr->fUserLock == (rpUserPtr) 0) {
					/*
						This realm is free, lock it to this user.
					*/
					theRealmPtr->fUserLock = theUserPtr;

#if RomPagerSecurityDigest
					if (*theRealmPtr->fChallengeNonce != '\0') {
						RP_STRCPY(theRealmPtr->fNonce,
									theRealmPtr->fChallengeNonce);
						*theRealmPtr->fChallengeNonce = '\0';
					}
#endif	/* RomPagerSecurityDigest */

				}
				/*
					else, this realm must already be locked to this user.
				*/
			}
			else {
				/*
					Realm locking is not on, but the fUserLock is used
					to indicate that the realm is enabled (somebody
					with access to the realm has an active session),
					so set the fUserLock.
				*/
				theRealmPtr->fUserLock = theUserPtr;

#if RomPagerSecurityDigest
				if (*theRealmPtr->fChallengeNonce != '\0') {
					RP_STRCPY(theRealmPtr->fNonce,
								theRealmPtr->fChallengeNonce);
					*theRealmPtr->fChallengeNonce = '\0';
				}
#endif	/* RomPagerSecurityDigest */

			}
		}
		theRealmPtr++;
		theRealmFlag = theRealmFlag << 1;
		theCount--;
	}

	if (thePasswordState != eRpPasswordAuthorized) {
		ClearRealmLocks(theDataPtr, theUserPtr);
	}

	return thePasswordState;
}


/*
	Reset a user session.
*/

static void ResetSession(rpDataPtr theDataPtr, rpUserPtr theUserPtr) {
	rpSessionCloseFuncPtr	theFunctionPtr;

	if (theUserPtr != (rpUserPtr) 0) {
#if RomPagerSecurityDigest
		ClearNonces(theDataPtr, theUserPtr->fHttpSessionCode);
#endif
		ClearRealmLocks(theDataPtr, theUserPtr);
		theUserPtr->fHttpSessionCode = kRpPageAccess_Unprotected;
		theUserPtr->fInternalSessionCode = kRpPageAccess_Unprotected;
		theUserPtr->fHttpSessionTimer = 0;

		theFunctionPtr = theUserPtr->fSessionCloseFuncPtr;

		if (theFunctionPtr != (rpSessionCloseFuncPtr) 0) {
			theFunctionPtr(theDataPtr, theUserPtr->fUserCookie);
			theUserPtr->fSessionCloseFuncPtr = (rpSessionCloseFuncPtr) 0;
		}
	}

	return;
}


static void SetupChallenge(rpDataPtr theDataPtr, rpAccess theAccessCode,
							rpPasswordState thePasswordState) {
	rpHttpRequestPtr		theRequestPtr;
	Unsigned8				theCount;
	rpAccess				theRealmFlag;
	rpRealmPtr				theLeastSecureRealmPtr;
	rpRealmPtr				theRealmPtr;

	/*
		Go through the realms, setting up a challenge for any
		realm that controls the object.  Also, keep track of the
		least secure realm.
	*/
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theLeastSecureRealmPtr = (rpRealmPtr) 0;
	theRealmPtr = theDataPtr->fRealms;
	theRealmFlag = kRpPageAccess_Realm1;
	theCount = kRpNumberOfRealms;

	while (theCount > 0) {
		if (theRealmFlag & theAccessCode) {
			/*
				The object is controlled by this realm.
			*/
			theRealmPtr->fChallengeTimeout =
					theDataPtr->fPasswordSessionTimeout;

			if (theLeastSecureRealmPtr == (rpRealmPtr) 0) {
				theLeastSecureRealmPtr = theRealmPtr;
			}
			else {
				if (theRealmPtr->fSecurityLevel <
						theLeastSecureRealmPtr->fSecurityLevel) {
					theLeastSecureRealmPtr = theRealmPtr;
				}
			}
		}
		theRealmPtr++;
		theRealmFlag = theRealmFlag << 1;
		theCount--;
	}

	/*
		Save away the realm to use for the challenge.
	*/
	theRequestPtr->fRealmPtr = theLeastSecureRealmPtr;

	/*
		Set up the type of challenge response (Basic, Digest, forbidden)
		and the object to send with the challenge.
	*/
	if (thePasswordState == eRpPasswordBusy) {
		/*
			This is a server busy response.  A needed realm
			is in use and locked by another user.  It doesn't
			matter whether the security level is Basic or Digest,
			we need to return a server busy response.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpForbidden;
		theRequestPtr->fObjectPtr = &gRpServerBusyPage;
		return;
	}

#if RomPagerSecurityDigest
	if (theLeastSecureRealmPtr->fSecurityLevel & kRpSecurityDigest) {
		/*
			Set up a Digest challenge.
		*/
		if (theRequestPtr->fClientSupportsDigest) {
			theRequestPtr->fHttpResponseState = eRpHttpNeedDigestAuthorization;
			theRequestPtr->fObjectPtr = &gRpAccessNotAllowedPage;
		}
		else {
			theRequestPtr->fHttpResponseState = eRpHttpForbidden;
			theRequestPtr->fObjectPtr = &gRpDigestUnsupportedPage;
		}
		return;
	}
#endif	/* RomPagerSecurityDigest */

	/*
		Set up a Basic challenge.
	*/
	theRequestPtr->fHttpResponseState = eRpHttpNeedBasicAuthorization;
	theRequestPtr->fObjectPtr = &gRpAccessNotAllowedPage;

	return;
}


/*
	The request has been authorized.  Start a session.

	Returns:

		eRpPasswordAuthorized			- the session has been started
		eRpPasswordNotAuthorized		- a realm needed by the user is
										  already in use
*/

static rpPasswordState StartSession(rpDataPtr theDataPtr,
									rpUserPtr theUserPtr,
									rpAccess theAuthRealms) {
	Unsigned8				theCount;
	rpSessionStartFuncPtr	theFunctionPtr;
	Boolean					theNewSessionFlag;
	rpPasswordState			thePasswordState;
	rpAccess				theRealmFlag;
	rpRealmPtr				theRealmPtr;
	rpHttpRequestPtr		theRequestPtr;

	/*
		Handle any realm locking.
	*/
	thePasswordState = LockRealms(theDataPtr, theUserPtr, theAuthRealms);

	if (thePasswordState == eRpPasswordAuthorized) {
		/*
			LockRealms didn't find any realm conflicts.
			Determine whether or not this is a new session.
		*/
		if (theUserPtr->fHttpSessionTimer == 0) {
			theNewSessionFlag = True;
		}
		else {
			theNewSessionFlag = False;
		}

		/*
			Start the session.
		*/
		theUserPtr->fHttpSessionCode = theAuthRealms;
		theUserPtr->fHttpSessionTimer = theUserPtr->fSessionTimeout;
#if RomPagerExternalPassword
		theUserPtr->fLastUsedTime = RpGetSysTimeInSeconds(theDataPtr);
#endif
		theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
		if (theRequestPtr != (rpHttpRequestPtr) 0) {
			theRequestPtr->fAuthenticatedUserPtr = theUserPtr;
		}

		if (theNewSessionFlag) {
			/*
				This is a new session, reset the challenge timers
				for any realms that this user has access to.
			*/
			theRealmPtr = theDataPtr->fRealms;
			theRealmFlag = kRpPageAccess_Realm1;
			theCount = kRpNumberOfRealms;
			while (theCount > 0) {
				if (theUserPtr->fAccessCode & theRealmFlag) {
					/*
						The user has access to this realm.
					*/
					theRealmPtr->fChallengeTimeout = 0;
				}
				theRealmPtr++;
				theRealmFlag = theRealmFlag << 1;
				theCount--;
			}

			/*
				This is a new session, call the start session
				callback function.
			*/
			theFunctionPtr = theDataPtr->fSessionStartFuncPtr;
			if (theFunctionPtr != (rpSessionStartFuncPtr) 0) {
				theFunctionPtr(theDataPtr,
								theUserPtr->fUsername,
								theUserPtr->fPassword,
								theAuthRealms);
			}
		}
	}

	return thePasswordState;
}

#endif	/* RomPagerServer && RomPagerSecurity */
