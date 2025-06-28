/*
 *  File:       Stcp4.c for Phase2
 *
 *  Contains:   Simple TCP interface routines
 *
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Copyright:  ¨ 1995-2001 by Allegro Software Development Corporation
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
 *  Change History (most recent first):
 *
 *      11/01/01    bva     remove dead code,
 *      				    add FIONBIO include
 * * * * Release 4.03  * * *
 * * * * Release 4.00  * * *
 *      10/21/00    dts     add multiple listener support
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 *      11/06/99    bva     fix SendBuffer for partial writes
 *      09/08/98    bva     add listener close to StcpDeInit
 *      08/21/98    dts     Linux port
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 *      11/26/97    bva     misc cleanup for RomPager 2.0
 *      07/14/97    bva     change StcpReceiveStatus to deal with rcv overflow
 * * * * Release 1.6 * * * *
 *      03/26/97    bva     change error codes
 * * * * Release 1.5 * * * *
 *      03/10/97    bva     incorporate customer fixes to StcpConnectionStatus,
 *                          StcpReceiveStatus, StcpAbortConnection
 * * * * Release 1.4 * * * *
 * * * * Release 1.3 * * * *
 *      07/05/96    bva     add StcpStatus for call completion to support
 *                          TCP TIME_WAIT processing
 * * * * Release 1.2 * * * *
 * * * * Release 1.0 * * * *
 *      01/17/96    bva     created
 *
 *  To Do:
 */
#include    "general.h"
#include	"RomPager.h"
#include	"RpExtern.h"
#include	"AsTypes.h"
#include	"Stcp.h"
#include	"lwipapi.h"

#define		SOCKET_ERROR		(-1)
#define     EWOULDBLOCK         (-12)
#define		kStcpMaxBacklog		(5)

#define  WEB_RECV_MIN_SLEEP 4 /*0.1 second*/
#define  WEB_RECV_MAX_LOOP  50

#include 	<stdio.h>
#include 	<assert.h> /* for catostrophic failures */

//#include 	"errno.h"

//typedef unsigned char Boolean;
//typedef int STATUS;

//#include "AsTypes.h"
//#include "Stcp.h"

/*
 *  The Stcp routines are the interface between RomPager and the device
 *  TCP routines.  The interface assumes that multiple connections to
 *  TCP can be maintained to support overlapping requests.  This is
 *  especially necessary to support the Netscape browser, as it fires
 *  off multiple requests to speed up graphics display.
 *
 *  The constant kStcpNumberOfConnections in Stcp.h defines how many
 *  connections are to be supported.
 *
 *  The StcpInit and StcpDeInit routines operate on all connections.
 *  They allocate/free memory for all connections and set up any
 *  necessary states for the entire interface.
 *
 *  The other routines all are passed a connection id which will range
 *  from 0 to (kStcpNumberOfConnections - 1).  The easiest way to use
 *  the connection id is as an index to an array of per connection data.
 *
 */

typedef enum {
    eSendPending,
    eSendStarted,
    eSendError
} SendStatus;

/*
 * structure for BSD/Winsock style TCP/IP sockets
 *
 * there is a tradeoff between generality and resource use here:
 * RomPager opens a single listener port, and accepts multiple connections
 * on the port.  We take advantage of this, and creating a single
 * 'master' server socket, which we use to retrieve client connections.
 */

typedef struct {
    Signed32    fClientSocket;
    void        *fReceiveBuffer;
    SendStatus  fSendStatus;
    char        *fPendingSendBuffer;
    StcpLength  fPendingSendBufferLen;
} StcpConnectInfo, *StcpConnectInfoPtr;

static StcpConnectInfo  gTcpConnectInfo[kStcpNumberOfConnections];
static Signed32         gTcpServerSocket[kStcpServerCount];
static StcpPort         gTcpListenPorts[kStcpServerCount];
static int              gTcpListenPortCount;
static char             tcp_receive_buffer[kStcpNumberOfConnections][kStcpReceiveBufferSize];

void SendBuffer(StcpConnectInfoPtr theConnectionPtr,
                char *theSendPtr,
                StcpLength theSendLength);

/*
 *  The StcpInit routine is called once when RomPager starts up, so that
 *  the TCP interface can initialize any internal variables and processes.
 *
 *  Returns:
 *      theResult:
 *          eRpNoError          - no error
 *          eRpOutOfMemory      - can't allocate memory
 *          eRpTcpInitError     - can't initialize TCP
 */

RpErrorCode StcpInit (void)
{
    RpErrorCode     theResult;
    int             i;

	/*	Set the default listener to be the HTTP server on port 80.*/
	gTcpListenPortCount = 1;
	gTcpListenPorts[0] = kHttpPort;
    theResult = eRpNoError;

    /*
        Set up signal handling

        For SIGPIPE, just ignore the signals. We'll get what we need from
        the actual I/O calls getting errors.
    */

    for (i = 0; i < kStcpServerCount; i++)
    {
        gTcpServerSocket[i] = SOCKET_ERROR;
    }

    /* init structure - we have no connections yet */
    for (i = 0; i < kStcpNumberOfConnections; ++i)
    {
        gTcpConnectInfo[i].fClientSocket = SOCKET_ERROR;
		gTcpConnectInfo[i].fReceiveBuffer = &tcp_receive_buffer[i][0];
        if (!gTcpConnectInfo[i].fReceiveBuffer)
        {
            return eRpOutOfMemory;
        }
    }
    return theResult;
}

/*
 *  The StcpDeInit routine is called once at when RomPager finishes so that
 *  the TCP interface can deinitialize any internal variables and processes.
 *  Any receive buffers that are still allocated, should be deallocated here.
 *
 *  Returns:
 *      theResult:
 *          eRpNoError          - no error
 *          eRpTcpDeInitError   - can't deinitialize TCP
 */

RpErrorCode StcpDeInit (void) 
{
    int     i;

    for (i = 0; i < kStcpNumberOfConnections; ++i)
    {
        if (gTcpConnectInfo[i].fClientSocket >= 0 )
        {
            if (gTcpConnectInfo[i].fReceiveBuffer)
            {
                gTcpConnectInfo[i].fReceiveBuffer = NULL;
            }
            #if 1
            close(gTcpConnectInfo[i].fClientSocket);
            #else
            s_close(gTcpConnectInfo[i].fClientSocket);
            #endif
        }
    }

    for (i = 0; i < kStcpServerCount; i++)
    {
        if (gTcpServerSocket[i] != SOCKET_ERROR)
        {
            #if 1
            close(gTcpServerSocket[i]);
            #else
            s_close(gTcpServerSocket[i]);
            #endif
            gTcpServerSocket[i] = SOCKET_ERROR;
        }
    }

    return eRpNoError;
}

/*
 *  The StcpOpenPassive routine is called to setup a passive TCP connection
 *  on the supplied port.  The routine returns when a connection has been
 *  set up, not when the connection has been made.  The StcpConnectionStatus
 *  call is used to detect an incoming connection.
 *
 *  Inputs:
 *      theConnection:              - connection id
 *      thePort:                    - value of the local port
 *
 *  Returns:
 *      theResult:
 *          eRpNoError              - no error
 *          eRpTcpCannotOpenPassive - can't open a passive connection
 */
/*Elsa modify for web, no use*/
RpErrorCode StcpOpenPassive(StcpConnection theConnection)
{
    int one = 1, on = 1;
    struct sockaddr_in  serverAddr; /* server's address */
    struct sockaddr_in      clientaddr ;
    int theStatus;
    StcpConnection theListener;
    int                     len;

    // has anyone tried to open connections yet?
	theListener = theConnection % gTcpListenPortCount;//~~ new

	if (gTcpServerSocket[theListener] == SOCKET_ERROR)
	{
           	// Open the socket.
        	// Use ARPA Internet address format and stream sockets.
            gTcpServerSocket[theListener] = socket (AF_INET, SOCK_STREAM, 0);
            if (gTcpServerSocket[theListener] == SOCKET_ERROR) {
                return eRpTcpCannotOpenPassive;
            }

            // Set up our internet address, and bind it so the client
            // can connect. If this process restarts, we want to use
            // the same port
            theStatus = setsockopt(gTcpServerSocket[theListener], SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));
            if (theStatus == SOCKET_ERROR)
            {
            	DEBUG_WEB( "StcpOpenPassive: setsockopt error (set SO_REUSERADDR): %d\n", theStatus );
                (void) close(gTcpServerSocket[theListener]);
                return eRpTcpCannotOpenPassive;
            }


            // Zero out the sock_addr structures.
            // This MUST be done before the socket calls.

            memset(&serverAddr, 0, sizeof(serverAddr));
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port   = gTcpListenPorts[theListener];//~~convert values byie between host and networks.
            if (bind(gTcpServerSocket[theListener], (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            {
            	DEBUG_WEB("StcpOpenPassive: bind failed!!");
                (void) close(gTcpServerSocket[theListener]);
                return eRpTcpCannotOpenPassive;
            }

            //~~ sioctl() is a phase2 function
            //~~ s_ioctl: Send IO control command to socket
			//~~ return: O on success, -1 on error.

			if ( ioctlsocket( gTcpServerSocket[theListener], FIONBIO, &on ) == SOCKET_ERROR )
			{
				DEBUG_WEB("stcp: ioctl failed!! Can't set non-blocking for socket %d\n", gTcpServerSocket);
				(void) close( gTcpServerSocket[theListener] );
				return eRpTcpCannotOpenPassive;
			}

            /* enable connections, with a small allowed backlog */

            if (listen(gTcpServerSocket[theListener], kStcpMaxBacklog) == SOCKET_ERROR) {
                (void) close(gTcpServerSocket[theListener]);
                return eRpTcpCannotOpenPassive;
            }
            len = sizeof(clientaddr);
            accept(gTcpServerSocket[theListener] , (struct sockaddr *) &clientaddr, &len) ;
    }

    return eRpNoError;
}


/*
 *  The StcpConnectionStatus routine is called to determine when the
 *  connection has been made.
 *
 *  Inputs:
 *      theConnection:              - connection id
 *      theCompletionStatusPtr:     - pointer to the connection status
 *      theRemoteAddressPtr:        - pointer to an IP address
 *      theLocalAddressPtr:         - pointer to an IP address
 *
 *  Returns:
 *      theCompletionStatusPtr:     - eStcpComplete, if a connection
 *                                  - eStcpPending, if no connection yet
 *                                  - eStcpOpenPassiveTimeout, if open
 *                                          passive connection has timed out
 *      theRemoteAddressPtr:        - the IP address of the remote connection
 *      theLocalAddressPtr:         - the IP address of the local connection
 *      theResult:
 *          eRpNoError              - no error
 *          eRpTcpCannotOpenPassive - can't open a passive connection (an I/O error occurred)
 */

RpErrorCode StcpConnectionStatus(StcpConnection theConnection,
        StcpStatus *theCompletionStatusPtr,
        StcpIpAddress *theRemoteAddressPtr,
        StcpIpAddress *theLocalAddressPtr,
        StcpPort *theLocalPortPtr, int acceptfd)
{
    /*wait ray to get this data*/
    StcpIpAddress     LocalAddr, peerAddr;
    StcpPort     LocalPort;

    gTcpConnectInfo[theConnection].fClientSocket = acceptfd;
    *theCompletionStatusPtr = eStcpComplete;
    *theRemoteAddressPtr = peerAddr;
    *theLocalAddressPtr = LocalAddr;
    *theLocalPortPtr = LocalPort;
    return eRpNoError;
}

/*
 *  The StcpOpenActive routine is called to setup a TCP connection to the
 *  specified Server IP Address and port number. The call is an asynchronous
 *  call and completes when the open has been started.The StcpActiveConnectionStatus
 *  routine is used to detect when the open has completed.
 *
 *  Inputs:
 *      theConnection:              - connection id
 *      theRemoteAddress:           - destination server
 *      theRemotePort:              - destination port
 *
 *  Returns:
 *      theResult:
 *          eRpNoError              - no error
 *          eRpTcpCannotOpenActive  - can't open an active connection
 */
/*Elsa modify for web, no use*/
RpErrorCode StcpOpenActive(StcpConnection theConnection, StcpIpAddress theRemoteAddress,
        StcpPort theRemotePort)
{
    return eRpNoError;
}



/*
 *  The StcpActiveConnectionStatus routine is called to determine when the
 *  connection has been made.
 *
 *      Inputs:
 *          theConnection:              - connection id
 *          theCompletionStatusPtr:     - pointer to the connection status
 *          theLocalAddressPtr:         - pointer to an IP address
 *
 *      Returns:
 *          theCompletionStatusPtr:     - eStcpComplete, if a connection
 *                                      - eStcpPending, if no connection yet
 *          theLocalAddressPtr:         - the IP address of the local connection
 *          theResult:
 *              eRpNoError              - no error
 *              eRpTcpCannotOpenActive  - can't open an active connection
 */
/*Elsa modify for web, no use*/
RpErrorCode StcpActiveConnectionStatus(StcpConnection theConnection,
        StcpStatus *theCompletionStatusPtr, StcpIpAddress *theLocalAddressPtr)
{
    return eRpNoError;
}


/*
 *  The StcpSend routine is called to send a buffer of information over
 *  the connection.  The call is an asynchronous call and completes when
 *  the send has been started.  The StcpSendStatus call is used to detect
 *  the completion of the send.  The last buffer received with the
 *  StcpReceiveStatus call can be deallocated in this call.
 *
 *  Inputs:
 *      theConnection:          - connection id
 *      theSendPtr:             - pointer to buffer to be sent
 *      theSendLength:          - length of characters to be sent
 *
 *  Returns:
 *      theResult:
 *          eRpNoError          - no error
 *          eRpTcpSendError     - can't send the buffer
 *          eRpTcpNoConnection  - there isn't a currently opened connection
 */
RpErrorCode StcpSend(StcpConnection theConnection, char *theSendPtr,
        StcpLength theSendLength)
{
    StcpConnectInfoPtr  theConnectionPtr = &gTcpConnectInfo[theConnection];

    /* we need a valid socket! */
    if(theConnectionPtr->fClientSocket == SOCKET_ERROR)
    {
        return eRpTcpNoConnection;
    }

    SendBuffer(theConnectionPtr, theSendPtr, theSendLength);
    if (theConnectionPtr->fSendStatus == eSendError)
    {
        return eRpTcpSendError;
    }

    return eRpNoError;
}

void SendBuffer(StcpConnectInfoPtr theConnectionPtr, char *theSendPtr,
        StcpLength theSendLength)
{

    int theStatus;

    theStatus = send(theConnectionPtr->fClientSocket, theSendPtr,
            theSendLength, 0);

    if (theStatus == SOCKET_ERROR)
    {
        theConnectionPtr->fSendStatus = eSendError;
    }
    else if (theStatus != theSendLength)
    {
        if (theStatus > 0 && theStatus < theSendLength)
        {
            /*
                In this case, theStatus is the number of bytes written
                and is less than the full buffer we were trying to
                write, so set the status to pending so we write out
                the rest and adjust the buffer pointers to reflect
                what has been written so far.
            */
            theConnectionPtr->fSendStatus = eSendPending;
            theConnectionPtr->fPendingSendBuffer = theSendPtr + theStatus;
            theConnectionPtr->fPendingSendBufferLen = theSendLength - theStatus;
        }
        else
        {
            /*
                some other kind of wierd error
            */
            theConnectionPtr->fSendStatus = eSendError;
        }
    }
    else
    {
        theConnectionPtr->fSendStatus = eSendStarted;
    }
    return;
}


/*
 *  The StcpSendStatus routine is called to determine whether the
 *  buffer has been sent.  When theCompletionStatusPtr is set to
 *  eStcpComplete, control of the buffer returns to the caller.
 *  This means that all lower layer TCP routines need to have
 *  finished with the buffer.
 *
 *  Inputs:
 *      theConnection:          - connection id
 *      theCompletionStatusPtr: - pointer to the send status
 *
 *  Returns:
 *      theCompletionStatusPtr: - eStcpComplete, the send has completed
 *                              - eStcpPending, the send has not completed yet
 *      theResult:
 *          eRpNoError          - no error
 *          eRpTcpSendError     - can't send the buffer (an I/O error occurred)
 */

RpErrorCode StcpSendStatus(StcpConnection theConnection,
        StcpStatus *theCompletionStatusPtr)
{

    StcpConnectInfoPtr theConnectionPtr = &gTcpConnectInfo[theConnection];


    //    we need a valid socket!
    if (theConnectionPtr->fClientSocket == SOCKET_ERROR)
    {
        return eRpTcpNoConnection;
    }

    if (theConnectionPtr->fSendStatus == eSendStarted)
    {
        /*
            Just report completion, since the buffer has
            been copied to the lower layers.
        */
        *theCompletionStatusPtr = eStcpComplete;
        return eRpNoError;
    }

    //    We must be pended because the buffer was full,
    //    so try the send again.
    SendBuffer(theConnectionPtr,
                theConnectionPtr->fPendingSendBuffer,
                theConnectionPtr->fPendingSendBufferLen);
    if (theConnectionPtr->fSendStatus == eSendError)
    {
        return eRpTcpSendError;
    }
    else
    {
        *theCompletionStatusPtr = eStcpPending;
        return eRpNoError;
    }
}


/*
 *  The StcpReceive routine is called to receive a buffer of information over
 *  the connection.  The call is an asynchronous call and completes when
 *  the receive has been started.  The StcpReceiveStatus call is used to
 *  detect the completion of the receive.  Receive buffer allocation (if not
 *  done in the StcpInit or StcpOpenPassive calls) should be done in this
 *  routine.  The receive buffer can be deallocated when this routine is
 *  called to receive another buffer, or when the StcpSend routine is called.
 *
 *  Inputs:
 *      theConnection:          - connection id
 *
 *  Returns:
 *      theResult:
 *          eRpNoError          - no error
 *          eRpTcpReceiveError  - can't start a receive
 *          eRpTcpNoConnection  - there isn't a currently opened connection
 */
RpErrorCode StcpReceive(StcpConnection theConnection)
{

    StcpConnectInfoPtr theConnPtr = &gTcpConnectInfo[theConnection];

    /* we need a valid socket! */
    if(theConnPtr->fClientSocket == SOCKET_ERROR)
    {
        return eRpTcpNoConnection;
    }

    /*
     * for Unix, the actual byte receiving is done in
     * the StcpReceiveStatus routine.
     */

    return eRpNoError;
}


/*
 *  The StcpReceiveStatus routine is called to determine whether a buffer
 *  has been received on the connection.
 *
 *  Inputs:
 *      theConnection:          - connection id
 *      theCompletionStatusPtr: - pointer to the receive status
 *      theReceivePtrPtr:       - pointer to a buffer pointer
 *      theReceiveLengthPtr:    - pointer to the received length
 *
 *  Returns:
 *      theCompletionStatusPtr: - eStcpComplete, if a buffer has been received
 *                              - eStcpPending, if no buffer has been received yet
 *      theReceivePtrPtr:       - a pointer to the buffer contents
 *      theReceiveLengthPtr:    - the length of the received buffer
 *      theResult:
 *          eRpNoError          - no error
 *          eTpTcpReceiveError  - can't receive a buffer (an I/O error occurred)
 */
RpErrorCode StcpReceiveStatus(StcpConnection theConnection,
								StcpStatus *theCompletionStatusPtr,
        						char **theReceivePtrPtr,
        						StcpLength *theReceiveLengthPtr)
{
    int                 len = 0, n=0 ;
     StcpConnectInfoPtr theConnPtr = &gTcpConnectInfo[theConnection];
    int                 off=0;
    int                 theStatus;
    int                 fd;
    int                 rcv_count = 0;

#if 1   /*wait ray receive*/
    struct timeval          tout;
    fd_set                  rs;

	//xprintf("StcpReceiveStatus - theConnection = %d\n", theConnection);
    /*
     * we need a valid socket!
     */
    fd=theConnPtr->fClientSocket;
    if (fd == SOCKET_ERROR)
    {
        return eRpTcpReceiveError;
    }
    /*
     * fd is noblocking, so we need sleep some time wait for connection.
     */

    tout.tv_sec=1;
    tout.tv_usec=0;


    FD_ZERO(&rs);
    FD_SET(fd,&rs);

    n = select(fd+1, &rs, NULL, NULL, &tout);
	//xprintf("socket = %d / n=%d\n", fd, n);
    while(n> 0)
    {
        len = recv(fd,theConnPtr->fReceiveBuffer, kStcpReceiveBufferSize, 0);
        if(len == EWOULDBLOCK && rcv_count < WEB_RECV_MAX_LOOP)
        {
            OSTimeDly(WEB_RECV_MIN_SLEEP);
            rcv_count++;
        }
        else
		{
            break;
		}

    }
#endif

    if (len <= 0)
    {
        #if 1   /*wait ray colse socket*/
        close(fd);
        #else
        s_close(fd);
        #endif
        return eRpTcpReceiveError;
    }
    else
    {
        /*
         *  We need to set fd blocking to reduse resource
         *  after receive HTTP request.
         */
        *theCompletionStatusPtr = eStcpComplete;
        *theReceivePtrPtr = theConnPtr->fReceiveBuffer;
        *theReceiveLengthPtr = (unsigned short)len;

    }

    return eRpNoError;
}


/*
 *  The StcpCloseConnection routine is called to gracefully close
 *  an active connection.  The call is asynchronous and completes
 *  when the closing process has been started.  The StcpCloseStatus
 *  is used to detect when the close has completed.
 *
 *  Inputs:
 *      theConnection:          - connection id
 *
 *  Returns:
 *      theResult:
 *          eRpNoError          - no error
 *          eRpTcpAlreadyClosed - the other side already closed the connection
 *          eRpTcpCloseError    - can't close the connection
 */
RpErrorCode StcpCloseConnection(StcpConnection theConnection)
{

    RpErrorCode theResult = eRpNoError;
    StcpConnectInfoPtr theConnPtr = &gTcpConnectInfo[theConnection];

    if (theConnPtr->fClientSocket == SOCKET_ERROR)
    {
        return eRpTcpAlreadyClosed;
    }

    #if 1
    if (close(theConnPtr->fClientSocket) == SOCKET_ERROR)
    #else
    if (s_close(theConnPtr->fClientSocket) == SOCKET_ERROR)
    #endif
    {
        theResult = eRpTcpCloseError;
    }

    // Mark the socket closed.
    gTcpConnectInfo[theConnection].fClientSocket = SOCKET_ERROR;
    return theResult;
}


/*
 *  The StcpCloseStatus routine is called to determine whether the
 *  connection has been closed.
 *
 *  Inputs:
 *      theConnection:          - connection id
 *      theCompletionStatusPtr: - pointer to the connection status
 *
 *
 *  Returns:
 *      theCompletionStatusPtr: - eStcpComplete, if the close has completed
 *                              - eStcpPending, if the close has not completed yet
 *                              - eStcpTimeWait, if the close has gone to TIME_WAIT
 *      theResult:
 *          eRpNoError          - no error
 *          eRpTcpCloseError    - can't close the connection
*/
RpErrorCode StcpCloseStatus(StcpConnection theConnection, StcpStatus *theCompletionStatusPtr)
{

    /*
        assume closes always successfully complete immediately
    */
    *theCompletionStatusPtr = eStcpComplete;
    return eRpNoError;
}


/*
 *  The StcpAbortConnection routine is called to forcefully close
 *  an active connection.  The call is synchronous and completes
 *  when the closing process has been completed.
 *
 *  Inputs:
 *      theConnection:          - connection id
 *
 *  Returns:
 *      theResult:
 *          eRpNoError          - no error
 *          eRpTcpAbortError    - can't close the connection
 */
RpErrorCode StcpAbortConnection(StcpConnection theConnection)
{

    /*
     *  go close and set our structure to take another connection
     */

    (void) StcpCloseConnection(theConnection);
    return eRpNoError;
}



/*
    The StcpSetListeners routine is called once when the RomPager engine
    starts up, to tell the TCP interface what ports to establish listeners for.


        Inputs:
            theListenerCount:           - number of ports in the array
            theListenerArrayPtr:        - pointer to an array of TCP ports

        Returns:
            theResult:
                eRpNoError              - no error
                eRpOutOfMemory          - can't allocate memory
                eRpTcpInternalError     - theListenerCount > kStcpListeners
*/


RpErrorCode StcpSetServerPortNumbers(int theListenerCount,
                                StcpPort *theListenerArrayPtr)
{
    RpErrorCode theResult;

    int i;

    gTcpListenPortCount = theListenerCount;

    for (i=0; i < theListenerCount; i++)
    {
        gTcpListenPorts[i] = theListenerArrayPtr[i];
    }

    theResult = eRpNoError;

    return theResult;
}

