/*

Copyright (c) 2005  Himax Technologies, Inc.

Module Name:

    sdcerr.c

Abstract:

    Process error situation.

Environment:

    ARM RealView Developer Suite

Revision History:

    2007/05/05  VCC Create

*/

#include "general.h"
#include "cferr.h"

u32 unCFErrorCode = 0;

/*

Routine Description:

    Handle the error condition.

Arguments:

    ucErrorCode - The error code which indicates the cause of error.
    ucOutputMessage - The message to print out.
    
Return Value:

    Error code.

*/

u32 cfErrHandle(u32 ucErrorCode, u8 *ucOutputMessage)
{
    DEBUG_CF("CF Error: %s\n", ucOutputMessage);
    unCFErrorCode = ucErrorCode;

    return ucErrorCode;
}

