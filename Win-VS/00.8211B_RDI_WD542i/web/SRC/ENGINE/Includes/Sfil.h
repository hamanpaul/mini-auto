/*
 *  File:       Sfil.h
 *
 *  Contains:   Prototypes for simple interface routines to file system
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
 * * * * Release 3.0 * * * *
 * * * * Release 2.2 * * * *
 *      08/13/98    bva     add theCompleteFlag to SfsCloseFile call
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.6 * * * *
 *      03/26/97    bva     move error codes to RpError.h
 *      02/15/97    bva     updated documentation
 *      01/24/97    rhb     moved MIME types to RpMimes.h
 *      10/15/96    rhb     revised
 *      10/14/96    bva     created
 * * * * Release 1.5 * * * *
 * * * * Release 1.4 * * * *
 * * * * Release 1.3 * * * *
 * * * * Release 1.2 * * * *
 * * * * Release 1.1 * * * *
 * * * * Release 1.0 * * * *
 *
 *  To Do:
 */

#include "AsError.h"
#include "AsMimes.h"

#ifndef _SFIL_
#define _SFIL_

typedef unsigned long   SfsLength;
typedef unsigned long   SfsDate;
typedef unsigned char   SfsFileNumber;
typedef unsigned char   SfsRealm;

/*
    Simple Disk call completion states
*/

typedef enum {
    eSfsPending,
    eSfsComplete,
    eSfsEndOfFile
} SfsStatus;

/*
    SfsFileInfo structure
*/
typedef struct {
    SfsLength       fFileSize;          /*  the size of the file in bytes;  */
    SfsDate         fFileDate;          /*  the date of the file in seconds
                                            since January 1, 1900.  */
    rpDataType      fFileType;          /*  the mime type as
                                            defined in RpMimes.h
                                        */
    SfsRealm        fFileAccess;        /*  the 8-bit security code
                                            (equal to rpAccess in RpPages.h)
                                        */
    char            fOtherMimeType[kMaxMimeTypeLength];
                                        /*  if fFileType is eRpDataTypeOther,
                                            then the real mime type string is
                                            stored here.
                                        */
} SfsFileInfo, *SfsFileInfoPtr;


extern RpErrorCode SfsOpenFileSystem(SfsLength theOpenFilesCount);

extern RpErrorCode SfsCloseFileSystem(void);

extern RpErrorCode SfsOpenFile(SfsFileNumber theFileNumber,
                                        char *theFullNamePtr);

extern RpErrorCode SfsOpenStatus(SfsFileNumber theFileNumber,
                                        SfsStatus *theCompletionStatusPtr,
                                        SfsFileInfo *theFileInfoPtr);

extern RpErrorCode SfsCreateFile(SfsFileNumber theFileNumber,
                                        char *theFullNamePtr,
                                        SfsFileInfo *theFileInfoPtr);

extern RpErrorCode SfsCreateStatus(SfsFileNumber theFileNumber,
                                        SfsStatus *theCompletionStatusPtr);

extern RpErrorCode SfsCloseFile(SfsFileNumber theFileNumber,
                                        int theCompleteFlag);

extern RpErrorCode SfsCloseStatus(SfsFileNumber theFileNumber,
                                        SfsStatus *theCompletionStatusPtr);

extern RpErrorCode SfsReadFile(SfsFileNumber theFileNumber,
                                        char *theReadPtr,
                                        SfsLength theByteCount);

extern RpErrorCode SfsReadStatus(SfsFileNumber theFileNumber,
                                        SfsStatus *theCompletionStatusPtr,
                                        SfsLength *theBytesReadPtr);

extern RpErrorCode SfsWriteFile(SfsFileNumber theFileNumber,
                                        char *theWritePtr,
                                        SfsLength theByteCount);

extern RpErrorCode SfsWriteStatus(SfsFileNumber theFileNumber,
                                        SfsStatus *theCompletionStatusPtr,
                                        SfsLength *theBytesWrittenPtr);

extern RpErrorCode SfsSetFilePosition(SfsFileNumber theFileNumber,
                                        SfsLength theBytePosition);

extern RpErrorCode SfsSetFilePositionStatus(SfsFileNumber theFileNumber,
                                        SfsStatus *theCompletionStatusPtr);


#endif

/*

    Simple File System Notes:

    From the point of view of the host operating system, the RomPager engine
    is a single task.  It contains its own scheduler and control blocks for
    supporting multiple HTTP requests.  The calls it makes to the file system
    are asynchronous and have status completion calls to determine whether a
    file system activity has been completed.  Since real disks have latencies,
    the underlying file system needs to support asynchronous calls, or create
    an operating system task that can block on call completion.  In this way,
    the RomPager engine can continue to service other simultaneous HTTP requests.

    The file system needs to maintain for all open files a current byte
    position.  The byte positions are 1-relative and range from 1 to
    filesize.  The current value of a byte position is that of the byte
    about to be read.  The SfsOpenFile call assumes that the byte position
    is set to 1.  The SfsReadFile call assumes the read is at the current
    byte position with the new byte position set at the current position plus
    bytes read.  For example, a byte position of 100 means that the read
    pointer is after byte 99 and before byte 100.  A read of 10 bytes will
    read bytes 100 to 109, and set the byte position to 110.

    Normal file reads and writes take place sequentially.  That is, a file
    will be opened, read until is is ended and closed, or a file will be
    created, written until complete, and closed.  The HTTP 1.1 protocol
    supports the concept of partial file reads in order to reduce
    retransmission times.  The ability to support random file positions is
    a pre-requisite to supporting the HTTP byte-range commands.

*/
