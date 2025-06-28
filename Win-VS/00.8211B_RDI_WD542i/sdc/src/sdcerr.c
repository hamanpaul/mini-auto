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
u8 gSdcerr=1;
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
#if 0
    if (gSdcerr==1)
    {
        gSdcerr=0;
        osdDrawCardError(0);
        OSTimeDly(5);
        uiClearOSDBuf(2);
        osdDrawPreviewIcon();        // civicuntest
    }
#endif
    return ucErrorCode;
}

