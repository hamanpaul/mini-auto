/*
 *  File:       Sudp.c for Win95
 *
 *  Contains:   Simple UDP interface routines
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Copyright:  © 1995-2001 by Allegro Software Development Corporation
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
 *      07/28/00    amp     added multicast
 *      11/29/99    bva     fixed include
 *      04/07/99    pjr     created Win95 version from Linux version
 *      02/15/99    bva     cleanup
 *      06/14/98    dts     created
 *
 *  To Do:
 */

#include <Winsock.h>
#include <stdio.h>
#include "AsExtern.h"
#include "Sudp.h"

typedef unsigned char   Boolean;
typedef int             STATUS;

#define SUDP_DEBUG  1

/*
 *  The Sudp routines are the interface between Allegro Software
 *  products and the device UDP routines.  The interface allows for
 *  multiple simultaneous associations.  As UDP is connectionless,
 *  traffic to or from multiple remote systems may be presented on
 *  a single endpoint.
 *
 *  The SudpInit and SudpDeInit routines operate on all "connections."
 *  They allocate/free memory for all connections and set up any
 *  necessary states for the entire interface.
 */

/*
 *  structure for BSD/Winsock style TCP/IP sockets
 *
 *  There is a tradeoff between generality and resource use here:
 *  RomPager opens a single listener port, and accepts multiple connections
 *  on the port.  We take advantage of this, and create a single 'master'
 *  server socket, which we use to retrieve client connections.
 */

typedef struct {
    Signed32            fClientSocket;
    void *              fReceiveBuffer;
    struct sockaddr_in  fSockAddr;
    Boolean             fMulticast;
} SudpConnectInfo, *SudpConnectInfoPtr;

static SudpConnectInfo *    gUdpConnectInfo = NULL;

static SudpIpAddress    gHostAddress = 0;
/*
 *  The SudpInit routine is called once at startup, so the UDP
 *  interface can initialize any internal variables and processes.
 *
 *  Returns:
 *      theResult:
 *          eRpNoError          - no error
 *          eRpOutOfMemory      - can't allocate memory
 *          eRpUdpInitError     - can't initialize UDP
 */

RpErrorCode SudpInit(void) {
    int             theIndex;
    RpErrorCode     theResult;
    char            theHostname[100];
    HOSTENT *       theHostEntPtr;

    theResult = eRpNoError;

    /*
        Allocate space for connection info.
    */
    gUdpConnectInfo = (SudpConnectInfoPtr) calloc(sizeof(SudpConnectInfo),
                                                    kSudpNumberOfConnections);

    /*
        Initialize the structure, we have no connections yet.
    */
    if (gUdpConnectInfo) {
        for (theIndex = 0; theIndex < kSudpNumberOfConnections; theIndex++) {
            gUdpConnectInfo[theIndex].fClientSocket = SOCKET_ERROR;
        }
    }
    else {
        theResult = eRpUdpInitError;
    }

    /*
        get the current address in host order
    */
    gethostname( theHostname, sizeof(theHostname));
    theHostEntPtr = gethostbyname(theHostname);
    if (theHostEntPtr->h_addr_list[0]) {
        RP_MEMCPY(&gHostAddress,theHostEntPtr->h_addr_list[0],
                sizeof(gHostAddress));
    }

    return theResult;
}

/*
    Return a pointer to the ip address
*/
SudpIpAddress *GetHostAddress(void) {
    return (SudpIpAddress *)&gHostAddress;
}


/*
 *  The SudpDeInit routine is called once when finished so that the
 *  UDP interface can deinitialize any internal variables and processes.
 *
 *  Returns:
 *      theResult:
 *          eRpNoError          - no error
 *          eRpUdpDeInitError   - can't deinitialize UDP
 */

RpErrorCode SudpDeInit(void) {
    int     theIndex;

    if (gUdpConnectInfo) {
        for (theIndex = 0; theIndex < kSudpNumberOfConnections; theIndex++) {
            if (gUdpConnectInfo[theIndex].fClientSocket >= 0) {
                closesocket(gUdpConnectInfo[theIndex].fClientSocket);
            }
        }
        free(gUdpConnectInfo);
    }

    return eRpNoError;
}


/*
 *  The SudpOpenConnection routine is called to setup a UDP connection
 *  to the specified Server IP Address and port number.  The call is
 *  synchronous and really just sets up an association as there is no
 *  real connection set up for UDP.
 *
 *  Inputs:
 *      theConnection:              - connection id
 *      theRemoteAddress:           - destination server
 *      theRemotePort:              - destination port
 *
 *  Returns:
 *      theResult:
 *          eRpNoError              - no error
 *          eRpUdpAlreadyOpen       - connection is already open
 *          eRpUdpCannotOpenActive  - can't open an active connection
 */
RpErrorCode SudpOpenConnection(SudpConnection theConnection,
                                SudpIpAddress theRemoteAddress,
                                SudpPort theRemotePort,
                                SudpIpAddress *theLocalAddressPtr,
                                SudpPort *theLocalPortPtr) {
    SudpConnectInfoPtr  theConnectionPtr;
    RpErrorCode         theResult;
    struct sockaddr_in  theLocalAddr;
    int                 theLocalAddrLength;
    int                 theStatus;
    int                 one;

    one = 1;
    theConnectionPtr = &gUdpConnectInfo[theConnection];
    theResult = eRpNoError;
    theConnectionPtr->fClientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    theConnectionPtr->fMulticast = False;

    if (theConnectionPtr->fClientSocket == SOCKET_ERROR) {
        theResult = eRpUdpCannotOpenActive;
    }
    else {
#if 0
    /*
                Allow other processes to listen on this port
        */
        if (setsockopt(theConnectionPtr->fClientSocket,
                SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)) == SOCKET_ERROR) {
            theResult = eRpUdpCannotOpenActive;

#if SUDP_DEBUG
            printf("Could not reuse address -setsockopt error: %d\n",
                WSAGetLastError ());
#endif
        }
#endif
        /*
            Bind a receive port to the socket
        */
        memset(&theLocalAddr, 0, sizeof(theLocalAddr));
        theLocalAddr.sin_family = AF_INET;
        theLocalAddr.sin_port = htons(INADDR_ANY /* was theRemotePort*/);
        theLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        theStatus = bind (theConnectionPtr->fClientSocket,
            (struct sockaddr FAR *) &theLocalAddr, sizeof (theLocalAddr));
        if (theStatus == SOCKET_ERROR) {
#if SUDP_DEBUG
            printf ("Binding socket failed! Error: %d\n", WSAGetLastError ());
#endif
            theResult = eRpUdpCannotOpenActive;
        }
        else {
            /*
                Save the remote connection information
            */
            memset(&theConnectionPtr->fSockAddr, 0, sizeof(struct sockaddr));
            theConnectionPtr->fSockAddr.sin_family = AF_INET;
            theConnectionPtr->fSockAddr.sin_addr.s_addr = theRemoteAddress;
            theConnectionPtr->fSockAddr.sin_port = htons(theRemotePort);

            /*
                Get the local ip address and port info
            */
            theLocalAddrLength = sizeof(theLocalAddr);
            theStatus = getsockname(theConnectionPtr->fClientSocket,
                    (struct sockaddr *) &theLocalAddr,
                    &theLocalAddrLength);
            if (theStatus != SOCKET_ERROR) {
                *theLocalAddressPtr = gHostAddress; /*theLocalAddr.sin_addr.s_addr;*/
                *theLocalPortPtr = ntohs(theLocalAddr.sin_port);
            }

        }
    }
    return theResult;
}


RpErrorCode SudpOpenMulticastSender(SudpConnection theConnection,
                                SudpIpAddress theMulticastAddress,
                                SudpPort theMulticastPort,
                                SudpTimeToLive theTimeToLive,
                                SudpIpAddress *theLocalAddressPtr,
                                SudpPort *theLocalPortPtr) {
    SudpConnectInfoPtr  theConnectionPtr;
    RpErrorCode         theResult;
    struct sockaddr_in  theLocalAddr;
    int                 theLocalAddrLength;
    int                 theStatus;
    struct ip_mreq      theMulticastGroup;
    int                 theTTL;
    int                 one;

    one = 1;
    theTTL = theTimeToLive;

    theConnectionPtr = &gUdpConnectInfo[theConnection];
    theResult = eRpNoError;
    theConnectionPtr->fClientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    /*
            Allow other processes to listen on this port
    */
    if (setsockopt(theConnectionPtr->fClientSocket,
                SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)) == SOCKET_ERROR) {
            theResult = eRpUdpCannotOpenActive;
#if SUDP_DEBUG
            printf("Could not reuse multicast group -setsockopt error: %d\n",
                WSAGetLastError ());
#endif
        }

    /* Associate the local address with Socket. */
    if (theConnectionPtr->fClientSocket == SOCKET_ERROR) {
        theResult = eRpUdpCannotOpenActive;
    }
    else {
        memset(&theLocalAddr, 0, sizeof(theLocalAddr));
        theLocalAddr.sin_family = AF_INET;
        theLocalAddr.sin_port = htons(theMulticastPort);
        theLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        theStatus = bind (theConnectionPtr->fClientSocket,
                (struct sockaddr FAR *) &theLocalAddr, sizeof (theLocalAddr));
        if (theStatus == SOCKET_ERROR) {
#if SUDP_DEBUG
            printf ("Binding socket failed! Error: %d", WSAGetLastError ());
#endif
            theResult = eRpUdpCannotOpenActive;
        }
        else  if (setsockopt (theConnectionPtr->fClientSocket, IPPROTO_IP,
                    IP_MULTICAST_TTL, (char *)&theTTL, sizeof (theTTL))
                    == SOCKET_ERROR)  {
#if SUDP_DEBUG
            printf ("Could not set Time-To-Live! Error: %d\n", WSAGetLastError ());
#endif
            theResult = eRpUdpCannotOpenActive;
            closesocket (theConnectionPtr->fClientSocket);
        }
        else {
            /* Join the multicast group from which to receive datagrams.*/
            theMulticastGroup.imr_multiaddr.s_addr = theMulticastAddress;
            theMulticastGroup.imr_interface.s_addr = INADDR_ANY;
            if (setsockopt (theConnectionPtr->fClientSocket, IPPROTO_IP,
                    IP_ADD_MEMBERSHIP, (char *)&theMulticastGroup,
                    sizeof (theMulticastGroup)) == SOCKET_ERROR)  {
#if SUDP_DEBUG
                printf ("Could not join multicast group -setsockopt failed! Error: %d\n",
                      WSAGetLastError ());
#endif
                theResult = eRpUdpCannotOpenActive;
                closesocket(theConnectionPtr->fClientSocket);
            }
            else {
                theConnectionPtr->fMulticast = True;
            }

            /*
                Save the connection information
            */
            memset(&theConnectionPtr->fSockAddr, 0, sizeof(struct sockaddr));
            theConnectionPtr->fSockAddr.sin_family = AF_INET;
            theConnectionPtr->fSockAddr.sin_addr.s_addr = theMulticastAddress;
            theConnectionPtr->fSockAddr.sin_port = htons(theMulticastPort);
        }
    }

    /*
        Get the local ip address and port info
    */
    theLocalAddrLength = sizeof(theLocalAddr);
    theStatus = getsockname(theConnectionPtr->fClientSocket,
            (struct sockaddr *) &theLocalAddr,
            &theLocalAddrLength);
    if (theStatus != SOCKET_ERROR) {
        *theLocalAddressPtr = gHostAddress; /*theLocalAddr.sin_addr.s_addr;*/
        *theLocalPortPtr = ntohs(theLocalAddr.sin_port);
    }

    return theResult;
}



/*
 *  The SudpSend routine is called to send a buffer of information over
 *  the connection.  The call is an asynchronous call and completes when
 *  the send has been started.
 *
 *  Inputs:
 *      theConnection:          - connection id
 *      theSendPtr:             - pointer to buffer to be sent
 *      theSendLength:          - length of characters to be sent
 *
 *  Returns:
 *      theResult:
 *          eRpNoError          - no error
 *          eRpUdpSendError     - can't send the buffer
 *          eRpUdpNoConnection  - there isn't a currently opened connection
 */

RpErrorCode SudpSend(SudpConnection theConnection,
                        char *theSendPtr,
                        SudpLength theSendLength) {
    SudpConnectInfoPtr  theConnectionPtr;
    RpErrorCode         theResult;
    int                 theStatus;

    theConnectionPtr = &gUdpConnectInfo[theConnection];
    theResult = eRpNoError;

    /*
        We need a valid socket!
    */
    if (theConnectionPtr->fClientSocket == SOCKET_ERROR) {
        theResult = eRpUdpNoConnection;
    }
    else {
        theStatus = sendto(theConnectionPtr->fClientSocket,
                            theSendPtr, theSendLength, 0,
                            (struct sockaddr *)&theConnectionPtr->fSockAddr,
                            sizeof(theConnectionPtr->fSockAddr));

        if (theStatus == SOCKET_ERROR) {
#if SUDP_DEBUG
            printf ("send failed! Error: %d", WSAGetLastError ());
#endif
            theResult = eRpUdpSendError;
        }
    }

    return theResult;
}

RpErrorCode SudpOpenMulticastReceiver(SudpConnection theConnection,
                                SudpIpAddress theMulticastAddress,
                                SudpPort theMulticastPort,
                                SudpIpAddress *theLocalAddressPtr,
                                SudpPort *theLocalPortPtr) {
    SudpConnectInfoPtr  theConnectionPtr;
    RpErrorCode         theResult;
    struct sockaddr_in  theLocalAddr;
    int                 theLocalAddrLength;
    int                 theStatus;
    struct ip_mreq      theMulticastGroup;
    int                 one;

    one = 1;

    theConnectionPtr = &gUdpConnectInfo[theConnection];
    theResult = eRpNoError;
    memset(&theLocalAddr, 0, sizeof(theLocalAddr));
    theLocalAddr.sin_family = AF_INET;
    theLocalAddr.sin_port = htons(theMulticastPort);
    theLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    theConnectionPtr->fClientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    /*
        Allow other processes to listen on this port
    */
    if (setsockopt(theConnectionPtr->fClientSocket,
            SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)) == SOCKET_ERROR) {
#if SUDP_DEBUG
            printf("Could not reuse multicast group -setsockopt error: %d\n",
                    WSAGetLastError ());
#endif
    }

    /* Associate the local address with Socket. */
    if (theConnectionPtr->fClientSocket == SOCKET_ERROR) {
        theResult = eRpUdpCannotOpenActive;
    }
    else {
        theStatus = bind (theConnectionPtr->fClientSocket,
            (struct sockaddr FAR *) &theLocalAddr,
            sizeof (theLocalAddr));
        if (theStatus == SOCKET_ERROR) {
#if SUDP_DEBUG
            printf ("Binding socket failed! Error: %d\n", WSAGetLastError ());
#endif
            theResult = eRpUdpCannotOpenActive;
        }
    }

    /* Join the multicast group from which to receive datagrams.*/
    theMulticastGroup.imr_multiaddr.s_addr = theMulticastAddress;
    theMulticastGroup.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt (theConnectionPtr->fClientSocket, IPPROTO_IP,
            IP_ADD_MEMBERSHIP, (char *)&theMulticastGroup,
            sizeof (theMulticastGroup)) == SOCKET_ERROR)  {
#if SUDP_DEBUG
        printf ("Could not join multicast group -setsockopt failed! Error: %d\n",
              WSAGetLastError ());
#endif
        closesocket(theConnectionPtr->fClientSocket);
    }

    /*
        Get the local ip address and port info
    */
    theLocalAddrLength = sizeof(theLocalAddr);
    theStatus = getsockname(theConnectionPtr->fClientSocket,
            (struct sockaddr *) &theLocalAddr,
            &theLocalAddrLength);
    if (theStatus != SOCKET_ERROR) {
        *theLocalAddressPtr = gHostAddress; /*theLocalAddr.sin_addr.s_addr;*/
        *theLocalPortPtr = ntohs(theLocalAddr.sin_port);
    }

    return theResult;
}



/*
 *  The SudpReceive routine is called to determine whether a buffer has
 *  been received on the connection.  The call is non-blocking, in that
 *  it should check to see if a buffer has been received and return
 *  immediately whether or not a buffer has been received.
 *
 *  Inputs:
 *      theConnection:          - connection id
 *      theCompletionStatusPtr: - pointer to the receive status
 *      theReceivePtrPtr:       - pointer to a buffer pointer
 *      theReceiveLengthPtr:    - pointer to the received length
 *
 *  Returns:
 *      theCompletionStatusPtr: - eStcpComplete, if a buffer has been received
 *                              - eStcpPending, if no buffer has been received
 *      theReceivePtrPtr:       - a pointer to the buffer contents
 *      theReceiveLengthPtr:    - the length of the received buffer
 *      theResult:
 *          eRpNoError          - no error
 *          eTpTcpReceiveError  - can't receive a buffer (an I/O error occurred)
 */

RpErrorCode SudpReceive(SudpConnection theConnection,
                        SudpStatus *theCompletionStatusPtr,
                        char *theReceivePtr,
                        SudpLength *theReceiveLengthPtr,
                        SudpIpAddress *theRemoteAddressPtr,
                        SudpPort *theRemotePortPtr) {
    int                 theBytesAvail;
    SudpConnectInfoPtr  theConnectionPtr;
    int                 theLength;
    RpErrorCode         theResult;
    STATUS              theStatus;
    struct sockaddr_in  theRemoteAddr;
    int                 theRemoteAddrLength;

    theConnectionPtr = &gUdpConnectInfo[theConnection];
    theResult = eRpNoError;

    /*
        We need a valid socket!
    */
    if (theConnectionPtr->fClientSocket == SOCKET_ERROR) {
        theResult = eRpUdpReceiveError;
    }
    else {
        /*
            Are there bytes waiting?
        */
        theBytesAvail = 0;
        theStatus = ioctlsocket(theConnectionPtr->fClientSocket, FIONREAD,
                                (u_long FAR *) &theBytesAvail);
        /*
            Verify that the incoming packet is received correctly and
            fits in the buffer allocated in SudpReceive.
        */
        if (theStatus == SOCKET_ERROR) {
            theResult = eRpUdpReceiveError;
        }
        else {
            if (theBytesAvail) {
                if (theBytesAvail > kSudpReceiveBufferSize) {
                    theBytesAvail = kSudpReceiveBufferSize;
                }

                theRemoteAddrLength = sizeof(theRemoteAddr);
                theLength = recvfrom(theConnectionPtr->fClientSocket,
                                    theReceivePtr, theBytesAvail, 0,
                                    (struct sockaddr FAR *)&theRemoteAddr,
                                    &theRemoteAddrLength);

                if (theLength == SOCKET_ERROR || theLength > theBytesAvail) {
                    theResult = eRpUdpReceiveError;
                }
                else {
                    *theRemoteAddressPtr = theRemoteAddr.sin_addr.s_addr;
                    *theRemotePortPtr = ntohs(theRemoteAddr.sin_port);
                    *theCompletionStatusPtr = eSudpComplete;
                    *theReceiveLengthPtr = theLength;
                }
            }
            else {
                *theCompletionStatusPtr = eSudpPending;
            }
        }
    }

    return theResult;
}


/*
 *  The SudpCloseConnection routine is called to gracefully close
 *  an active connection.  The call is synchronous.
 *
 *  Inputs:
 *      theConnection:          - connection id
 *
 *  Returns:
 *      theResult:
 *          eRpNoError          - no error
 *          eRpUdpAlreadyClosed - the other side already closed the connection
 *          eRpUdpCloseError    - can't close the connection
 */

RpErrorCode SudpCloseConnection(SudpConnection theConnection) {
    SudpConnectInfoPtr  theConnectionPtr;
    RpErrorCode         theResult;
    struct ip_mreq theMulticastGroup;

    theConnectionPtr = &gUdpConnectInfo[theConnection];
    theResult = eRpNoError;

    if (theConnectionPtr->fClientSocket == SOCKET_ERROR) {
        theResult = eRpUdpAlreadyClosed;
    }
    else {
        if (theConnectionPtr->fMulticast) {
            /*
                Drop out of multicast group
            */
            theMulticastGroup.imr_multiaddr.s_addr =
                    theConnectionPtr->fSockAddr.sin_addr.s_addr;
            theMulticastGroup.imr_interface.s_addr = INADDR_ANY;
            if (setsockopt (theConnectionPtr->fClientSocket, IPPROTO_IP,
                    IP_DROP_MEMBERSHIP, (char *)&theMulticastGroup,
                    sizeof (theMulticastGroup)) == SOCKET_ERROR)  {
#if SUDP_DEBUG
                printf("Could not drop from multicast group\n");
#endif
            }
        }

        if (closesocket(theConnectionPtr->fClientSocket) == SOCKET_ERROR) {
                theResult = eRpUdpCloseError;
        }
    }

    /*
        Mark the socket as closed.
    */
    gUdpConnectInfo[theConnection].fClientSocket = SOCKET_ERROR;

    return theResult;
}
