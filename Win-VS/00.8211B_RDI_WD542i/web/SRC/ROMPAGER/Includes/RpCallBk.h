/*
 *	File:		RpCallBk.h
 *
 *	Contains:	Call back and engine exit routine prototypes
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Copyright:	© 1995-2003 by Allegro Software Development Corporation
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
 *		09/19/03	pjr		add RpSetRemoteAuthenticateInfo prototypes
 *		09/03/03	pjr		add RpGetUserCount
 *		07/24/03	bva		bump connection size for delayed function routines
 *		07/13/03	bva		add RpSetRedirectAbsolute
 * * * * Release 4.21  * * *
 * * * * Release 4.20  * * *
 *		12/18/02	bva		bump theConnection size for RpCheckAuthorization
 *		12/17/02	bva		increase timer sizes for various calls
 *		11/25/02	bva		alignment
 *		09/24/02	pjr		add RpGetUserIndex prototype
 *		09/04/02	pjr		add rpPasswordState enum values, RpSetSession -
 *							StartFunction, RpCheckUserExitAuthorization and
 *							RpStartUserExitSession prototypes.  move
 *							RpCheckAuthorization and RpStartUserSession
 *							prototypes from RpIntPrt.h
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *		07/25/02	bva		add RpGetSoapAction
 * * * * Release 4.10  * * *
 *		06/08/02	bva		remove RomPagerSlaveIdentity
 * * * * Release 4.07  * * *
 * * * * Release 4.03  * * *
 *		10/30/01	bva		remove conditionals for RpSetConnectionClose
 *		09/14/01	rhb		add RpSetConnectionCloseFunction
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		08/07/01	pjr		make theUserIndex for RpGetUserAttributes 16 bits
 *		02/15/01	bva		remove dictionary patching
 *		02/14/01	rhb		rename rpItemError to asItemError
 *		02/13/01	rhb		rename RomPagerUse64BitIntegers to AsUse64BitIntegers
 *		12/04/00	pjr		add AsSetVarAccessItemList
 *		11/07/00	pjr		add theTaskDataPtr to time access routines
 *		10/06/00	pjr		add a realm access code to RpSetIppPrinterName
 *							and remove RpSetIppAccess (obsolete)
 *		08/31/00	pjr		modify for the new security model
 *		06/29/00	pjr		add support for multiple IPP printer names
 * * * * Release 3.10  * * *
 *		05/30/00	bva		fix compile checks for SH1 compiler
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.06  * * *
 * * * * Release 3.05  * * *
 *		10/21/99	bva		comment changes
 * * * * Release 3.04  * * *
 * * * * Release 3.01  * * *
 *		04/13/99	bva		add RpGetCurrentConnection
 * * * * Release 3.0 * * * *
 *		02/27/99	bva		add HTTP Cookie support
 *		02/23/99	pjr		add RpGetMonthDayYearInSeconds
 *		02/08/99	bva		add RpGetHostName
 *		01/31/99	bva		add RpSetRequestUserPhraseDictionary
 * * * * Release 2.2 * * * *
 *		11/14/98	bva		add RpSetRomObjectList
 *		11/11/98	bva		add RpSetServerPasswordTimeout
 *		10/20/98	bva		add RpSetIppAccess, RpSetPutAccess
 *		08/31/98	pjr		add RpGetFileUploadStatus
 *		06/16/98	bva		add RpSetSlaveIdentity
 *		06/08/98	bva		add support for multiple Remote Hosts
 *		06/04/98	bva		add RpSetCurrentConnection
 * * * * Release 2.1 * * * *
 *		05/22/98	bva		add RpSetRedirect, RpGetCurrentUrl
 *		05/14/98	bva		add RpGetFormBufferPtr
 *		04/28/98	bva		add RpSetIppPrinterName
 *		01/23/98	pjr		add prototypes for RpPhraseDictPatching feature.
 *		01/06/98	pjr		add prototypes for RomPagerLogging feature.
 * * * * Release 2.0 * * * *
 *		12/03/97	bva		move RpGetFormItem and RpReceiveItem from RomPager.h
 *		11/10/97	rhb		add support for 64 bit integers
 *		11/07/97	pjr		add RpSetRemoteHostIpAddress
 *		08/20/97	bva		add RpSetNextFilePage
 *		07/31/97	rhb		add RpSetUrlState.
 *		07/31/97	edk		rework RpSetUserPhraseDictionary for broader 
 *								compiler support
 *		06/19/97	bva		add RpGetRepeatWhileValue 
 *		06/12/97	bva		add RpGetUrlState 
 * * * * Release 1.6 * * * *
 *		03/25/97	bva		add RpSetServerPushTime, RpSetServerPushPage 
 *		02/17/97	bva		add documentation
 *		02/02/97	bva		fix documentation on RpSetRefreshTime
 *								add RpSetUserPhraseDictionary,
 *								UserDataPtr -> Cookie
 *		02/01/97	bva		refreshSeconds becomes Unsigned16
 *		01/20/97	bva		add RpGetUserAgent, RpGetAcceptLanguage, 
 *								RpSetUserRequestPtr, RpGetUserRequestPtr,
 *								RpSetUserFormItemFunction 
 *		01/16/97	bva		add RpSetUserDataPtr, RpGetUserDataPtr
 *		01/04/97	bva		add RpGetQueryIndexLevel
 *		12/06/96	bva		add RpSetConnectionClose callback 
 * * * * Release 1.5 * * * *
 *		11/06/96	bva		add RpSetRefreshTime, RpSetRefreshPage callbacks 
 *								add RpHandleUnknownUrl engine exit
 *		11/01/96	bva		created from RpPages.h
 * * * * Release 1.4 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_RPCALL_
#define	_RPCALL_

/*
	Call back routines
	
	There are a variety of call back routines that are used to control
	various internal states of the web server engine.  These routines
	can be called from the device Set/Get routines or the optional
	page/form processing routines.  The call back routines all pass back
	the engine global data pointer and other variables as appropriate.
*/


/*****************************************************************************

	Security call back routines
	
	These call back routines are used to control the security environment.
	There are a maximum of 8 realms supported that each have a realm name
	and a security level.

	There is an internal users database which contains the number of user
	entries defined by kRpNumberOfUsers in RpConfig.h.  Each user entry
	contains a realm access code that indicates which realms the user has
	access to.

	The security callback routines are described below under the following
	categories:

		Realm manipulation routines
		User manipulation routines
		Session manipulation routines
		Feature access control routines

*****************************************************************************/

/*
	Realm manipulation routines.

	The RpSetRealmLocking routine is called to enable and disable realm
	locking.  If Realm Locking is enabled, when a user logs on successfully,
	all the pages in the realm(s) they have access to become reserved for
	that user until they complete their session.  Any other user who logs on
	(even if they normally have access to a given realm) will be blocked from
	accessing the pages and forms in that realm until the first user logs off
	or Realm Locking is disabled.

	The RpSetRealmName routine is passed an index (0 - 7) of the realm and
	a pointer to an ASCII string containing the new realm name.

	The RpGetRealmName routine is passed an index (0 - 7) of the realm and
	returns a pointer to an ASCII string containing the current realm name.

	The RpSetSecurityLevel routine is passed an index (0 - 7) of the realm
	and an rpSecurityLevel indicating the new security level for the realm.

	The RpGetSecurityLevel routine is passed an index (0 - 7) of the realm
	and returns an rpSecurityLevel which is defined in RpPages.h.
*/

/*
	Password states.
*/

typedef enum {
	eRpPasswordNotAuthorized,
	eRpPasswordAuthorized,
	eRpPasswordPending,
	eRpPasswordBusy,
	eRpPasswordDone
} rpPasswordState;


extern void				RpSetRealmLocking(void *theTaskDataPtr,
							Boolean theLockState);

extern void				RpSetRealmName(void *theTaskDataPtr,
							unsigned char 	theIndex,
							char 			*theRealmNamePtr);

extern char *			RpGetRealmName(void *theTaskDataPtr,
							unsigned char 	theIndex);

extern void				RpSetSecurityLevel(void *theTaskDataPtr,
							unsigned char 	theIndex,
							rpSecurityLevel theSecurityLevel);

extern rpSecurityLevel	RpGetSecurityLevel(void *theTaskDataPtr,
							unsigned char 	theIndex);


/*
	User manipulation routines.

	The RpSetUserAttributes routine is called to create a user entry for a
	new user or modify the attributes of an existing user.  Each username in
	the user database must be unique.  The first time an RpSetUserAttributes
	call is issued for a specific username, a new entry will be set up in the
	user database.  Additional calls with the same username can be used to
	change the attributes for a specific user.  The RpSetUserAttributes will
	return False if there is no more room in the user database.  If
	theIpAddress is 0, any IP address will be acceptable.  If
	theTimeoutSeconds is 0, the server's master security session timeout
	value will be used.

	The RpGetUserAttributes routine is passed an index (0-based) into the
	user database.  It returns the username, password, realm access code,
	IP address, and session timeout value for the user entry.  If there
	is no user entry for the requested index, the return value of
	theUsernamePtr will be NULL.  The index passed into RpGetUserAttributes
	is automatically adjusted for empty user entries.  If an index value
	of 2 is passed in, the parameters for the third active user entry will
	be returned, any empty user entries will be skipped.  Using this method,
	once an empty user entry is returned, there are no more user entries
	with a higher index.

	The RpGetUserCount routine returns the current number of users in the
	user database.

	The RpGetUserIndex routine is passed a username string.  This routine
	will find the user in the user database and return the index for that
	user entry.

	The RpDeleteUser routine is passed a pointer to an ASCII string
	containing the name of the user to be deleted.  This routine is
	used to remove an active entry from the user database.  If the user
	being deleted has an active security session, it will be terminated
	and the user entry will be deleted.

	The RpGetCurrentUserName routine can be called from within a page to
	determine the name of the user that has authenticated access.  It
	returns a pointer to an ASCII string containing the name of the
	current authenticated user.

	The internal user security database can store an opaque variable that
	can be used for communicating with external password servers such as
	RADIUS servers.  The opaque variable (or cookie) is set using the
	RpSetPasswordCookie routine and retrieved using the RpGetPasswordCookie
	routine.
*/

extern Boolean		RpSetUserAttributes(void *theTaskDataPtr,
						char 		*theUsernamePtr,
						char 		*thePasswordPtr,
						rpAccess 	theAccessCode,
						Unsigned32 	theIpAddress,
						Signed32 	theTimeoutSeconds);

extern void			RpGetUserAttributes(void *theTaskDataPtr,
						Unsigned16 	theUserIndex,
						char 		**theUsernamePtr,
						char 		**thePasswordPtr,
						rpAccess 	*theAccessCodePtr,
						Unsigned32 	*theIpAddressPtr,
						Signed32 	*theTimeoutSecondsPtr);

extern Signed16		RpGetUserCount(void *theTaskDataPtr);

extern Signed16		RpGetUserIndex(void *theTaskDataPtr,
						char 		*theUsernamePtr);

extern void			RpDeleteUser(void *theTaskDataPtr,
						char 		*theUsernamePtr);

extern char *		RpGetCurrentUserName(void *theTaskDataPtr);

extern void			RpSetPasswordCookie(void *theTaskDataPtr,
						char 		*theUsernamePtr,
						void 		*theCookie);

extern void *		RpGetPasswordCookie(void *theTaskDataPtr,
						char 		*theUsernamePtr);

/*
	Session manipulation routines.

	The RpSetCurrentSessionCloseFunction routine is used set up a routine that
	will be called when the current user security session completes. Typically,
	this will happen when the user's security session timer expires.  The
	value of the user's security session timer is set when the user entry is
	created or modified with the RpSetUserAttributes call.  The caller can pass
	an opaque variable (theUserCookie) that will be returned to the completion
	function when it is called.

	The RpSetSessionStartFunction routine is used set up a routine that will
	be called when a user security session is started.

	The RpSetServerPasswordTimeout routine is used to change the server's
	master security session timeout value.  This timeout value is set at
	initialization time to the value defined by kPasswordSessionTimeout
	in RpConfig.h.  It is used as the default timeout value for calls to
	RpSetUserAttributes with a 'theTimeoutSeconds' value of 0.

	The RpCheckSession routine is used to find out the current access state
	for a realm or realms.  The call passes in the access code containing
	the realm(s) to be checked (such as kRpPageAccess_Realm2) and receives
	back a Boolean indicating whether access is currently authorized for the 
	realm(s). 

	The RpResetCurrentSession routine can be called from within a page to
	reset the security session for the current authenticated user.  The
	user security session will be reset and any locked realms will be
	released.  Further access will result in a re-challenge.  This call
	uses the rpProcessDataFuncPtr format and may be issued directly from
	the page object structure.

	The RpResetUserSession routine may be called from any point within the
	device and will force the reset of a specific user security session.

	The RpCheckAuthorization routine is called to check a user's credentials
	with RomPager's internal security database.  This routine does not support
	digest access authentication and is meant to be used by the RomCLI product.

	The RpStartUserSession routine is called to begin an internal security
	session once the user's credentials have been verified using the
	RpCheckAuthorization routine.  This security session does not time out
	as an HTTP security session does and must be reset by calling the
	RpResetUserSession routine.

	The RpCheckUserExitAuthorization routine is called to check a user's
	credentials with RomPager's internal security database.

	The RpStartUserExitSession routine is called to begin an HTTP security
	session once the user's credentials have been verified using the
	RpCheckUserExitAuthorization routine.
*/

extern void				RpSetCurrentSessionCloseFunction(void *theTaskDataPtr,
							rpSessionCloseFuncPtr	theFunctionPtr,
							void 					*theUserCookie);

typedef void 			(*rpSessionStartFuncPtr) (void *theTaskDataPtr,
							char 		*theUsernamePtr,
							char 		*thePasswordPtr,
							rpAccess 	theRealmAccess);

extern void				RpSetSessionStartFunction(void *theTaskDataPtr,
							rpSessionStartFuncPtr theFunctionPtr);

extern void				RpSetServerPasswordTimeout(void *theTaskDataPtr,
							Signed32 	theTimeoutSeconds);

extern Boolean			RpCheckSession(void *theTaskDataPtr,
							rpAccess 	theAccessCode);

extern void				RpResetCurrentSession(void *theTaskDataPtr,
							Signed16Ptr theIndexValuesPtr);

extern void				RpResetUserSession(void *theTaskDataPtr,
							char 		*theUsernamePtr);


extern rpPasswordState	RpCheckAuthorization(void *theTaskDataPtr,
							Unsigned16 	theConnection,
							char 		*theUsernamePtr,
							char 		*thePasswordPtr,
							Unsigned32	theIpAddress,
							rpAccess 	*theAccessCodePtr,
							Boolean 	*theSessionFlag);

extern Boolean			RpStartUserSession(void *theTaskDataPtr,
							char 		*theUsernamePtr,
							rpAccess 	theAccessCode);

#if RomPagerUserExit

extern rpPasswordState	RpCheckUserExitAuthorization(void *theTaskDataPtr,
							Unsigned16 	theConnection,
							char 		*theUsernamePtr,
							char 		*thePasswordPtr,
							Unsigned32 	theIpAddress,
							rpAccess 	*theAccessCodePtr);

extern rpPasswordState	RpStartUserExitSession(void *theTaskDataPtr,
							char 		*theUsernamePtr,
							rpAccess 	theAccessCode);

#endif	/* RomPagerUserExit */


/*
	Feature access control routines.

	The RpSetPutAccess routine is used to assign the security realms that
	control PUT access for the device.  These security realms apply to all
	PUT access for the device.  The access code field contains the same
	security realm flags (such as kRpPageAccess_Realm1) that are stored
	in the fObjectAccess field of a page or form object.
*/

extern void				RpSetPutAccess(void *theTaskDataPtr, 
							rpAccess theAccessCode);


/*
	Page Flow and Connection Control routines
	
	These call back routines are used to control the page state.  The
	RpGetSubmitButtonValue returns a character pointer to the ASCII value 
	of the Submit button.  This routine is useful with forms that have 
	more than one Submit button so that the management application can 
	determine which button was pressed.  The RpGetCurrentUrl routine returns
	a character pointer to the URL that invoked the page or form.  This can
	be useful for processing routines that are common to multiple pages
	or forms.
	
	The RpSetNextPage routine passes in an object pointer to the rom page 
	to be served next after redirection.  This routine is typically called 
	in the optional form post-processing routine to override the value in the 
	fPagePtr of the rpObjectExtension structure. The RpSetNextFilePage 
	callback works the same way as RpSetNextPage, but passes a pointer to a 
	string that contains the next URL to be served.  In this way, the next 
	page to be served can be located in the file system, or on a remote host, 
	as well as in the rom. If the RpSetNextPage or RpSetNextFilePage functions 
	are called in the page pre-processing routine, normally the page items 
	pointed to by the "Next" page will be served directly.  If it is desirable 
	to have the page served by browser redirection, then the RpSetRedirect 
	call should be used. 
	
	If client pull refresh is supported, the RpSetRefreshTime and 
	RpSetRefreshPage call back routines may be used to dynamically set up the 
	time in seconds and the page pointer to the next page. Setting the refresh 
	time to 0 will end the page refresh cycle. The information may also be set
	up statically in the object extension fields. If these call back routine 
	are used, they will only be effective if called during page setup 
	pre-processing. They are not effective if called during page item 
	processing, since the HTTP headers have already been sent. They also are 
	not effective during form processing, since forms handling provides 
	redirection to a new page that will have its own refresh values.
	
	If server push is supported, the RpSetServerPushTime and 
	RpSetServerPushPage call back routines may be used to dynamically set up 
	the time in seconds and the pointer to the next object to be pushed.  
	The information may also be set up statically in the object extension 
	field of the currently pushed object.  If these call backs are used, they 
	will only be effective if called during page setup pre-processing of the 
	object currently being pushed. They will not be effective if called during 
	page item processing, since the HTTP headers will have already been sent. 
	They will not be effective during form processing, since form input is not
	allowed during a server push session.
	
	The RpSetRequestCloseFunction call back is used to set up a function that
	will be called after the HTTP request has been completely processed and
	the TCP connection has been closed.  The primary use for this function is
	to trigger device reset functions after being assured that the user page 
	with "Reset about to start" message has been completely delivered.
	
	The RpSetConnectionCloseFunction call back is used to set up a function 
	that will be called after the the TCP connection has been closed. This 
	can be useful when an application needs to know that the client has 
	closed the TCP connection, especially for Netscape "keep alive" and 
	HTTP 1.1 connections.

	The RpSetConnectionClose call back is used to force the TCP connection 
	being used for the current HTTP request to close after the current HTTP 
	request is completed.  Both Netscape "keep alive" support and HTTP 1.1 
	persistent connections attempt to keep the TCP connection open for 
	multiple HTTP requests from a single browser. In some cases, it may be 
	useful for the server to free up a TCP connection for other users or 
	other TCP applications.
*/

extern char *		RpGetSubmitButtonValue(void *theTaskDataPtr);
extern void			RpSetNextPage(void *theTaskDataPtr, 
						rpObjectDescriptionPtr theNextPagePtr);
extern void			RpSetRedirect(void *theTaskDataPtr);
extern char * 		RpGetCurrentUrl(void *theTaskDataPtr);

#if RomPagerFileSystem || RomPagerUserExit
extern void			RpSetNextFilePage(void	*theTaskDataPtr, 
									char 	*theNextPagePtr);
#endif
extern void			RpSetRedirectAbsolute(void *theTaskDataPtr,
									Boolean	theUseSslFlag,
									char 	*theHostNamePtr,
									char	*thePathPtr);

#if RomPagerClientPull
extern void			RpSetRefreshTime(void 	*theTaskDataPtr, 
								Unsigned16 	theRefreshSeconds);
extern void			RpSetRefreshPage(void 	*theTaskDataPtr, 
						rpObjectDescriptionPtr theRefreshPagePtr);
#endif

#if RomPagerServerPush
extern void 		RpSetServerPushTime(void *theTaskDataPtr, 
						Unsigned16 theServerPushSeconds);
extern void 		RpSetServerPushPage(void *theTaskDataPtr, 
						rpObjectDescriptionPtr theServerPushPagePtr);
#endif

extern void 		RpSetRequestCloseFunction(void 	*theTaskDataPtr, 
						rpProcessCloseFuncPtr 		theFunctionPtr);

extern void 		RpSetConnectionCloseFunction(void 	*theTaskDataPtr, 
								rpConnCloseFuncPtr 		theFunctionPtr, 
								void 					*theCookie);

extern void			RpSetConnectionClose(void *theTaskDataPtr);



/*
	Query Index call back routines
	
	These call back routines are used to modify the query index state.  The
	RpPushQueryIndex routine places a new value on the query index stack and 
	the RpPopQueryIndex routine pops a value off the stack and returns it to
	the caller.  These routines may be used to control the indices that are
	passed to other pages.  The values used in these routines are internal 
	index values and therefore are 0-relative.
	
	The RpGetQueryIndexLevel routine reports back the current index level in 
	use starting at 0.  If there are no index levels in use, the value returned
	will be -1.
*/

extern Signed16 	RpPopQueryIndex(void *theTaskDataPtr);
extern void 		RpPushQueryIndex(void *theTaskDataPtr, 
						Signed16 theQueryValue);
extern Signed8 		RpGetQueryIndexLevel(void *theTaskDataPtr);

/*
	Delayed Function call back routines
	
	These call back routines are used to pend a connection while a delayed
	data set up function is completing. The RpInitiateDelayedFunction routine 
	is called when a delayed function is started and returns the connection 
	number of the internal RomPager task that will be suspended. The 
	connection number should be saved away for use by the completion routine 
	of the delayed function. When the completion routine of the delayed 
	function is ready to allow the RomPager internal task resume processing, 
	it should issue the RpCompleteDelayedFunction call with the saved 
	connection number.  
	
	If the completion routine of the delayed function needs to
	issue any other RomPager callback routines such as RpSetNextPage it
	needs issue a RpSetCurrentConnection call to set up RomPager for the
	correct RomPager internal connection.  The RpSetCurrentConnection call
	returns the old current connection which will need to be restored with
	another RpSetCurrentConnection call just before the completion routine
	finishes. The RpGetCurrentConnection call may be used to determine
	which connection is being used by the internal RomPager task.
	
	Typically, the RpInitiateDelayedFunction is called from within the 
	initial optional page processing function to set up local variables 
	that will be accessed when the page is served.  It can also be called
	from the optional form processing function to save away variables that
	have been submitted from a form.
*/

extern Unsigned16 	RpInitiateDelayedFunction(void *theTaskDataPtr);
extern void 		RpCompleteDelayedFunction(void *theTaskDataPtr, 
							Unsigned16 theConnection);
extern Unsigned16 	RpGetCurrentConnection(void *theTaskDataPtr);
extern Unsigned16 	RpSetCurrentConnection(void *theTaskDataPtr, 
							Unsigned16 theConnection);


/*
	User Data routines
	
	The RpInitUserData routine is called by the RomPager engine at startup 
	time if global data is assigned dynamically.  This allows the device to 
	perform any web server specific data storage initialization.  

	The RpSetCookie routine is used to save a pointer to arbitrary user data 
	in the engine data structure for retrieval later with the RpGetCookie 
	routine.  The RpSetRequestCookie routine saves a pointer to arbitrary 
	user data in the current request structure for retrieval later with the 
	RpGetRequestCookie routine.

	The RpSetUserPhraseDictionary routine allows the user to overwrite the 
	default user phrase dictionary with an alternate phrase dictionary.  A 
	phrase dictionary is an array of char pointers indexed by characters 
	used in the HTML text.  Using alternate user phrase dictionaries can be 
	helpful in setting up pages that will have alternate appearances depending 
	on the dictionary.  theCompressionFlag is used to signal whether the 
	phrases in the dictionary can contain compressed phrases.  True means 
	that the phrases in this dictionary may contain additional compressed 
	phrases, from either the system or user dictionaries. False means no 
	further expansion should be performed on phrases in this dictionary.  
	Some RomPager customers use this technique for internationalization 
	support.  To reset back to the default user phrase dictionary, issue 
	the call:
		 RpSetUserPhraseDictionary(theTaskDataPtr, gUserPhrases, True);
		 
	The RpSetUserPhraseDictionary routine changes default user dictionary 
	for the entire system. The RpSetRequestUserPhraseDictionary routine changes
	the user dictionary for a single page or form request. This routine can
	be called from within the first item on a page or form to set the dictionary
	for the rest of the items.
	
	The RpGetRepeatWhileValue routine can be used in individual Get functions 
	that are called during processing of an eRpItemType_RepeatGroupWhile item.
	The routine retrieves the current value that was returned by the Repeat
	While function.  This allows the individual Get functions to use the 
	current repeat value as an index or in any other way to modify the value 
	the Get function returns.	 

	The AsExpandString routine uses the phrase dictionaries to expand strings. 
	The routine takes a string to be expanded and a buffer and returns the 
	expanded string in the buffer. It returns eAsItemError_TooManyCharacters 
	if the buffer is too small.
*/

#if RomPagerDynamicGlobals
extern void 	RpInitUserData(void *theTaskDataPtr);
#endif

#if RpUserCookies
extern void 	RpSetCookie(void *theTaskDataPtr, void *theCookie);
extern void * 	RpGetCookie(void *theTaskDataPtr);

extern void 	RpSetRequestCookie(void *theTaskDataPtr, void *theCookie);
extern void * 	RpGetRequestCookie(void *theTaskDataPtr);
#endif

extern void 	RpSetUserPhraseDictionary(void *theTaskDataPtr, 
					char **theUserDictionaryPtr, 
					Boolean theCompressionFlag);

extern void 	RpSetRequestUserPhraseDictionary(void *theTaskDataPtr, 
					char **theUserDictionaryPtr, 
					Boolean theCompressionFlag);

extern void * 	RpGetRepeatWhileValue(void *theTaskDataPtr);


/*
	Number to string conversion routines
	
	The RpConvertUnsigned32ToAscii and RpConvertSigned32ToAscii routines are
	used internally by the RomPager engine and may also be used by the device 
	page display routines.  These routines are passed in a signed or unsigned 
	32 bit number and a char pointer to the buffer to place the string.  
	The routines return the number of characters in the converted string. 
*/

extern Unsigned16	RpConvertSigned32ToAscii(Signed32 theNumber, 
						char *theBufferPtr);
extern Unsigned16	RpConvertUnsigned32ToAscii(Unsigned32 theNumber, 
						char *theBufferPtr);

#if AsUse64BitIntegers
/*
	64 bit string conversion routines
	
	The RpConvertUnsigned64ToAscii and RpConvertSigned64ToAscii routines are
	used internally by the RomPager engine and may also be used by the device 
	page display routines if 64 bit integers are used. 
*/

extern Unsigned16	RpConvertSigned64ToAscii(Signed64 theNumber, 
						char *theBufferPtr);
extern Unsigned16	RpConvertUnsigned64ToAscii(Unsigned64 theNumber, 
						char *theBufferPtr);
#endif

/*
	String to number conversion routines
	
	The string to number routines are used internally by the RomPager engine 
	during the forms item handling and may also be used by the device form 
	item routines.  Normally, the fTextType field is set with the numeric
	type and the engine will perform the conversion.  In the case where the
	device form item handling routine wishes to examine the string before
	the conversion, or look at the number after the conversion, the fTextType
	field should be set to eRpTextType_ASCII and the function pointed to by
	the fSetPtr field can call a conversion routine.  

	The RpGetConversionErrorCode callback may be used to check the results of 
	the conversion routine.  The RpSetUserErrorMessage callback can be used 
	to trigger an error page display with a custom message.
*/

extern void 		RpConvertHexString(void 	*theTaskDataPtr, 
									char 		*theHexPtr, 
									char 		*theValuePtr, 
									Unsigned8 	theOutputCharCount, 
									char 		theSeparator);

extern void			RpConvertDotFormString(void 	*theTaskDataPtr, 
										char 		*theDotFormPtr, 
										char 		*theValuePtr, 
										Unsigned8	theOutputCharCount);

extern Signed8		RpConvertStringToSigned8(void *theTaskDataPtr, 
						char *theValuePtr);
extern Signed16 	RpConvertStringToSigned16(void *theTaskDataPtr, 
						char *theValuePtr);
extern Signed32 	RpConvertStringToSigned32(void *theTaskDataPtr, 
						char *theValuePtr);
extern Unsigned8 	RpConvertStringToUnsigned8(void *theTaskDataPtr, 
						char *theValuePtr);
extern Unsigned16 	RpConvertStringToUnsigned16(void *theTaskDataPtr, 
						char *theValuePtr);
extern Unsigned32 	RpConvertStringToUnsigned32(void *theTaskDataPtr, 
						char *theValuePtr);
#if AsUse64BitIntegers
extern Signed64 	RpConvertStringToSigned64(void *theTaskDataPtr, 
						char *theValuePtr);
extern Unsigned64 	RpConvertStringToUnsigned64(void *theTaskDataPtr, 
						char *theValuePtr);
#endif
extern asItemError 	RpGetConversionErrorCode(void *theTaskDataPtr);
extern void 		RpSetUserErrorMessage(void *theTaskDataPtr, 
						char *theMessagePtr);


/*
	Time access routines
	
	The RpGetMonthDayYearInSeconds routine is used to translate an external
	date into internal format which is seconds since 1/1/1901.

	The RpGetSysTimeInSeconds routine is used internally by the RomPager
	engine and may also be used by the user application routines.  It
	returns the system time in internal format.
*/

extern Unsigned32	RpGetMonthDayYearInSeconds(void 	*theTaskDataPtr,
											Unsigned32 	theMonth,
											Unsigned32 	theDay,
											Unsigned32 	theYear);
extern Unsigned32	RpGetSysTimeInSeconds(void *theTaskDataPtr);


/*
	Browser request information
	
	The RpGetUserAgent routine is used to return the information from the
	User-Agent HTTP header that the browser sends in with the request.  This 
	can be useful in distinguishing between browser types to decide which 
	kind of HTML to serve.  The RpGetAcceptLanguage routine is used to return 
	the information from the Accept-Language HTTP header.  This may be useful
	for internationalization support.  The RpGetHostName routine passes in the
	host name used by the browser to access the page. This may be useful for 
	constructing fully-qualified URL references. The RpGetSoapAction routine 
	is used to return the information from the Soap-Action HTTP header.  This 
	is useful for SOAP-based Web services.
*/

#if RomPagerCaptureUserAgent
extern char * 		RpGetUserAgent(void *theTaskDataPtr);
#endif

#if RomPagerCaptureLanguage
extern char * 		RpGetAcceptLanguage(void *theTaskDataPtr);
#endif

#if RomPagerCaptureSoapAction
extern char * 		RpGetSoapAction(void *theTaskDataPtr);
#endif

extern char * 		RpGetHostName(void *theTaskDataPtr);

/*
	Form item handling
	
	Normally, RomPager handles forms for the user application by processing
	items against a form item list pointed to by a form object.  In some
	cases, an application may want to handle it's own form items, or may
	want to use some of the internal RomPager routines to do partial form
	processing.
	
	The RpGetFormBufferPtr call returns a pointer to the form arguments 
	passed in from the browser with a GET Query command or a POST command. 
	Request arguments from HTML forms are passed as a group of name/value pairs 
	encoded in a format called 'Form URL Encoding'.  The RpGetFormItem routine 
	is used to decode and retrieve a name/value pair from the form buffer.  The 
	buffer pointer will be updated to point to the next name/value pair and the 
	values of the current name/value pair will be copied into the buffers passed 
	as input to the call. The RpReceiveItem routine passes the name/value pair 
	against the current form object to use the standard RomPager processing to 
	drive calls to the Set functions. The current form object may be set up by 
	the RpSetFormObject call.
*/

extern char * 	RpGetFormBufferPtr(void *theTaskDataPtr);
extern void 	RpGetFormItem(char 	**theBufferPtr, 
							char 	*theNamePtr, 
							char 	*theValuePtr);
extern void 	RpSetFormObject(void *theTaskDataPtr, 
							rpObjectDescriptionPtr theObjectPtr);
extern void		RpReceiveItem(void 	*theTaskDataPtr, 
							char 	*theNamePtr,
							char 	*theValuePtr);

/*
	URL States
	
	If URL states are enabled, URLs with the form 'http://<deviceaddress>
	/XXX/SSSSS/pathname' will be examined to see if a state should be stored 
	away.  If the 'XXX' matches the value set by kUrlState (which has a 
	default value of 'US'), then the 'SSSSS' is stored in internal structures 
	that can be accessed by the RpGetUrlState routine.  The RpSetUrlState can 
	be used to change the current URL state prior to serving a page that 
	contains eRpItemType_UrlState elements. See RpCheckUrlState in RpHttpRq.c 
	for further information on URL states.
*/

#if RomPagerUrlState
extern char * 		RpGetUrlState(void *theTaskDataPtr);
extern void 		RpSetUrlState(void *theTaskDataPtr, char *theNewState);
#endif

#if RomPagerHttpCookies
extern char * 		RpGetHttpCookie(void *theTaskDataPtr, Unsigned8 theIndex);
extern void 		RpSetHttpCookie(void 		*theTaskDataPtr, 
									Unsigned8 	theIndex, 
									char 		*theCookie);
#endif


/*
	Remote Hosts
	
	The RpSetRemoteHostIpAddress routine is used to set the IP address
	of the host to access for remote URL retrieval.  If multiple remote
	hosts are supported, the RpSetRemoteHostIpAddress call takes an
	index to the array of remote host addresses.

	The RpSetRemoteHostAuthInfo routine is used to set the security
	credentials (username and password) to be used when making remote
	host requests.  If multiple remote hosts are supported, the
	RpSetRemoteHostAuthInfo call takes an index to the array of
	remote hosts.
*/

#if RomPagerRemoteHost

#if RpRemoteHostMulti
extern void			RpSetRemoteHostIpAddress(void 		*theTaskDataPtr,
											Unsigned32 	theIpAddress,
											Unsigned8	theHostIndex);
#else
extern void			RpSetRemoteHostIpAddress(void 		*theTaskDataPtr,
											Unsigned32 	theIpAddress);
#endif

#if RpRhLocalCredentials
#if RpRemoteHostMulti
extern void			RpSetRemoteHostAuthInfo(void		*theTaskDataPtr,
											char 		*theUsernamePtr,
											char 		*thePasswordPtr,
											Unsigned8	theHostIndex);

#else
extern void			RpSetRemoteHostAuthInfo(void		*theTaskDataPtr,
											char 		*theUsernamePtr,
											char 		*thePasswordPtr);
#endif
#endif	/* RpRhLocalCredentials */

// +++ _Alphanetworks_Patch_, 11/04/2003, jacob_shih

/*
	alphaRpSetRemoteHostIpAddress
*/

#if RpRemoteHostMulti
void alphaRpSetRemoteHostIpAddress(
									Unsigned32 theIpAddress,
									Unsigned8 theHostIndex);
#else
void alphaRpSetRemoteHostIpAddress(
								Unsigned32 theIpAddress);
#endif

/*
	alphaRpSetRemoteHostAuthInfo
*/

#if RpRhLocalCredentials
#if RpRemoteHostMulti
void alphaRpSetRemoteHostAuthInfo(
								char *theUsernamePtr,
								char *thePasswordPtr,
								Unsigned8 theHostIndex);
#else	/* !RpRemoteHostMulti */

void alphaRpSetRemoteHostAuthInfo(
								char *theUsernamePtr,
								char *thePasswordPtr);
#endif
#endif	/* RpRhLocalCredentials */

/*
	alphaRpIsRemoteHostValid
*/

#if RpRemoteHostMulti
Boolean alphaRpIsRemoteHostValid(void* theTaskDataPtr, Unsigned16 theHostIndex);
#else
Boolean alphaRpIsRemoteHostValid(void* theTaskDataPtr);
#endif

// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih

#endif	/* RomPagerRemoteHost */


/*
	HTTP Event Logging
	
	If the RomPagerLogging flag is enabled in RpConfig.h, then the RomPager
	engine will store an event in a memory ring buffer for every page that 
	is served, or form that is processed.  Two callback routines are available 
	to query the log. An example of the use of these routines is shown in the 
	HTTP History Page in RpPages.c

	The RpGetHttpLogItemCount routine returns the number of items currently
	in the HTTP event log.

	Given an item index, the RpBuildHttpEventStrings routine builds the
	Event Time, Type, and Object strings in the string buffers supplied
	by the caller.  If an invalid index is given, the strings will contain
	only a null string terminator.  If the index is valid, the Event Time
	and Type strings will be created, but the Event Object string may be
	an empty string since some event types do not have an object
	associated with them.
*/

#if RomPagerLogging
extern Signed16		RpGetHttpLogItemCount(void *theTaskDataPtr);
extern void			RpBuildHttpEventStrings(void 		*theTaskDataPtr,
											Signed16 	theIndex,
											char 		*theEventTimeString,
											char 		*theEventTypeString,
											char 		*theEventObjectString);
#endif



#if RomPagerIpp

/*
	IPP Printer Name

	The IPP printer name is the name that identifies the printer in the URL.
	In other words, URLs that need to be passed to the IPP parser should 
	have the form "<hostname>/xxxxx/yyyyy" where "xxxxx" is the printer 
	name and "/yyyyy" is an optional job id.

	There may be one or more printer names.  The number of printer names
	configured is determined by the value of kIppNumberOfPrinters in
	RpConfig.h.  The printer name(s) are set at startup time by calling
	RpSetIppPrinterName from the RpInitializeBox routine in RpUser.c.
	Security access for each printer name is set using theAccessCode
	parameter.  The access code field contains the same security realm
	flags (such as kRpPageAccess_Realm1) that are stored in the
	fObjectAccess field of a page or form object.
*/

extern void 		RpSetIppPrinterName(void 		*theTaskDataPtr, 
										Unsigned8 	theIndex,
										rpAccess 	theAccessCode,
										char 		*thePrinterName);

#endif	/* RomPagerIpp */


#if RomPagerFileSystem
#if RomPagerFileUpload
/*
	File Upload Status

	The RpGetFileUploadStatus returns True if a File Upload has been
	completed successfully, otherwise it returns False.
*/

extern Boolean		RpGetFileUploadStatus(void * theTaskDataPtr);
#endif
#endif	/* RomPagerFileSystem */

/*
	Rom Object Master list

	The rom object list consists of a master list that points to individual
	item lists each of which point to a set of rom objects. The lists are
	searched linearly, so there may be some small gains acheived by putting 
	the popular pages towards the front. The home or root page is the first 
	item in the first list (Index offset 0).  Other than that, there are no 
	order dependencies in the lists.
	
	The RpSetRomObjectList call back routine is used to tell the RomPager engine
	what to use for the current rom object list. The rom object list may be 
	controlled at runtime by varying the contents of the master object list.
	This can be useful for dynamically enabling/disabling feature sets.  
	
	The default rom object list is set to gRpMasterObjectList by the 
	RpInitializeBox routine in RpUser.c.
*/

extern void 	RpSetRomObjectList(void *theTaskDataPtr, 
					rpObjectDescPtrPtr *theMasterObjectListPtr);



#endif
