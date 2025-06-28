/*
 *	File:		RpRemHst.c
 *
 *	Contains:	Remote Host routines for the RomPager Web Server
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
 *		09/19/03	pjr		add RpRhLocalCredentials feature
 *		08/08/03	rhb		add remote host prefix for redirect
 *		07/31/03	bva		fix object for remote host redirect 
 *		07/30/03	bva		fix prefix stripping for RpUnknownUrlsAreRemote 
 * * * * Release 4.21  * * *
 * * * * Release 4.20  * * *
 *		02/05/03	pjr		fix sending of last chunk indication to the client
 *		01/31/03	rhb		fix Soft Pages from Remote Host
 *		01/28/03	rhb		call Soft Pages Preprocessing function
 * * * * Release 4.12  * * *
 * * * * Release 4.01  * * *
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		06/02/00	bva		use gRpDataPtr
 *		04/05/00	pjr		rework building of the accept header
 *		02/09/00	bva		use AsEngine.h
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 *		01/29/99	bva		fHttpClientPatternTable -> fHttpProxyPatternTable
 *		12/31/98	pjr		use RP_MEMCMP
 *		12/30/98	bva		eRpHttpClient -> eRpHttpProxy
 *		12/11/98	rhb		merge Soft Pages
 * * * * Release 2.2 * * * *
 *		11/30/98	bva		fix compile warnings
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		11/02/98	pjr		optimize BuildRequestHeaders
 *		10/30/98	pjr		when there's an error on the Remote Host connection,
 *							abort the Client connection instead of sending a
 *							"404 Not Found" response.
 *		10/27/98	pjr		eliminate RpHandleWaitingForRHReceive
 *		10/14/98	pjr		fix some compiler warnings, change some comments
 *		09/01/98	bva		change rpParsingControl structure
 *		08/26/98	pjr		rework connection error handling
 *		08/04/98	bva		change multi Remote host to support 3 digits
 *		07/28/98	pjr		fix compiler warnings
 *		06/22/98	pjr		add basic authentication support.
 *		06/18/98	pjr		add POST and Redirect support.
 *		06/08/98	bva		add support for multiple Remote Hosts
 * * * * Release 2.1 * * * *
 *		04/30/98	bva		remove unneeded include
 *		04/22/98	bva		add check for unset ipaddress
 *		04/15/98	bva		remove eRpObjectTypeRemote
 *		02/17/98	pjr		handle opening the active connection to the
 *							remote host at this level.
 *		02/16/98	pjr		eliminate possible receive overrun condition on
 *							the remote host connection.  handle an object
 *							that ends exactly on a send buffer boundary.
 *		02/14/98	bva		eRpConnectionRemoteHostWait->eRpConnectionHolding
 *		02/05/98	pjr		eRpConnectionWaitingForReceive ->
 *								eRpConnectionWaitingForRHReceive.
 *		01/21/98	bva		kMaxContentTypeLength->kMaxMimeTypeLength
 *		01/19/98	bva		eRpConnectionNeedsHttpAction ->
 *								eRpConnectionNeedsProtocolAction
 *		01/13/98	rhb		use correct type (rpConnectionPtr not void *) for
 *							rpRHRequest.fClientConnectionPtr and
 *							rpRHRequest.fServerConnectionPtr
 * * * * Release 2.0 * * * *
 *		12/09/97	pjr		add "Pragma: no-cache" support.
 *		12/08/97	bva		documentation
 *		11/26/97	pjr		add more HTTP headers to the Remote Host request.
 *							add date handling.
 *		11/20/97	pjr		fix for lost object data that immediately followed
 *							the headers from the Remote Host in the first TCP
 *							receive buffer.
 *		11/07/97	pjr		initial version.
 * * * * Release 1.6 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"
#include "general.h"
#if RomPagerRemoteHost

static RpErrorCode	ClientConnectionError(rpConnectionPtr theConnectionPtr);

static RpErrorCode	ServerConnectionError(rpConnectionPtr theConnectionPtr);

static Signed32		BuildRequestHeaders(rpRHRequestPtr theRHRequestPtr);

static Unsigned16	ConvertUnsigned32ToDotString(Unsigned32 theValue,
													char * theStringPtr);

static rpRHRequestPtr	GetRemoteHostRequestBlock(rpDataPtr theDataPtr);

static Boolean	ParseRemoteHttpHeaders(rpDataPtr theDataPtr,
										rpRHRequestPtr theRHRequestPtr);

static void		ParseRHHeader(rpRHRequestPtr theRHRequestPtr,
								rpRHPatternTablePtr thePatternTablePtr);

static void		ProcessHttpVersion(rpRHRequestPtr rpRHRequestPtr,
									char *theStartOfTokenPtr,
									Unsigned16 theTokenLength);

static void		ProcessContentType(rpRHRequestPtr rpRHRequestPtr,
									char *theStartOfTokenPtr,
									Unsigned16 theTokenLength);

static void		ProcessDate(rpRHRequestPtr theRHRequestPtr,
							char *theStartOfTokenPtr,
							Unsigned16 theTokenLength);

static void		ProcessExpires(rpRHRequestPtr theRHRequestPtr,
								char *theStartOfTokenPtr,
								Unsigned16 theTokenLength);

static void		ProcessLastModified(rpRHRequestPtr theRHRequestPtr,
									char *theStartOfTokenPtr,
									Unsigned16 theTokenLength);

static void		ProcessDateCommon(rpRHRequestPtr theRHRequestPtr,
									char *theStartOfTokenPtr,
									char *theStringPtr);

static void		ProcessLocation(rpRHRequestPtr theRHRequestPtr,
								char *theStartOfTokenPtr,
								Unsigned16 theTokenLength);

static void		ProcessPragma(rpRHRequestPtr theRHRequestPtr,
								char *theStartOfTokenPtr,
								Unsigned16 theTokenLength);

static void		ProcessAuthenticate(rpRHRequestPtr theRHRequestPtr,
									char *theStartOfTokenPtr,
									Unsigned16 theTokenLength);

static void		InitHttpProxyPatternTable(rpRHPatternTablePtr
											thePatternTablePtr);



void RpRemoteHostInit(rpDataPtr theDataPtr) {
	rpRHRequestPtr		theRHRequestPtr;
	Unsigned8			theIndex;

#if RpRemoteHostMulti
	for (theIndex = 0; theIndex < RpRemoteHostCount; theIndex++) {
		theDataPtr->fRemoteHostIpAddress[theIndex] = 0;
	}
#else
	theDataPtr->fRemoteHostIpAddress = 0;
#endif

	for (theIndex = 0; theIndex < kStcpActiveConnections; theIndex++) {
		theRHRequestPtr = &theDataPtr->fRHRequests[theIndex];
		theRHRequestPtr->fRemoteRequestState = eRpRemoteRequestFree;
		theRHRequestPtr->fServerConnectionPtr = (rpConnectionPtr) 0;
		theRHRequestPtr->fClientConnectionPtr = (rpConnectionPtr) 0;
		theRHRequestPtr->fRemoteHostState = eRpRemoteHostSendRequest;
	}

	/*
		Initialize the HTTP Client pattern recognition table.
	*/
	InitHttpProxyPatternTable(theDataPtr->fHttpProxyPatternTable);

	return;
}


/*
	RpSetRemoteHostIpAddress
*/

#if RpRemoteHostMulti
void RpSetRemoteHostIpAddress(void *theTaskDataPtr,
									Unsigned32 theIpAddress,
									Unsigned8 theHostIndex) {

	if (theHostIndex < RpRemoteHostCount) {
		((rpDataPtr) theTaskDataPtr)->
				fRemoteHostIpAddress[theHostIndex] = theIpAddress;
	}
#if RomPagerDebug
	else {
		RP_PRINTF("RpSetRemoteHostIpAddress, invalid host index\n");
	}
#endif

	return;
}
#else
void RpSetRemoteHostIpAddress(void *theTaskDataPtr,
								Unsigned32 theIpAddress) {

	((rpDataPtr) theTaskDataPtr)->fRemoteHostIpAddress = theIpAddress;

	return;
}
#endif


/*
	RpSetRemoteHostAuthInfo
*/

#if RpRhLocalCredentials
#if RpRemoteHostMulti
void RpSetRemoteHostAuthInfo(void *theTaskDataPtr,
								char *theUsernamePtr,
								char *thePasswordPtr,
								Unsigned8 theHostIndex) {
	rpDataPtr	theDataPtr;
	char		theUserPassword[((kRpMaxUserNameLength +
									kRpMaxPasswordLength) + 1)];

#if RomPagerDebug
	if (theHostIndex >= RpRemoteHostCount) {
		RP_PRINTF("RpSetRemoteHostAuthInfo, theHostIndex is invalid\n");
		return;
	}
	if (RP_STRLEN(theUsernamePtr) >= kRpMaxUserNameLength) {
		RP_PRINTF("RpSetRemoteHostAuthInfo, username too long\n");
		return;
	}
	if (RP_STRLEN(thePasswordPtr) >= kRpMaxPasswordLength) {
		RP_PRINTF("RpSetRemoteHostAuthInfo, password too long\n");
		return;
	}
#endif

	theDataPtr = (rpDataPtr) theTaskDataPtr;

// +++ 11/26/2003, jacob_shih
// to encode the empty username
#if 0
	if (*theUsernamePtr == '\0') {
		/*
			If the Username is empty, clear the credentials.
		*/
		*theDataPtr->fRemoteHostEncodedAuth[theHostIndex] = '\0';
	}
	else {
		RP_STRCPY(theUserPassword, theUsernamePtr);
		if (*thePasswordPtr != '\0') {
			RP_STRCAT(theUserPassword, kColon);
			RP_STRCAT(theUserPassword, thePasswordPtr);
		}

		AsBuildBase64String(theUserPassword,
							(Unsigned16) RP_STRLEN(theUserPassword),
							theDataPtr->fRemoteHostEncodedAuth[theHostIndex]);
	}
#else
		RP_STRCPY(theUserPassword, theUsernamePtr);
		RP_STRCAT(theUserPassword, kColon);
		RP_STRCAT(theUserPassword, thePasswordPtr);

		AsBuildBase64String(theUserPassword,
							(Unsigned16) RP_STRLEN(theUserPassword),
							theDataPtr->fRemoteHostEncodedAuth[theHostIndex]);
#endif
// --- 11/26/2003, jacob_shih

	return;
}

#else	/* !RpRemoteHostMulti */

void RpSetRemoteHostAuthInfo(void *theTaskDataPtr,
								char *theUsernamePtr,
								char *thePasswordPtr) {
	rpDataPtr	theDataPtr;
	char		theUserPassword[((kRpMaxUserNameLength +
									kRpMaxPasswordLength) + 1)];

#if RomPagerDebug
	if (RP_STRLEN(theUsernamePtr) >= kRpMaxUserNameLength) {
		RP_PRINTF("RpSetRemoteHostAuthInfo, username too long\n");
		return;
	}
	if (RP_STRLEN(thePasswordPtr) >= kRpMaxPasswordLength) {
		RP_PRINTF("RpSetRemoteHostAuthInfo, password too long\n");
		return;
	}
#endif

	theDataPtr = (rpDataPtr) theTaskDataPtr;

// +++ 11/26/2003, jacob_shih
// to encode the empty username
#if 0
	if (*theUsernamePtr == '\0') {
		/*
			If the Username is empty, clear the credentials.
		*/
		*theDataPtr->fRemoteHostEncodedAuth = '\0';
	}
	else {
		RP_STRCPY(theUserPassword, theUsernamePtr);
		if (*thePasswordPtr != '\0') {
			RP_STRCAT(theUserPassword, kColon);
			RP_STRCAT(theUserPassword, thePasswordPtr);
		}

		AsBuildBase64String(theUserPassword,
							(Unsigned16) RP_STRLEN(theUserPassword),
							theDataPtr->fRemoteHostEncodedAuth);
	}
#else
		RP_STRCPY(theUserPassword, theUsernamePtr);
		RP_STRCAT(theUserPassword, kColon);
		RP_STRCAT(theUserPassword, thePasswordPtr);

		AsBuildBase64String(theUserPassword,
							(Unsigned16) RP_STRLEN(theUserPassword),
							theDataPtr->fRemoteHostEncodedAuth);
#endif
// --- 11/26/2003, jacob_shih

	return;
}

#endif
#endif	/* RpRhLocalCredentials */


void RpHandleNeedsRemoteHostRequestBlock(rpConnectionPtr
											theClientConnectionPtr) {
	rpDataPtr			theDataPtr;
	rpHttpRequestPtr	theClientRequestPtr;
	rpRHRequestPtr		theRHRequestPtr;

	theClientRequestPtr = theClientConnectionPtr->fHttpRequestPtr;
	theDataPtr = theClientConnectionPtr->fDataPtr;

	/*
		If there is not a Remote Host IP address, then reject the request.
	*/
#if RpRemoteHostMulti
	if (theDataPtr->fRemoteHostIpAddress[theClientRequestPtr->fRemoteHostIndex] == 0) {
		theClientConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
		return;
	}
#else
	if (theDataPtr->fRemoteHostIpAddress == 0) {
		theClientConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
		return;
	}
#endif

	theRHRequestPtr = GetRemoteHostRequestBlock(theDataPtr);

	if (theRHRequestPtr != (rpRHRequestPtr) 0) {
		/*
			We found a free Remote Host request block.
			Set up a pointer from the Client's connection block
			to the Remote Host request block.
		*/
		theClientConnectionPtr->fRHRequestPtr = theRHRequestPtr;

		/*
			Set up a pointer from the Remote Host request block
			to the Client connection block.
		*/
		theRHRequestPtr->fClientConnectionPtr = theClientConnectionPtr;

		/*
			Suspend processing on the Client connection while we
			attempt to contact the Remote Host.
		*/
		theClientConnectionPtr->fState = eRpConnectionHolding;
		/*
			When the Client connection resumes, we should be
			ready to send the response headers.
		*/
		theClientRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;

		/*
			Set up a pointer from the Client's request block to
			the Remote Host date string buffer.
		*/
		theClientRequestPtr->fRemoteDateStringPtr =
										theRHRequestPtr->fWorkBuffer;

#if RpRemoteHostMulti
		/*
			Set the Remote Host index in the Remote Host request block.
		*/
		theRHRequestPtr->fHostIndex = theClientRequestPtr->fRemoteHostIndex;
#endif
	}

	return;
}


/*
	RpCaptureRemoteHostRequest
*/

rpRHRequestPtr RpCaptureRemoteHostRequest(rpDataPtr theDataPtr) {
	rpRHRequestPtr		theRHRequestPtr;
	Unsigned8			theIndex;

	for (theIndex = 0; theIndex < kStcpActiveConnections; theIndex++) {
		theRHRequestPtr = &theDataPtr->fRHRequests[theIndex];
		if (theRHRequestPtr->fRemoteRequestState ==
					eRpRemoteRequestNeedsConnection) {

			return theRHRequestPtr;
		}
	}

	return (rpRHRequestPtr) 0;
}


void RpFreeRemoteHostRequestBlock(rpConnectionPtr theConnectionPtr) {
	rpRHRequestPtr		theRHRequestPtr;

	theRHRequestPtr = theConnectionPtr->fRHRequestPtr;
	theConnectionPtr->fRHRequestPtr = (rpRHRequestPtr) 0;

	if (theRHRequestPtr->fServerConnectionPtr == theConnectionPtr) {
		theRHRequestPtr->fServerConnectionPtr = (rpConnectionPtr) 0;
		if (theRHRequestPtr->fClientConnectionPtr == (rpConnectionPtr) 0) {
			theRHRequestPtr->fRemoteRequestState = eRpRemoteRequestFree;
		}
	}
	else {
		if (theRHRequestPtr->fClientConnectionPtr == theConnectionPtr) {
			theRHRequestPtr->fClientConnectionPtr = (rpConnectionPtr) 0;
			if (theRHRequestPtr->fServerConnectionPtr == (rpConnectionPtr) 0) {
				theRHRequestPtr->fRemoteRequestState = eRpRemoteRequestFree;
			}
		}
	}

	return;
}


/*
	RpOpenRemoteHostConnection will try to open a connection
	to the Remote Host.
*/
void RpOpenRemoteHostConnection(rpConnectionPtr theConnectionPtr,
								rpRHRequestPtr theRHRequestPtr) {
	RpErrorCode			theResult;
	rpConnectionPtr		theClientConnectionPtr;

	/*
		Set up the links between the Remote Host connection block
		and the Remote Host request block.
	*/
	theConnectionPtr->fRHRequestPtr = theRHRequestPtr;
	theConnectionPtr->fParsingControl.fTempBufferPtr =
			theRHRequestPtr->fTempLineBuffer;
	theRHRequestPtr->fServerConnectionPtr = theConnectionPtr;
	theClientConnectionPtr =
			(rpConnectionPtr) theRHRequestPtr->fClientConnectionPtr;

	/*
		Indicate the the request block is linked to a
		connection block.
	*/
	theRHRequestPtr->fRemoteRequestState = eRpRemoteRequestHasConnection;
	theConnectionPtr->fProtocol = eRpHttpProxy;
	theConnectionPtr->fSkipHttpRequestBlock = True;

	/*
		Set up the connection error callback routines.
	*/
	theConnectionPtr->fErrorProcPtr = &ServerConnectionError;
	theClientConnectionPtr->fErrorProcPtr = &ClientConnectionError;

	theResult = StcpOpenActive(theConnectionPtr->fIndex,
								theConnectionPtr->fIpRemote,
								kHttpPort);

	if (theResult == eRpNoError) {
		theConnectionPtr->fState = eRpConnectionProtocolWaitExternal;
		theRHRequestPtr->fRemoteHostState =
									eRpRemoteHostWaitingForConnection;
	}
	else {
		/*
			We got an error opening the Remote Host connection.
		*/
		theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
	}

	return;
}


/*
	RpHandleRemoteHostAction will handle sending a request to the
	Remote Host and processing it's responses.

	The tasks of this routine are:

		* generating the request and sending it to the Remote Host.

		* parsing the headers received in the response from the
		  Remote Host.

		* setting up the states to send response headers to the Client.

		* setting up the states for sending the object received from
		  the Remote Host to the Client.

	Parameters:
		theConnectionPtr	pointer to the Remote Host connection block
*/

RpErrorCode RpHandleRemoteHostAction(rpConnectionPtr theConnectionPtr) {
	RpErrorCode			theResult;
	rpRHRequestPtr		theRHRequestPtr;
	rpHttpRequestPtr	theClientRequestPtr;
	rpConnectionPtr		theClientConnectionPtr;
	rpParsingControlPtr	theParsingPtr;
	Signed32			theLength;
	StcpStatus			theCompletionStatus;
	Boolean				theReadMoreFlag;
#if RpRhLocalCredentials
	rpDataPtr			theDataPtr;
#endif

	theResult = eRpNoError;
	theRHRequestPtr = theConnectionPtr->fRHRequestPtr;
	theClientConnectionPtr = theRHRequestPtr->fClientConnectionPtr;
	theClientRequestPtr = theClientConnectionPtr->fHttpRequestPtr;

	switch (theRHRequestPtr->fRemoteHostState) {

		case eRpRemoteHostWaitingForConnection:
			/*
				This is the initial state seen by this routine.

				Wait for the connection to the Remote Host to open,
				then set up the states to send the request.
			*/
			theResult =	StcpActiveConnectionStatus(theConnectionPtr->fIndex,
							&theCompletionStatus,
							(StcpIpAddress *) &theConnectionPtr->fIpLocal);

			if (theResult == eRpNoError) {
				if (theCompletionStatus == eStcpComplete) {
					/*
						We have a connection.  Set up the states to send
						the request to the Remote URL Server.
					*/
					theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
					theRHRequestPtr->fRemoteHostState = eRpRemoteHostSendRequest;
				}
				/*
					else keep waiting...
				*/
			}
			break;

		case eRpRemoteHostSendRequest:
			/*
				An active connection to the Remote Host has
				already been established.

				Build the request headers.
			*/
			theLength = BuildRequestHeaders(theRHRequestPtr);

			if (theLength > 0) {
				/*
					Send the request.
				*/
				theResult = StcpSend(theConnectionPtr->fIndex,
										theRHRequestPtr->fWorkBuffer,
										(StcpLength) theLength);
			}
			else {
				/*
					The work buffer was too small for the headers!
					Cause the connections to be aborted by simulating
					a send error.
				*/
#if RomPagerDebug
				RP_PRINTF("Remote Host Work buffer overflow in BuildRequestHeaders\n");
#endif

				theResult = eRpTcpSendError;
			}

				if (theResult == eRpNoError) {
					theConnectionPtr->fState = eRpConnectionSendingReply;

					if (theClientRequestPtr->fHttpCommand ==
														eRpHttpPostCommand) {
						/*
							When the send completes, we'll want to send the
							data that goes with the POST.
						*/
						theRHRequestPtr->fRemoteHostState =
														eRpRemoteHostSendData;
					}
					else {
						/*
							When the send completes, we'll want to initiate a
							receive for the reply.
						*/
						theRHRequestPtr->fRemoteHostState =
												eRpRemoteHostIssueFirstReceive;
					}
				}
			break;

		case eRpRemoteHostSendData:
			/*
				Send the data for the "POST".
			*/
			theResult = StcpSend(theConnectionPtr->fIndex,
							theClientRequestPtr->fHttpWorkBuffer,
							(StcpLength) theClientRequestPtr->fPostRequestLength);

			if (theResult == eRpNoError) {
				theConnectionPtr->fState = eRpConnectionSendingReply;

				/*
					When the send completes, we'll want to initiate a
					receive for the reply.
				*/
				theRHRequestPtr->fRemoteHostState =
											eRpRemoteHostIssueFirstReceive;
			}
			break;

		case eRpRemoteHostIssueFirstReceive:
			/*
				Initiate a receive for the reply to our request.
			*/
			theResult = StcpReceive(theConnectionPtr->fIndex);

			if (theResult == eRpNoError) {
				/*
					Set up the connection state to get the reply from the
					Remote Host.
				*/
				theConnectionPtr->fState = eRpConnectionWaitingForReceive;

				/*
					The next time this routine gets called, we should have
					received the response headers from the Remote Host.
				*/
				theRHRequestPtr->fRemoteHostState = eRpRemoteHostParsingHeaders;
			}
			break;

		case eRpRemoteHostParsingHeaders:
			/*
				Parse the reply headers from the Remote Host.
			*/
			theReadMoreFlag = ParseRemoteHttpHeaders(theConnectionPtr->fDataPtr,
														theRHRequestPtr);
			if (theReadMoreFlag) {
				/*
					We need more data.  Issue another receive.
				*/
				theResult = StcpReceive(theConnectionPtr->fIndex);

				if (theResult == eRpNoError) {
					theConnectionPtr->fState = eRpConnectionWaitingForReceive;
				}
			}
			break;

		case eRpRemoteHostAnalyzeResponse:
			/*
				We are done parsing the reply headers from the Remote Host.
			*/
			if (theRHRequestPtr->fHttpResponseState == eRpHttpNormal) {

#if RomPagerSoftPages
				/*
					Parse HTML objects--they might be Soft Pages.
				*/
				if (theRHRequestPtr->fObject.fMimeDataType == eRpDataTypeHtml) {
					theClientRequestPtr->fSoftPageFlag = True;
					PreprocessSoftPage(theConnectionPtr->fDataPtr,
							theConnectionPtr->fIndexValues);
				}
#endif

				/*
					We got a normal response from the Remote Host so
					set up the states to send a normal reply to the
					Client.
				*/
				theClientRequestPtr->fHttpResponseState = eRpHttpNormal;
				theClientRequestPtr->fObjectPtr = &theRHRequestPtr->fObject;
				theClientConnectionPtr->fState = eRpConnectionNeedsProtocolAction;

				/*
					Suspend action on the Remote Host connection
					while we send the response headers to the Client.
				*/
				theConnectionPtr->fState = eRpConnectionHolding;

				/*
					Determine what state we should be in when we do resume
					action on the Remote Host connection.
				*/
				theParsingPtr = theRHRequestPtr->fParsingControlPtr;

				if (theParsingPtr->fIncomingBufferLength > 0) {
					/*
						There is still some data in the incoming buffer.
						We will need to process it.
					*/
					theRHRequestPtr->fRemoteHostState = eRpRemoteHostReceiveObject;
				}
				else {
					/*
						The incoming buffer is empty.
						We will need to issue a receive.
 					*/
					theRHRequestPtr->fRemoteHostState = eRpRemoteHostIssueReceive;
				}
			}
			else if (theRHRequestPtr->fHttpResponseState == eRpHttpNotModified) {
				/*
					We got a Not Modified response from the Remote Host so
					set up the states to send a Not Modified response to
					the Client.
				*/
				theClientRequestPtr->fHttpResponseState = eRpHttpNotModified;
				theClientConnectionPtr->fState = eRpConnectionNeedsProtocolAction;

				/*
					There is no object data with a Not Modified response,
					so close the Remote Host connection.
				*/
				(void) RpConnectionCloseTcp(theConnectionPtr);
			}
			else if (theRHRequestPtr->fHttpResponseState == eRpHttpRedirect) {
				/*
					We got a Redirect response from the Remote Host.
					Just pass it on to the Client.
				*/
				theClientRequestPtr->fHttpResponseState = eRpHttpRedirect;
				theClientRequestPtr->fObjectPtr = &theRHRequestPtr->fObject;
				theClientRequestPtr->fObjectPtr->fURL = theRHRequestPtr->fRedirectPath;
				theClientConnectionPtr->fState = eRpConnectionNeedsProtocolAction;

#if RomPagerQueryIndex
				/*
					Suppress any query indices that might be lying around.
				*/
				theClientConnectionPtr->fIndexDepth = -1;
#endif

				/*
					There is no object data with a Redirect response,
					so close the Remote Host connection.
				*/
				(void) RpConnectionCloseTcp(theConnectionPtr);
			}
#if RomPagerSecurity
			else if (theRHRequestPtr->fHttpResponseState == eRpHttpNeedBasicAuthorization) {

#if RpRhLocalCredentials
				/*
					If the Remote Host authorization credentials are stored locally
					and we've only sent them once, set up the states to send the
					request again in order to respond to a security challenge.
				*/
				theDataPtr = theConnectionPtr->fDataPtr;

#if RpRemoteHostMulti
				if (*theDataPtr->fRemoteHostEncodedAuth[
						theRHRequestPtr->fHostIndex] != '\0' &&
						theRHRequestPtr->fAuthorizationCount < 2)
#else
				if (*theDataPtr->fRemoteHostEncodedAuth != '\0' &&
						theRHRequestPtr->fAuthorizationCount < 2)
#endif
				{
					/*
						Set up the states to send the request again.
					*/
					theRHRequestPtr->fRemoteRequestState = eRpRemoteRequestNeedsConnection;

					/*
						Initialize the response state to object not found.
					*/
					theRHRequestPtr->fHttpResponseState = eRpHttpNoObjectFound;

					/*
						Set the request and object default values.
					*/
					theRHRequestPtr->fRemoteHostState = eRpRemoteHostSendRequest;
					theRHRequestPtr->fDateStringPtr = theRHRequestPtr->fWorkBuffer;
					theRHRequestPtr->fObject.fMimeDataType = eRpDataTypeHtml;
					theRHRequestPtr->fObject.fExtensionPtr = (rpObjectExtensionPtr) 0;

				/*
					There may be object data with the Security Challenge response,
					but we don't want it.
				*/
				(void) RpConnectionAbortTcp(theConnectionPtr);
			}
				else {
					/*
						We either don't have credentials for the Remote Host or
						we've sent them more than once.  Set up the states to
						return a 404 Not Found response to the requestor.
					*/
					theRHRequestPtr->fRemoteHostState = eRpRemoteHostNoObjectFound;
				}

#else	/* !RpRhLocalCredentials */

				/*
					We got a Security Challenge response from the Remote Host.
					Set up the states to send a challenge to the Client.
				*/
				theClientRequestPtr->fHttpResponseState = eRpHttpNeedBasicAuthorization;
				theClientRequestPtr->fObjectPtr = &gRpAccessNotAllowedPage;
				theClientRequestPtr->fRealmPtr = &theRHRequestPtr->fRealm;
				theClientConnectionPtr->fState = eRpConnectionNeedsProtocolAction;

				/*
					There may be object data with the Security Challenge response,
					but we don't want it.
				*/
				(void) RpConnectionAbortTcp(theConnectionPtr);

#endif	/* RpRhLocalCredentials */
			}
#endif	/* RomPagerSecurity */
			else {
				/*
					We did not get a 200 OK response from the Remote Host.
					It could be that the object we requested was not found
					or authentication is required, or a number of other
					reasons.  We will treat this as an object not found.

					Set up the states to abort the connection to the Remote
					Host and send a "404 Not Found" response to the Client.
				*/
				theRHRequestPtr->fRemoteHostState = eRpRemoteHostNoObjectFound;
			}
			break;

		case eRpRemoteHostReceiveObject:
			/*
				We have received some object data from the Remote Host.

				Copy the object data from the receive buffer of
				the Remote Host connection to the response
				buffer in the Client connection's request block.

				Set up the states to send the object data to the Client.
			*/
			theParsingPtr = theRHRequestPtr->fParsingControlPtr;

// +++ debug 11/26/2003
////xprintf("tt...(%d)<%4d>[%s]eRpRemoteHostReceiveObject\n", theConnectionPtr->fIndex, theParsingPtr->fIncomingBufferLength, theRHRequestPtr->fClientConnectionPtr->fHttpRequestPtr->fPath);
// --- debug 11/26/2003
			if (theParsingPtr->fIncomingBufferLength <=
					theClientRequestPtr->fFillBufferAvailable) {
				/*
					There is enough room in the current fill buffer
					to fit the complete receive buffer.
				*/
				theLength = theParsingPtr->fIncomingBufferLength;
			}
			else {
				/*
					There is NOT enough room in the current fill buffer
					to fit the complete receive buffer.
				*/
#if RomPagerSoftPages
				if (theClientRequestPtr->fSoftPageFlag) {
					/*
						Abort the remote host connection and let the
						Soft Page code take care of putting out the
						error for the client connection.
					*/
					theResult = RpConnectionAbortTcp(theConnectionPtr);
					RpInitializeParsingHtml(theClientConnectionPtr, False);
					theClientConnectionPtr->fState =
							eRpConnectionNeedsProtocolAction;
					break;
				}
#endif
				theLength = theClientRequestPtr->fFillBufferAvailable;

			}

			if (theLength > 0) {
				/*
					Copy the object data from the Remote Host Connection's receive
					buffer to the Client connection's current response buffer.
				*/
				RP_MEMCPY(theClientRequestPtr->fHtmlFillPtr,
						theParsingPtr->fCurrentBufferPtr, theLength);

				/*
					Update the pointers and lengths.
				*/
				theParsingPtr->fCurrentBufferPtr += theLength;
				theParsingPtr->fIncomingBufferLength -= theLength;
				theClientRequestPtr->fHtmlFillPtr += theLength;
				theClientRequestPtr->fFillBufferAvailable -= (Unsigned16) theLength;
			}

			/*
				Send the buffer if it's full.
			*/
			if (theClientRequestPtr->fFillBufferAvailable == 0) {

				theClientRequestPtr->fHtmlBufferReady = False;
				RpFlipResponseBuffers(theClientRequestPtr);
/* Arthur 2004.09.02 - For Commander SSL Enable Issue */
//				xprintf("SSL: fIsTlsFlag=%d fHtmlResponseLength=%d\n", theClientConnectionPtr->fIsTlsFlag, theClientRequestPtr->fHtmlResponseLength);
#if RomPagerSecure || RomWebClientSecure
		if (theClientConnectionPtr->fSslContext!=0) //SSL
		{
				theResult = SSLSend(theClientConnectionPtr->fSslContext,
									theClientRequestPtr->fHtmlResponsePtr,
									theClientRequestPtr->fHtmlResponseLength);
				if (theResult != SSL_ERROR)
					theResult = eRpNoError;
		}
		else
#endif
/******************************************************/
				theResult = StcpSend(theClientConnectionPtr->fIndex,
									theClientRequestPtr->fHtmlResponsePtr,
									theClientRequestPtr->fHtmlResponseLength);

				if (theResult == eRpNoError) {
					/*
						Suspend action on the Remote Host connection while
						the object data is being sent to the Client.
					*/
					theConnectionPtr->fState = eRpConnectionHolding;
					theClientConnectionPtr->fState = eRpConnectionSendingReply;
					if (theParsingPtr->fIncomingBufferLength == 0) {
						/*
							Once the data has been sent to the Client, we need to
							issue another receive on the Remote Host connection.
						*/
						theRHRequestPtr->fRemoteHostState = eRpRemoteHostIssueReceive;
					}
				}
				else {
					/*
						We got a send error on the Client connection.
						Set theConnectionPtr to the Client Connection so the
						proper connection error callback routine is called.
					*/
					theConnectionPtr = theClientConnectionPtr;
				}
			}
			else {
				/*
					The send buffer is not full.  Set up the states to get
					more data from the Remote Host.
				*/
				theRHRequestPtr->fRemoteHostState = eRpRemoteHostIssueReceive;
			}
			break;

		case eRpRemoteHostIssueReceive:
			/*
				Issue a receive on the Remote Host connection.
			*/
			theResult = StcpReceive(theConnectionPtr->fIndex);

			if (theResult == eRpNoError) {
				/*
					Set up the connection state to get the reply from the
					Remote Host.
				*/
				theConnectionPtr->fState = eRpConnectionWaitingForReceive;
				theRHRequestPtr->fRemoteHostState = eRpRemoteHostReceiveObject;
			}
// +++ debug 11/26/2003
////
#if 0
xprintf("gg...(%d)<%04x>[%s]:%d:eRpRemoteHostIssueReceive\n",
	theConnectionPtr->fIndex, (Unsigned16)theResult, theRHRequestPtr->fClientConnectionPtr->fHttpRequestPtr->fPath, theRHRequestPtr->fRemoteHostState);
#endif
// --- debug 11/26/2003
			break;

		case eRpRemoteHostObjectComplete:
			/*
				We have received the whole object from the Remote Host.

				Close the Remote Host connection.
			*/
			(void) RpConnectionCloseTcp(theConnectionPtr);

#if RomPagerSoftPages
			if (theClientRequestPtr->fSoftPageFlag) {
				/*
					Now let the Soft Page code parse the
					object in the Soft Page parse buffer.
				*/
				RpInitializeParsingHtml(theClientConnectionPtr, True);
				theClientConnectionPtr->fState =
						eRpConnectionNeedsProtocolAction;
				break;
			}
#endif

			/*
				Make sure we have sent all of the object data to the
				Client.
			*/
			if (theClientRequestPtr->fFillBufferAvailable < kHtmlMemorySize) {
				/*
					We have some residual object data in the buffer.
					Send it to the Client.
				*/
				theClientRequestPtr->fHtmlBufferReady = False;
				theClientRequestPtr->fHttpTransactionState =
						eRpHttpResponseComplete;
				RpFlipResponseBuffers(theClientRequestPtr);

/* Arthur 2004.09.02 - For Commander SSL Enable Issue */
#if RomPagerSecure || RomWebClientSecure
		if (theClientConnectionPtr->fSslContext!=0) //SSL
		{
				theResult = SSLSend(theClientConnectionPtr->fSslContext,
									theClientRequestPtr->fHtmlResponsePtr,
									theClientRequestPtr->fHtmlResponseLength);
				if (theResult != SSL_ERROR)
					theResult = eRpNoError;
		}
		else
#endif
/******************************************************/
				theResult = StcpSend(theClientConnectionPtr->fIndex,
									theClientRequestPtr->fHtmlResponsePtr,
									theClientRequestPtr->fHtmlResponseLength);

				if (theResult == eRpNoError) {
					theClientConnectionPtr->fState = eRpConnectionSendingReply;
					theClientRequestPtr->fHttpTransactionState =
							eRpHttpResponseComplete;
				}
				else {
					/*
						We got a send error on the Client connection.
						Set theConnectionPtr to the Client Connection so the
						proper connection error callback routine is called.
					*/
					theConnectionPtr = theClientConnectionPtr;
				}
			}
			else {
				/*
					There is no more data to send to the Client.
					Set up the response complete states.
				*/
				theClientConnectionPtr->fState =
						eRpConnectionNeedsProtocolAction;
				theClientRequestPtr->fHttpTransactionState =
						eRpHttpResponseComplete;
			}
			break;

		case eRpRemoteHostNoObjectFound:
			/*
				Either we did not receive a "200 OK" response from the
				Remote Host or the headers it sent us were unparseable.

				Abort the Remote Host connection and set up the states
				to send a "404 Not Found" response to the Client.
			*/

			(void) RpConnectionAbortTcp(theConnectionPtr);

			theClientConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
			theClientRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
			theClientRequestPtr->fHttpResponseState = eRpHttpNoObjectFound;
			theClientRequestPtr->fObjectPtr = &gRpHttpNoObjectFoundPage;
			theClientRequestPtr->fObjectSource = eRpRomUrl;
			break;

		default:
			break;
	}

	if (theResult != eRpNoError) {
		/*
			There must have been an error on one of the connections.
		*/

		theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
	}

	return theResult;
}


/*
	This routine is called if there's an error on the Remote Host
	connection.

	Abort the Client connection and close or abort the Remote Host
	connection.  Aborting the Client connection is the best way to
	signal that there was an error on the Remote Host connection.
*/

static RpErrorCode ServerConnectionError(rpConnectionPtr theConnectionPtr) {
	RpErrorCode			theResult = eRpNoError;
	rpConnectionPtr		theClientConnectionPtr = (rpConnectionPtr) 0;
	rpRHRequestPtr		theRHRequestPtr = (rpRHRequestPtr) 0;

	theRHRequestPtr = theConnectionPtr->fRHRequestPtr;

	if (theRHRequestPtr != (rpRHRequestPtr) 0) {
		theClientConnectionPtr = theRHRequestPtr->fClientConnectionPtr;
	}

	if (theClientConnectionPtr != (rpConnectionPtr) 0) {
		/*
			We have a Client connection and there's been an error on the
			Remote Host Connection.
		*/
// +++ debug 11/26/2003
//xprintf("yy...ServerConnectionError() (%d)[%s]\n", theConnectionPtr->fIndex, theRHRequestPtr->fClientConnectionPtr->fHttpRequestPtr->fPath);
// --- debug 11/26/2003
		if (theRHRequestPtr->fRemoteHostState == eRpRemoteHostReceiveObject) {
			/*
				It looks like we were receiving an object from the
				Remote Host.  Assume that the object is complete and
				the Remote Host closed the connection.

				Set up the states to send the remainder of the object
				to the Client and close the connections.
			*/
			theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
			theRHRequestPtr->fRemoteHostState = eRpRemoteHostObjectComplete;
			return theResult;
		}
		else {
			/*
				We weren't in the process of receiving an object from the
				Remote Host.

				Since we got an error on the Remote Host connection,
				cause an error on the Client connection by aborting it.
			*/
			theResult = RpConnectionAbortTcp(theClientConnectionPtr);
		}
	}

	/*
		Close or abort the Remote Host connection.
	*/
	(void) RpConnectionCheckTcpClose(theConnectionPtr);

	return theResult;
}


/*
	This routine is called if there is an error on the Client connection.

	Abort the connection to the Remote Host and close or abort the
	Client connection.
*/

static RpErrorCode ClientConnectionError(rpConnectionPtr theConnectionPtr) {
	RpErrorCode			theResult = eRpNoError;
	rpRHRequestPtr		theRHRequestPtr;
	rpConnectionPtr		theServerConnectionPtr;

	if (theConnectionPtr->fRHRequestPtr != (rpRHRequestPtr) 0) {
		theRHRequestPtr = theConnectionPtr->fRHRequestPtr;

		if (theRHRequestPtr->fServerConnectionPtr != (rpConnectionPtr) 0) {
			theServerConnectionPtr = theRHRequestPtr->fServerConnectionPtr;
			theResult = RpConnectionAbortTcp(theServerConnectionPtr);
		}
	}

	/*
		Close or abort the Client connection.
	*/
	(void) RpConnectionCheckTcpClose(theConnectionPtr);

	return theResult;
}


/*
	Build the headers for the request to the Remote Host.
*/

static Signed32 BuildRequestHeaders(rpRHRequestPtr theRHRequestPtr) {
	rpHttpRequestPtr	theClientRequestPtr;
	rpConnectionPtr		theConnectionPtr;
	char *				theSendBufferPtr;
#if RomPagerDebug
	Signed32			theLength;
	Signed32			theLengthRemaining;
#endif
#if RomPagerSecurity
	char *				theAuthPtr;
#endif
#if RpRhLocalCredentials
	rpDataPtr			theDataPtr;
#endif

	theConnectionPtr = theRHRequestPtr->fServerConnectionPtr;
	theClientRequestPtr =
		theRHRequestPtr->fClientConnectionPtr->fHttpRequestPtr;

	/*
		Generate an HTTP 1.0 request for the URL.

		Example:
			GET /MainPage HTTP/1.0<CR><LF>
			Host: xxx.xxx.xxx.xxx<CR><LF>
			If-Modified-Since: Tues, 6 May 1997 14:02:44 GMT<CR><LF>
			User-Agent: Allegro-Software-Remote Host/x.xx<CR><LF>
			Accept: asterisk/asterisk<CR><LF>
	*/
	theSendBufferPtr = theRHRequestPtr->fWorkBuffer;
#if RomPagerDebug
	theLengthRemaining = kRemoteHostWorkBufferSize;
#endif

	if (theClientRequestPtr->fHttpCommand == eRpHttpPostCommand) {
		/*
			Build the "POST" header.
		*/
#if RomPagerDebug
		theLengthRemaining -= sizeof(kHttpGet) - 1;
#endif
		RP_STRCPY(theSendBufferPtr, kHttpPost);
	}
	else {
		/*
			Build the "GET" header.
		*/
#if RomPagerDebug
		theLengthRemaining -= sizeof(kHttpGet) - 1;
#endif
		RP_STRCPY(theSendBufferPtr, kHttpGet);
	}

#if RomPagerDebug
	theLength = RP_STRLEN(theClientRequestPtr->fPath);
	if ((theLengthRemaining -= theLength) < 0) {
		return -1;
	}
#endif
	RP_STRCAT(theSendBufferPtr, theClientRequestPtr->fPath);

#if RomPagerDebug
	if ((theLengthRemaining -= (sizeof(kHttpEndRemoteUrl) - 1)) < 0) {
		return -1;
	}
#endif
	RP_STRCAT(theSendBufferPtr, kHttpEndRemoteUrl);

	/*
		Build the "Host" header.
	*/
#if RomPagerDebug
	if ((theLengthRemaining -= (sizeof(kHttpHost) +
								sizeof(kCRLF) + 14)) < 0) {
		return -1;
	}
#endif
	RP_STRCAT(theSendBufferPtr, kHttpHost);
	theSendBufferPtr += RP_STRLEN(theSendBufferPtr);
	theSendBufferPtr += ConvertUnsigned32ToDotString(
			theConnectionPtr->fIpRemote, theSendBufferPtr);
	RP_STRCPY(theSendBufferPtr, kCRLF);

	/*
		Build the "If-Modified-Since" header if the Client
		sent us one.
	*/
	if (theClientRequestPtr->fBrowserDate != 0) {
#if RomPagerDebug
		if ((theLengthRemaining -= (sizeof(kHttpIfModified) + 16)) < 0) {
			return -1;
		}
#endif
		RP_STRCAT(theSendBufferPtr, kHttpIfModified);
		theSendBufferPtr += RP_STRLEN(theSendBufferPtr);
		RpBuildDateString(theConnectionPtr->fDataPtr, theSendBufferPtr,
							theClientRequestPtr->fBrowserDate);
	}

	/*
		Build the "User-Agent" header.
	*/
#if RomPagerDebug
	if ((theLengthRemaining -= ((sizeof(kHttpUserAgent) - 1) +
								(sizeof(kRemoteHostClientHeader) - 1))) < 0) {
		return -1;
	}
#endif
	RP_STRCAT(theSendBufferPtr, kHttpUserAgent);
	RP_STRCAT(theSendBufferPtr, kRemoteHostClientHeader);

#if RomPagerSecurity
	/*
		Determine where we're getting the credentials and set up
		theAuthPtr accordingly.
	*/
#if RpRhLocalCredentials
	theDataPtr = theConnectionPtr->fDataPtr;
#if RpRemoteHostMulti
	theAuthPtr = theDataPtr->fRemoteHostEncodedAuth[theRHRequestPtr->fHostIndex];
#else
	theAuthPtr = theDataPtr->fRemoteHostEncodedAuth;
#endif	/* RpRemoteHostMulti */	
#else
	theAuthPtr = theClientRequestPtr->fEncodedUserPassword;
#endif	/* RpRhLocalCredentials */

	if (*theAuthPtr != '\0') {
		/*
			Build the "Authorization" header.
		*/
#if RomPagerDebug
		theLength = RP_STRLEN(theAuthPtr);
		if ((theLengthRemaining -= ((sizeof(kHttpAuthorizationBasic) - 1) +
									(sizeof(kCRLF) - 1) + theLength)) < 0) {
			return -1;
		}
#endif
		RP_STRCAT(theSendBufferPtr, kHttpAuthorizationBasic);
		RP_STRCAT(theSendBufferPtr, theAuthPtr);
		RP_STRCAT(theSendBufferPtr, kCRLF);
#if RpRhLocalCredentials
		theRHRequestPtr->fAuthorizationCount++;
#endif
	}
#endif	/* RomPagerSecurity */

	if (theClientRequestPtr->fHttpCommand == eRpHttpPostCommand) {
		/*
			Build the "Content-Type" header.
		*/
		if (*theClientRequestPtr->fHttpContentType != '\0') {
#if RomPagerDebug
			theLength = RP_STRLEN(theClientRequestPtr->fHttpContentType);
			if ((theLengthRemaining -= ((sizeof(kHttpContentType) - 1) +
										(sizeof(kCRLF) - 1) + theLength)) < 0) {
				return -1;
			}
#endif
			RP_STRCAT(theSendBufferPtr, kHttpContentType);
			RP_STRCAT(theSendBufferPtr, theClientRequestPtr->fHttpContentType);
			RP_STRCAT(theSendBufferPtr, kCRLF);
		}

		/*
			Build the "Content-Length" header.
		*/
#if RomPagerDebug
		if ((theLengthRemaining -= ((sizeof(kHttpContentLength) - 1) +
									(sizeof(kCRLF) - 1) + 4)) < 0) {
			return -1;
		}
#endif
		RP_STRCAT(theSendBufferPtr, kHttpContentLength);
		RpCatSigned32ToString(theClientRequestPtr->fPostRequestLength,
				theSendBufferPtr);
		RP_STRCAT(theSendBufferPtr, kCRLF);
	}

	/*
		Build the "Accept" header.  This is the last header
		so we need the extra <CR, LF>.
	*/
#if RomPagerDebug
	if ((theLengthRemaining -= (sizeof(kHttpAcceptAll) - 1)) < 0) {
		return -1;
	}
#endif
	RP_STRCAT(theSendBufferPtr, kHttpAcceptAll);
	RP_STRCAT(theSendBufferPtr, kCRLF);

	/*
		Return the length of the headers.
	*/
	return RP_STRLEN(theRHRequestPtr->fWorkBuffer);
}


static Unsigned16 ConvertUnsigned32ToDotString(Unsigned32 theValue,
												char * theStringPtr) {
	Unsigned8		theByte;
	char *			theBeginStringPtr = theStringPtr;

	theByte = (Unsigned8) (theValue >> 24);
	theStringPtr += RpConvertUnsigned32ToAscii(theByte, theStringPtr);
	*theStringPtr++ = kAscii_Dot;

	theByte = (Unsigned8) (theValue >> 16);
	theStringPtr += RpConvertUnsigned32ToAscii(theByte, theStringPtr);
	*theStringPtr++ = kAscii_Dot;

	theByte = (Unsigned8) (theValue >> 8);
	theStringPtr += RpConvertUnsigned32ToAscii(theByte, theStringPtr);
	*theStringPtr++ = kAscii_Dot;

	theByte = (Unsigned8) theValue;
	theStringPtr += RpConvertUnsigned32ToAscii(theByte, theStringPtr);

	return theStringPtr - theBeginStringPtr;
}


static rpRHRequestPtr GetRemoteHostRequestBlock(rpDataPtr theDataPtr) {
	Unsigned8			theIndex;
	rpRHRequestPtr		theRHRequestPtr;

	for (theIndex = 0; theIndex < kStcpActiveConnections; theIndex++) {
		theRHRequestPtr = &theDataPtr->fRHRequests[theIndex];
		if (theRHRequestPtr->fRemoteRequestState == eRpRemoteRequestFree) {
			/*
				We found a free Remote Host request block.
				Mark it as in use.
			*/
			theRHRequestPtr->fRemoteRequestState = eRpRemoteRequestNeedsConnection;

			/*
				Initialize the response state to object not found.
			*/
			theRHRequestPtr->fHttpResponseState = eRpHttpNoObjectFound;

			/*
				Set the request and object default values.
			*/
			theRHRequestPtr->fRemoteHostState = eRpRemoteHostSendRequest;
			theRHRequestPtr->fDateStringPtr = theRHRequestPtr->fWorkBuffer;
			theRHRequestPtr->fObject.fMimeDataType = eRpDataTypeHtml;
			theRHRequestPtr->fObject.fExtensionPtr = (rpObjectExtensionPtr) 0;
#if RpRhLocalCredentials
			theRHRequestPtr->fAuthorizationCount = 0;
#endif
			return theRHRequestPtr;
		}
	}

	return (rpRHRequestPtr) 0;
}


static Boolean ParseRemoteHttpHeaders(rpDataPtr theDataPtr,
										rpRHRequestPtr theRHRequestPtr) {
	rpLineState				theLineState;
	Boolean					theReadMoreFlag;

	theReadMoreFlag = False;
	theLineState = RpParseReplyBuffer(theRHRequestPtr->fParsingControlPtr);

	switch (theLineState) {
		case eRpLineComplete:
			RpConvertHeaderToLowerCase(theRHRequestPtr->fParsingControlPtr);
			ParseRHHeader(theRHRequestPtr, theDataPtr->fHttpProxyPatternTable);
			break;

		case eRpLinePartial:
			theReadMoreFlag = True;
			break;

		case eRpLineError:
			/*
				The response headers from the Remote Host are
				unparseable!
			*/
			theRHRequestPtr->fRemoteHostState = eRpRemoteHostNoObjectFound;
			break;

		case eRpLineEmpty:
			/*
				We have an empty line (CR, LF or LF), so we
				are done parsing the headers.

				Next, we have to analyze the response headers.
			*/
			theRHRequestPtr->fRemoteHostState = eRpRemoteHostAnalyzeResponse;
			break;
	}

	return theReadMoreFlag;
}


static void ParseRHHeader(rpRHRequestPtr theRHRequestPtr,
						rpRHPatternTablePtr thePatternTablePtr) {
	rpParsingControlPtr	theParsingPtr;
	char *				theBeginLinePtr;
	char *				theStartOfTokenPtr;
	Unsigned16			theTokenLength;

	theParsingPtr = theRHRequestPtr->fParsingControlPtr;
	theBeginLinePtr = theParsingPtr->fCurrentBeginLinePtr;

	/*
		look for the HTTP header in the header table
	*/
	while (thePatternTablePtr->fPatternLength != 0) {
		if (RP_MEMCMP(theBeginLinePtr, thePatternTablePtr->fPattern,
					thePatternTablePtr->fPatternLength) == 0) {
			/*
				We found a line we need to do something with.
				Skip over the matching pattern....
			*/
			theBeginLinePtr += thePatternTablePtr->fPatternLength;
			/*
				Find the next token.
			*/
			theStartOfTokenPtr = RpFindTokenStart(theBeginLinePtr);
			theTokenLength = RpFindTokenEnd(theStartOfTokenPtr);
			/*
				Pass the token to the action routine.
			*/
			(*thePatternTablePtr->fAction)(theRHRequestPtr,
					theStartOfTokenPtr,
					theTokenLength);
			break;
		}
		else {
			thePatternTablePtr += 1;
		}
	}
	return;
}


/*
	This routine parses the HTTP version line of the response
	(the first line).

	Some examples of what this line may contain follow:
		HTTP/1.0 200 OK<CR><LF>
		HTTP/1.1 200 OK<CR><LF>
		HTTP/1.0 304 Not Modified<CR><LF>
		HTTP/1.0 401 Unauthorized<CR><LF>
*/

static void ProcessHttpVersion(rpRHRequestPtr theRHRequestPtr,
								char *theStartOfTokenPtr,
								Unsigned16 theTokenLength) {

	/*
		Skip past the second digit of the HTTP version number
		since we are only matching against the first seven
		characters of version string "HTTP/1." to get to
		this routine.
	*/
	theStartOfTokenPtr++;

	if (RP_MEMCMP(theStartOfTokenPtr, kPageFound,
				(sizeof(kPageFound) - 1)) == 0) {
		theRHRequestPtr->fHttpResponseState = eRpHttpNormal;
	}
	else if (RP_MEMCMP(theStartOfTokenPtr, kNotModified,
					(sizeof(kNotModified) - 1)) == 0) {
			theRHRequestPtr->fHttpResponseState = eRpHttpNotModified;
	}
	else if (RP_MEMCMP(theStartOfTokenPtr, kMoved, 4) == 0) {
			/*
				We've matched " 302".  It must be " 302 Found"
				or " 302 Moved Temporarily"
			*/
			theRHRequestPtr->fHttpResponseState = eRpHttpRedirect;
	}

	/*
		Initialize the work buffer to store incoming date headers.
	*/
	*theRHRequestPtr->fWorkBuffer = '\0';
	return;
}


static void ProcessContentType(rpRHRequestPtr theRHRequestPtr,
								char *theStartOfTokenPtr,
								Unsigned16 theTokenLength) {
	rpConnectionPtr		theClientConnectionPtr;
	rpHttpRequestPtr	theClientRequestPtr;
	rpDataType			theDataType;

	if (theTokenLength >= kMaxMimeTypeLength) {
		theRHRequestPtr->fRemoteHostState = eRpRemoteHostNoObjectFound;
	}
	else {
		/*
			Make it a C string.
		*/
		*(theStartOfTokenPtr + theTokenLength) = '\0';

		theDataType = RpStringToMimeType(theStartOfTokenPtr);

		if (theDataType == eRpDataTypeOther) {
			theClientConnectionPtr = theRHRequestPtr->fClientConnectionPtr;
			theClientRequestPtr = theClientConnectionPtr->fHttpRequestPtr;
			RP_STRCPY(theClientRequestPtr->fOtherMimeType, theStartOfTokenPtr);
		}

		theRHRequestPtr->fObject.fMimeDataType = theDataType;
	}
	return;
}


static void ProcessDate(rpRHRequestPtr theRHRequestPtr,
						char *theStartOfTokenPtr,
						Unsigned16 theTokenLength) {

	ProcessDateCommon(theRHRequestPtr, theStartOfTokenPtr, kHttpDate);

	return;
}


static void ProcessExpires(rpRHRequestPtr theRHRequestPtr,
							char *theStartOfTokenPtr,
							Unsigned16 theTokenLength) {

	ProcessDateCommon(theRHRequestPtr, theStartOfTokenPtr, kHttpExpires);

	return;
}


static void ProcessLastModified(rpRHRequestPtr theRHRequestPtr,
								char *theStartOfTokenPtr,
								Unsigned16 theTokenLength) {

	ProcessDateCommon(theRHRequestPtr, theStartOfTokenPtr, kHttpLastModified);

	return;
}


static void ProcessDateCommon(rpRHRequestPtr theRHRequestPtr,
								char *theStartOfTokenPtr,
								char *theStringPtr) {
	rpParsingControlPtr	theParsingPtr;
	Unsigned16			theTokenLength;

	theParsingPtr = theRHRequestPtr->fParsingControlPtr;

	theTokenLength = theParsingPtr->fCurrentEndOfLinePtr - theStartOfTokenPtr;

	RP_STRCPY(theRHRequestPtr->fDateStringPtr, theStringPtr);
	theRHRequestPtr->fDateStringPtr += RP_STRLEN(theStringPtr);

	RP_MEMCPY(theRHRequestPtr->fDateStringPtr, theStartOfTokenPtr, theTokenLength);
	theRHRequestPtr->fDateStringPtr += theTokenLength;

	*theRHRequestPtr->fDateStringPtr = '\0';

	return;
}


/*
	This routine processes the "Location" header which is
	received along with a redirect response from the
	Remote Host.

	The token passed to us with the "Location" header is:

		http://Remote Host ID/Object Path

	We need to build a string that begins with a forward slash
	followed by the Remote Host prefix that we need identify the
	remote host, followed by a forward slash then the Object Path:

		/Remote Host prefix/Object Path
*/

static void ProcessLocation(rpRHRequestPtr theRHRequestPtr,
								char *theStartOfTokenPtr,
								Unsigned16 theTokenLength) {
	Unsigned32	thePrefixLength;

	*theRHRequestPtr->fRedirectPath = '\0';
	*(theStartOfTokenPtr + theTokenLength) = '\0';

	theTokenLength = RpFindTokenDelimited(theStartOfTokenPtr, kAscii_Slash);
	if (*(theStartOfTokenPtr + theTokenLength) == kAscii_Slash) {
		if (*(theStartOfTokenPtr + theTokenLength + 1) == kAscii_Slash) {
			/*
				Skip over the double slash.
			*/
			theStartOfTokenPtr += theTokenLength + 2;

			/*
				Find another slash.
			*/
			theTokenLength = RpFindTokenDelimited(theStartOfTokenPtr, 
					kAscii_Slash);
			if (*(theStartOfTokenPtr + theTokenLength) == kAscii_Slash) {
				theStartOfTokenPtr += theTokenLength;
				RP_STRCPY(theRHRequestPtr->fRedirectPath, kSlash);
				RP_STRCAT(theRHRequestPtr->fRedirectPath, kRemoteHostPrefix);
				thePrefixLength = sizeof(kRemoteHostPrefix);
#if RpRemoteHostMulti
				RpCatUnsigned32ToString(theRHRequestPtr->fHostIndex, 
						theRHRequestPtr->fRedirectPath, 3);
				thePrefixLength += 3;
				*(theRHRequestPtr->fRedirectPath + thePrefixLength) = '\0';
#endif
				RpStrLenCpyTruncate(theRHRequestPtr->fRedirectPath + 
						thePrefixLength, theStartOfTokenPtr, 
						kMaxSaveHeaderLength - thePrefixLength);
			}
		}
	}

	return;
}


static void ProcessPragma(rpRHRequestPtr theRHRequestPtr,
							char *theStartOfTokenPtr,
							Unsigned16 theTokenLength) {

	/*
		Make it a C string.
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';

	if (RP_STRCMP(theStartOfTokenPtr, kHttpPatternNoCache) == 0) {
		RP_STRCPY(theRHRequestPtr->fDateStringPtr, kHttpNoCache);
		theRHRequestPtr->fDateStringPtr += sizeof(kHttpNoCache) - 1;
	}

	return;
}


#if RomPagerSecurity
static void ProcessAuthenticate(rpRHRequestPtr theRHRequestPtr,
								char *theStartOfTokenPtr,
								Unsigned16 theTokenLength) {

	/*
		See if the (case-insensitive) token is "basic".
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	RpConvertTokenToLowerCase(theStartOfTokenPtr, theTokenLength);
	if (RP_STRCMP(theStartOfTokenPtr, kHttpPatternBasic) == 0) {
		theRHRequestPtr->fHttpResponseState = eRpHttpNeedBasicAuthorization;
		/*
			We need to get the Realm name string.  This
			should be a quoted string.
		*/
		theStartOfTokenPtr += theTokenLength + 1;
		theTokenLength = RpFindTokenDelimited(theStartOfTokenPtr, kAscii_Quote);
		theStartOfTokenPtr += theTokenLength + 1;
		theTokenLength = RpFindTokenDelimited(theStartOfTokenPtr, kAscii_Quote);
		*(theStartOfTokenPtr + theTokenLength) = '\0';
		RP_STRCPY(theRHRequestPtr->fRealm.fRealmName, theStartOfTokenPtr);
	}
	else {
		/*
			The Remote Host feature only supports Basic Authentication.
			Since this is not a Basic Authentication request, just
			set up the states to return "404 Not Found".
		*/
		theRHRequestPtr->fHttpResponseState = eRpHttpNoObjectFound;
	}

	return;
}
#endif


static void InitHttpProxyPatternTable(rpRHPatternTablePtr thePatternTablePtr) {

	thePatternTablePtr->fPattern       = kHttpVersion;
	thePatternTablePtr->fPatternLength = 7;
	thePatternTablePtr->fAction        = ProcessHttpVersion;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternContentType;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternContentType) - 1;
	thePatternTablePtr->fAction        = ProcessContentType;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternDate;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternDate) - 1;
	thePatternTablePtr->fAction        = ProcessDate;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternExpires;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternExpires) - 1;
	thePatternTablePtr->fAction        = ProcessExpires;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternLastModified;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternLastModified) - 1;
	thePatternTablePtr->fAction        = ProcessLastModified;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternPragma;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternPragma) - 1;
	thePatternTablePtr->fAction        = ProcessPragma;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternLocation;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternLocation) - 1;
	thePatternTablePtr->fAction        = ProcessLocation;

#if RomPagerSecurity
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternAuthenticate;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternAuthenticate) - 1;
	thePatternTablePtr->fAction        = ProcessAuthenticate;
#endif

	thePatternTablePtr += 1;
	thePatternTablePtr->fPatternLength = 0;

	return;
}

// +++ _Alphanetworks_Patch_, 11/04/2003, jacob_shih
/*
	alphaRpSetRemoteHostIpAddress
*/

#if RpRemoteHostMulti
void alphaRpSetRemoteHostIpAddress(
									Unsigned32 theIpAddress,
									Unsigned8 theHostIndex) {
#if defined(WIN32)
	RpSetRemoteHostIpAddress(&gRpData, theIpAddress, theHostIndex);
#else
	int i;
	for(i=0; i<kNumberOfTaskData; i++)
	{
		RpSetRemoteHostIpAddress(&gRpData[i], theIpAddress, theHostIndex);
	}
#endif
	return;
}
#else
void alphaRpSetRemoteHostIpAddress(
								Unsigned32 theIpAddress) {
#if defined(WIN32)
	RpSetRemoteHostIpAddress(&gRpData, theIpAddress);
#else
	int i;
	for(i=0; i<kNumberOfTaskData; i++)
	{
		RpSetRemoteHostIpAddress(&gRpData[i], theIpAddress);
	}
#endif
	return;
}
#endif


/*
	alphaRpSetRemoteHostAuthInfo
*/

#if RpRhLocalCredentials
#if RpRemoteHostMulti
void alphaRpSetRemoteHostAuthInfo(
								char *theUsernamePtr,
								char *thePasswordPtr,
								Unsigned8 theHostIndex) {
#if defined(WIN32)
	RpSetRemoteHostAuthInfo(&gRpData, theUsernamePtr, thePasswordPtr, theHostIndex);
#else
	int i;
	for(i=0; i<kNumberOfTaskData; i++)
	{
		RpSetRemoteHostAuthInfo(&gRpData[i], theUsernamePtr, thePasswordPtr, theHostIndex);
	}
#endif
	return;
}

#else	/* !RpRemoteHostMulti */

void alphaRpSetRemoteHostAuthInfo(
								char *theUsernamePtr,
								char *thePasswordPtr) {
#if defined(WIN32)
	RpSetRemoteHostAuthInfo(&gRpData, theUsernamePtr, thePasswordPtr);
#else
	int i;
	for(i=0; i<kNumberOfTaskData; i++)
	{
		RpSetRemoteHostAuthInfo(&gRpData[i], theUsernamePtr, thePasswordPtr);
	}
#endif
	return;
}

#endif
#endif	/* RpRhLocalCredentials */
#if RpRemoteHostMulti
	#if defined(WIN32)
	Boolean alphaRpIsRemoteHostValid(void* theTaskDataPtr, Unsigned16 theHostIndex)
	{
		return TRUE;
	}
	#else
	Boolean alphaRpIsRemoteHostValid(void* theTaskDataPtr, Unsigned16 theHostIndex)
	{
        return TRUE;
	}
	#endif	//	defined(WIN32)
#else	//	RpRemoteHostMulti
	#if defined(WIN32)
	Boolean alphaRpIsRemoteHostValid(void* theTaskDataPtr)
	{
		return TRUE;
	}
	#else
	Boolean alphaRpIsRemoteHostValid(void* theTaskDataPtr)
	{
		return TRUE;
	}
	#endif	//	defined(WIN32)
#endif	//	RpRemoteHostMulti

// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih

#endif	/* RomPagerRemoteHost */
