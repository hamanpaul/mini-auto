/*
 *	File:		RpIntPrt.h
 *
 *	Contains:	Internal prototypes for RomPager and RomPager Basic
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
 *		08/25/03	rhb		disable RpAccess.c routines if no RomPagerSecurity
 *		08/07/03	bva		add routines for Range support
 * * * * Release 4.21  * * *
 *		06/20/03	bva		add RpHandleXmlServices, RpProcessXmlRequest
 *		04/18/03	bva		bump theConnectionId size
 * * * * Release 4.20  * * *
 *		01/31/03	rhb		fix Soft Pages from Remote Host
 *		09/04/02	pjr		add RpSecurityDeInit prototype, move
 *							RpCheckAuthorization and RpStartUserSession
 *							prototypes to RpCallBk.h
 * * * * Release 4.12  * * *
 * * * * Release 4.00  * * *
 *		06/28/01	pjr		move RpInitializeBox prototype to AsEngine.h
 *		02/16/01	rhb		move Custom & SNMP Access prototypes to AsEngine.h
 *		02/15/01	bva		remove dictionary patching,
 *							change RpExpandedMatch parameter
 *		02/14/01	rhb		rename rpItemError to asItemError
 *		02/13/01	rhb		rename RomPagerSnmpAccess to AsSnmpAccess and
 *								RpCustomVariableAccess to AsCustomVariableAccess
 *		11/25/00	pjr		add RpReceiveText prototype
 *		11/07/00	pjr		add theDataPtr to RpInitRequestStates prototype
 *		10/06/00	pjr		add RpGetLeastSecureRealm prototype
 *		08/31/00	pjr		modify for the new security model
 *		06/02/00	bva		change prototypes for RpQuery.c routines
 *		02/09/00	bva		created from RomPager.h
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef _RPINTPRT_
#define _RPINTPRT_


#if RomPagerHttpOneDotOne
extern void			RpCloseChunkedBuffer(rpHttpRequestPtr theRequestPtr);
extern void			RpSetupChunkedBuffer(rpHttpRequestPtr theRequestPtr);
#endif


/*
	RpFrmItm.c routines
*/
extern void 		RpEscapeDecodeString(char * theEncodedStringPtr,
						char * theDecodedStringPtr);


/*
	RpHttp.c routines
*/
extern void			RpAnalyzeHttpRequest(rpHttpRequestPtr theRequestPtr);
extern void			RpBuildHostName(rpHttpRequestPtr theRequestPtr,
						char *theResponsePtr);
extern void			RpBuildHttpResponseHeader(rpHttpRequestPtr theRequestPtr);
extern void			RpBuildQueryValues(rpHttpRequestPtr theRequestPtr, 
						char *theStringPtr, Boolean theForUrlFlag);
extern RpErrorCode 	RpBuildReply(rpConnectionPtr theConnectionPtr);
extern void 		RpHandlePageNotFound(rpDataPtr theDataPtr);
extern void			RpInitRequestStates(rpDataPtr theDataPtr,
						rpHttpRequestPtr theRequestPtr);
extern RpErrorCode 	RpSendReplyBuffer(rpConnectionPtr theConnectionPtr);

#if RomPagerUserExit
extern void			RpHandleCgiResponse(rpHttpRequestPtr theRequestPtr);
extern void 		RpHandleUserExit(rpConnectionPtr theConnectionPtr);
#endif

/*
	RpHttpPs.c routines
*/
extern void			RpParseHeader(rpHttpRequestPtr theRequestPtr,
						rpPatternTablePtr thePatternTablePtr);
extern void 		RpInitPatternTable(rpPatternTablePtr thePatternTablePtr);

#if RomPagerSecurityDigest
extern void			RpInitAuthPatternTable(rpPatternTablePtr thePatternTablePtr);
#endif

#if RomPagerFileUpload
extern void			RpInitMpPatternTable(rpPatternTablePtr thePatternTablePtr);
extern void			RpInitDispositionPatternTable(rpPatternTablePtr thePatternTablePtr);
extern void			RpProcessMpContentDisposition(rpHttpRequestPtr theRequestPtr,
						char *theStartOfTokenPtr, Unsigned16 theTokenLength);
extern void			RpProcessMpContentType(rpHttpRequestPtr theRequestPtr,
						char *theStartOfTokenPtr, Unsigned16 theTokenLength);
#endif

#if RpEtagHeader
extern void 		RpBuildEtagString(char *theEtagString, Unsigned32 theTag);
#endif


/*
	RpHttpRq.c routines
*/
extern Boolean		RpGetObjectData(rpHttpRequestPtr theRequestPtr);
extern Boolean		RpGetChunkedObjectData(rpHttpRequestPtr theRequestPtr);
extern RpErrorCode	RpHandleHttpAction(rpConnectionPtr theConnectionPtr);
extern Boolean		RpParseHttpHeaders(rpHttpRequestPtr theRequestPtr);
extern RpErrorCode	RpStartHttpResponse(rpConnectionPtr theConnectionPtr);

#if RomPagerUrlState
extern void 		RpCheckUrlState(rpHttpRequestPtr theRequestPtr);
#endif

#if RomPagerHttpOneDotOne
extern Boolean 		RpGetChunkedData(rpHttpRequestPtr theRequestPtr);
#endif

/*
	RpUrl.c routines
*/
extern void			RpFindHttpObject(rpConnectionPtr theConnectionPtr);
extern void			RpFindUrl(rpConnectionPtr theConnectionPtr);
extern void 		RpFinishUrlSearch(rpHttpRequestPtr theRequestPtr);
extern void			RpIdentifyObjectSource(rpHttpRequestPtr theRequestPtr);
extern void			RpSearchRomObjectList(rpHttpRequestPtr theRequestPtr);



/*
	RpAccess.c routines
*/
#if RomPagerSecurity

extern rpPasswordState	RpCheckAccess(rpDataPtr theDataPtr, 
							Unsigned8 theAccessCode);
extern rpPasswordState	RpCheckAccess2(rpDataPtr theDataPtr, 
							Unsigned8 theAccessCode);
extern void 			RpCheckPasswordTimers(rpDataPtr theDataPtr);
extern void				RpSecurityDeInit(rpDataPtr theDataPtr);

#if RomPagerExternalPassword
extern void			RpHandleExternalSecurity(rpConnectionPtr theConnectionPtr);
#endif

#if RomPagerSecurityDigest
extern void			RpGenerateNonce(rpHttpRequestPtr theRequestPtr,
						char * theNoncePtr);
#endif

#if RomPagerExternalPassword || RomPagerIpp || RomPagerSecure
extern rpRealmPtr	RpGetLeastSecureRealm(rpDataPtr theDataPtr,
						rpAccess theAccessCode);
#endif

#endif	/* RomPagerSecurity */


/*
	RpForm.c routines
*/
extern rpItemPtr 	RpFindItemFromName(rpItemPtr theItemArrayPtr, 
										char *theNamePtr, 
										char *theValuePtr);
extern void 		RpProcessForm(rpDataPtr theDataPtr);
extern void 		RpProcessImageMap(rpDataPtr theDataPtr);
extern asItemError	RpReceiveText(rpDataPtr theDataPtr, char * theValuePtr,
						rpVariableType theSetPtrType, void *theSetPtr,
						rpTextType theTextType, Unsigned8 theFieldSize,
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);


/*
	RpHtml.c routines
*/
extern void 		RpBuildHtmlPageReply(rpHttpRequestPtr theRequestPtr);
extern void			RpBuildHtmlItemReply(rpHttpRequestPtr theRequestPtr, 
						rpItemPtr theItemPtr, rpItemPtr theNextItemPtr);
extern char *		RpGetBytesPtr(rpDataPtr theDataPtr, void * theGetPtr,
						rpVariableType theGetPtrType, char *theNamePtr,
						Signed16Ptr theIndexValuesPtr);
extern Unsigned8	RpGetDynamicDisplayIndex(rpDataPtr theDataPtr,
						rpDynamicDisplayItemPtr theDynamicDisplayItemPtr, 
						char *theNamePtr);
extern void			RpSendDataOutDecimalUnsigned(rpHttpRequestPtr theRequestPtr,
						Unsigned32 theData);
extern void			RpSendDataOutZeroTerminated(rpHttpRequestPtr theRequestPtr, 
						const char *theZeroTerminatedPtr);
extern Boolean		RpSendItem(rpHttpRequestPtr theRequestPtr, rpItemPtr theItemPtr);
extern void 		RpSetLastBuffer(rpHttpRequestPtr theRequestPtr);

#if RpHtmlSelectFixedSingle || RpHtmlSelectFixedMulti
extern Boolean		RpExpandedMatch(rpHttpRequestPtr theRequestPtr, 
									char *theInputPtr, 
									char *theMatchPtr);
#endif


/*
	RpHttpDy.c routines
*/
#if RomPagerClientPull
extern void			RpBuildRefresh(rpHttpRequestPtr theRequestPtr,
						char *theResponsePtr);
#endif

#if RomPagerServerPush
extern RpErrorCode	RpHandleServerPushWait(rpConnectionPtr theConnectionPtr);
extern void			RpSendServerPushHeaders(rpHttpRequestPtr theRequestPtr);
extern void			RpSendServerPushSeparator(rpHttpRequestPtr theRequestPtr);
#endif


#if RomPagerQueryIndex || RomPagerImageMapping
/*
	RpQuery.c routines
*/
extern void 		RpCheckQuery(rpHttpRequestPtr theRequestPtr);
extern void			RpStoreQueryValues(rpDataPtr theDataPtr,
						char *theQueryPtr);
#endif	/* RomPagerQueryIndex || RomPagerImageMapping */


#if RomPagerFileSystem
/*
	RpFile.c routines
*/
extern RpErrorCode	RpFileClose(rpConnectionPtr theConnectionPtr);
extern RpErrorCode	RpHandleFileStates(rpDataPtr theDataPtr);

#if RpFileInsertItem
extern void			RpSendFileInsertItemError(rpConnectionPtr theConnectionPtr, 
						rpHttpTransactionState theHttpTransactionState);
#endif
#endif


#if RomPagerFileUpload
/*
	RpMulti.c routines
*/
extern Boolean		RpProcessMultipart(rpHttpRequestPtr theRequestPtr);
extern void 		RpSetupFileUploadRequest(rpHttpRequestPtr theRequestPtr);
#endif
	

#if RomPagerPutMethod
/*
	RpPut.c routines
*/
extern Boolean		RpHandlePut(rpHttpRequestPtr theRequestPtr);
extern void 		RpSetupPutRequest(rpHttpRequestPtr theRequestPtr);
#endif
	

#if RomPagerSoftPages
/*
	RpSoftPg.c routines
*/
extern void			RpInitializeParsingHtml(rpConnectionPtr theConnectionPtr,
											Boolean theObjectFitsFlag);
extern RpErrorCode	RpInitializeSoftPages(rpDataPtr theDataPtr);
extern void			RpHandleParsingHtml(rpHttpRequestPtr theRequestPtr);
extern void			RpSoftPageDeInit(rpDataPtr theDataPtr);
#endif


#if RomPagerIpp
/*
	RpIpp.c routines
*/
extern Boolean		RpHandleIpp(rpHttpRequestPtr theRequestPtr);
extern void 		RpSetupIppRequest(rpHttpRequestPtr theRequestPtr);
extern void 		HandleIppParserResponse(rpConnectionPtr theConnectionPtr, 
											ippHttpResponse theParserResponse);
#endif

#if RomPagerRanges
/*
	RpRange.c routines
*/
extern void 		RpCheckRange(rpHttpRequestPtr theRequestPtr);
extern RpErrorCode	RpHandleFilePosition(rpConnectionPtr theConnectionPtr);
extern void 		RpParseRange(rpHttpRequestPtr theRequestPtr,
										char *theRangeRequestPtr);
extern void 		RpSetFilePosition(rpConnectionPtr theConnectionPtr);
#endif

#if RomPlugAdvanced
/*
    RpSoap.c routines
*/
Boolean		RpHandleXmlServices(rpConnectionPtr theConnectionPtr);
void		RpProcessXmlRequest(rpConnectionPtr theConnectionPtr);
#endif

#if RomPagerRemoteHost
/*
	RpRemHst.c routines
*/
extern rpRHRequestPtr	RpCaptureRemoteHostRequest(rpDataPtr theDataPtr);
extern void				RpFreeRemoteHostRequestBlock(
										rpConnectionPtr theConnectionPtr);
extern void 			RpHandleNeedsRemoteHostRequestBlock(
										rpConnectionPtr theClientConnectionPtr);
extern RpErrorCode		RpHandleRemoteHostAction(
										rpConnectionPtr theConnectionPtr);
extern void				RpOpenRemoteHostConnection(
										rpConnectionPtr theConnectionPtr,
										rpRHRequestPtr theRHRequestPtr);
extern void				RpRemoteHostInit(rpDataPtr theDataPtr);
#endif


#if RomPagerLogging
/*
	RpLog.c routines
*/
extern void				RpLoggingInit(rpDataPtr theDataPtr);
extern void				RpLogHttpEvent(rpDataPtr theDataPtr,
										rpHttpResponseAction theEventType,
										rpObjectSource theObjectSource,
										void * theEventInfoPtr);
#endif


#if RomPagerExternalPassword
/*
	The SpwdGetExternalPassword routine is used to retrieve a password that is 
	external to the RomPager engine rather than using the built in password 
	database.  The call passes in the, user name, and IP address of
	the user to be authorized.  It passes a pointer to an area for the external
	routine to return the authorized password.  By maintaining a copy of the
	password, the RomPager engine can handle authorization requests within the
	same session without bothering the external password routine.  The call
	also contains a TCP connection id that may be useful if multiple user
	verifications will be performed simultaneously.
	
	From the point of view of the host operating system, the RomPager engine 
	is a single task. It contains its own scheduler and control blocks for 
	supporting multiple HTTP requests. The call it makes to the external
	password routine must be considered asynchronous for any activity that will 
	incur delay. The call has a completion code to determine whether the external 
	password function has been completed.  Since there may be a desire to pass on 
	the password verification to an external password server, any verification 
	operation that can incur delay needs to create a separate operating system 
	task that can block on call completion. In this way, the RomPager engine can 
	continue to service other simultaneous HTTP requests.

	The possible return states from the SpwdGetExternalPassword call are:

	eRpPasswordPending			

	The external authorization process is not complete. The RomPager engine 
	will call the SpwdGetExternalPassword routine again.
						
	eRpPasswordDone		
	
	The external authorization process is complete.  The user is partially 
	authorized and the string pointed to by thePasswordPtr has been filled 
	in with the authorization password.  The RomPager engine will complete
	the rest of the authorizaton process.

	eRpPasswordNotAuthorized
							
	The authorization process is complete.  The user is not authorized 
	and no password has been filled in by the authorization process.
*/

extern rpPasswordState	SpwdGetExternalPassword(void *theTaskDataPtr,
											Unsigned16 theConnectionId,
											char *theUsernamePtr,
											char *thePasswordPtr,
											Unsigned32 *theIpAddressPtr,
											rpAccess *theRealmAccessPtr);
#endif	/* RomPagerExternalPassword	*/



#endif /* _RPINTPRT_ */
