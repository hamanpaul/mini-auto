/*
 *  File:       SfilInt.h
 *
 *  Contains:   Internal definitions for simple file system interface routines
 *              for Standard C library
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Copyright:  © 1997-2001 by Allegro Software Development Corporation
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
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Change History (most recent first):
 *
 *      08/12/98    rhb     created from Macintosh version
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.6 * * * *
 * * * * Release 1.5 * * * *
 * * * * Release 1.4 * * * *
 * * * * Release 1.3 * * * *
 * * * * Release 1.2 * * * *
 * * * * Release 1.1 * * * *
 * * * * Release 1.0 * * * *
 *
 *  To Do:
 */


#ifndef _SFILINT_
#define _SFILINT_


#define kAscii_Backslash    0x5C
#define kAscii_Colon        0x3A


typedef struct {
    size_t      fBytesRead;
    FILE *      fFilePtr;
    rpDataType  fFileType;
} FileData, *FileDataPtr;


#endif


