/*
 *  File:       Sudp.h
 *
 *  Contains:   Prototypes for simple interface routines to UDP/IP
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Copyright:  © 1995-2002 by Allegro Software Development Corporation
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
 * * * * Release 4.00  * * *
 *      06/29/01    amp     rework kSudpNumberOfConnections for RomUpnpControl
 *      05/02/01    dts     rework kSudpNumberOfConnections assignment
 *      08/17/00    amp     rework SudpReceive interface
 *      08/09/00    bva     rework call interfaces
 *      08/08/00    amp     SudpTimeToLive is an int
 *      07/30/00    bva     add RomUpnp support
 * * * * Release 3.0 * * * *
 *      02/26/99    bva     change connection setup definitions
 *      02/15/99    bva     rework
 *      01/24/99    pjr     fix SudpReceiveStatus and SudpSend prototypes
 *      09/02/98    dts     created
 *
 *  To Do:
 */

#include "AsExtern.h"
#include "AsError.h"


#ifndef _SUDP_
#define _SUDP_

typedef unsigned short  SudpLength;
typedef unsigned long   SudpIpAddress;
typedef unsigned short  SudpPort;
typedef unsigned char   SudpConnection;
typedef unsigned short  SudpTimeToLive;

#define kSudpReceiveBufferSize      (1500)  /* size of receive buffer */

#if RomDns
#define kSudpNumberOfDnsConnections     (4)
#else
#define kSudpNumberOfDnsConnections     (0)
#endif

#if RomUpnp
#define kSudpNumberOfUpnpConnections    (2)
#else
#define kSudpNumberOfUpnpConnections    (0)
#endif

#if RomUpnpControl
#define kSudpNumberOfCpUpnpConnections  (2)
#else
#define kSudpNumberOfCpUpnpConnections  (0)
#endif

#if RomTime
#define kSudpNumberOfTimeConnections    (2)
#else
#define kSudpNumberOfTimeConnections    (0)
#endif

#define kSudpNumberOfConnections    (kSudpNumberOfDnsConnections +      \
                                        kSudpNumberOfUpnpConnections +  \
                                        kSudpNumberOfTimeConnections +  \
                                        kSudpNumberOfCpUpnpConnections)

#define kDnsPort    53
#define kNtpPort    123

/*
    Simple Udp call completion states
*/

typedef enum {
    eSudpPending,
    eSudpComplete
} SudpStatus;

extern RpErrorCode SudpInit(void);
extern RpErrorCode SudpDeInit(void);
extern RpErrorCode SudpOpenConnection(SudpConnection theConnection,
                                    SudpIpAddress theRemoteAddress,
                                    SudpPort theRemotePort,
                                    SudpIpAddress *theLocalAddressPtr,
                                    SudpPort *theLocalPortPtr);
extern RpErrorCode SudpOpenMulticastReceiver(SudpConnection theConnection,
                                    SudpIpAddress theMulticastAddress,
                                    SudpPort theMulticastPort,
                                    SudpIpAddress *theLocalAddressPtr,
                                    SudpPort *theLocalPortPtr);
extern RpErrorCode SudpOpenMulticastSender(SudpConnection theConnection,
                                    SudpIpAddress theMulticastAddress,
                                    SudpPort theMulticastPort,
                                    SudpTimeToLive theTimeToLive,
                                    SudpIpAddress *theLocalAddressPtr,
                                    SudpPort *theLocalPortPtr);
extern RpErrorCode SudpCloseConnection(SudpConnection theConnection);
extern RpErrorCode SudpReceive(SudpConnection theConnection,
                                    SudpStatus *theCompletionStatusPtr,
                                    char *theReceivePtr,
                                    SudpLength *theReceiveLengthPtr,
                                    SudpIpAddress *theRemoteAddressPtr,
                                    SudpPort *theRemotePortPtr);
extern RpErrorCode SudpSend(SudpConnection theConnection,
                                    char *theSendPtr,
                                    SudpLength theSendLength);

#endif
