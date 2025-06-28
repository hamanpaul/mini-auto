/*

Copyright (c) 2008 Mars Semiconductor Corp.

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
#include "sdcerr.h"

extern void uiClearOSDBuf(u8);
extern void osdDrawCardError(char );
extern void osdDrawPreviewIcon(void);
/*

Routine Description:

    Handle the error condition.

Arguments:

    ucErrorCode - The error code which indicates the cause of error.
    ucOutputMessage - The message to print out.

Return Value:

    Error code.

*/

u32 errHandle(u32 ucErrorCode, u8 *ucOutputMessage)
{
    DEBUG_SDC("SDC Error: %s\n", ucOutputMessage);
    
    return ucErrorCode;
}

