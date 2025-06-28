/*
 *  File:       Spwd.c
 *
 *  Contains:   Test Shell for Simple External Password routine.
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
 * * * * Release 4.00  * * *
 * * * * Release 4.00b7  * * *
 *      08/31/00    pjr     modify for the new security model
 *      02/09/00    bva     use AsEngine.h
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.2 * * * *
 *      11/17/98    bva     add theDataPtr to SpwdGetExternalPassword
 *      09/03/98    bva     modified documentation
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 *      12/04/97    bva     modified documentation
 *      08/13/97    pjr     initial version
 * * * * Release 1.6 * * * *
 *
 *  To Do:
 */


/*  C Headers  */
#include <stdlib.h>
#include <string.h>


/*  Our Headers  */
#include "AsEngine.h"


#if RomPagerExternalPassword


typedef struct {
    char        fUsername[32];
    char        fPassword[32];
    rpAccess    fRealmAccessCode;
} rpPasswordTableEntry, *rpPasswordTableEntryPtr;


rpPasswordTableEntry gPasswordTable[] = {

        "User",
        "Password",
        kRpPageAccess_Realm1,

        "User2",
        "Password2",
        kRpPageAccess_Realm2,

        "User3",
        "Password3",
        kRpPageAccess_Realm1 | kRpPageAccess_Realm2,

        "John",
        "Smith",
        kRpPageAccess_Realm1,

        "Jane",
        "Doe",
        kRpPageAccess_Realm2,

        "NoPass",
        "",
        kRpPageAccess_Realm1,

        "",
        ""
};

int     gPendingCount = 0;

/*
    The SpwdGetExternalPassword routine is called when the RomPager engine
    needs to validate a user/password combination that the browser has
    provided against an external database. The call allows flexibility
    for the external database to be on another machine by providing
    a eRpPasswordPending response so that the RomPager engine can
    continue while an external verification takes place.

    Inputs:
        theConnectionId:        - the number of the connection this request
                                  came in on. This parameter is useful if
                                  more than one external request will be
                                  handled simultaneously.
        theUsernamePtr:         - the browser provided username

    Returns:
        thePasswordState:   eRpPasswordPending
                                - password authentication not yet complete.

                            eRpPasswordNotAuthorized
                                - authentication complete, the username
                                  was not recognized, reject the request.

                            eRpPasswordDone
                                - external authentication complete, RomPager
                                  needs to finish the authentication.

        The following fields are returned when the username is matched and
        eRpPasswordDone is returned:

        thePasswordPtr:         - the password for the user.

        theIpAddressPtr:        - the IP address for the user.  This may be
                                  set if a request will only be accepted from
                                  a particular IP address (for realms that
                                  have IP address protection enabled).  If
                                  a request will be accepted from any IP
                                  IP address, this field should be set to 0.

        theRealmAccessPtr:      - the realm access code indicating which
                                  realms the user is allowed to access.
*/

rpPasswordState SpwdGetExternalPassword(void *theTaskDataPtr,
                                    Unsigned8 theConnectionId,
                                    char *theUsernamePtr,
                                    char *thePasswordPtr,
                                    Unsigned32 *theIpAddressPtr,
                                    rpAccess *theRealmAccessPtr) {
    rpPasswordState             thePasswordState;
    rpPasswordTableEntryPtr     theTablePtr;

    *theRealmAccessPtr = 0;
    *theIpAddressPtr = 0;

    if (gPendingCount < 2) {
        gPendingCount++;
        thePasswordState = eRpPasswordPending;
    }
    else {
        gPendingCount = 0;

        thePasswordState = eRpPasswordNotAuthorized;

        theTablePtr = gPasswordTable;

        while (*theTablePtr->fUsername != '\0') {
            if (strcmp(theUsernamePtr, theTablePtr->fUsername) == 0) {
                strcpy(thePasswordPtr, theTablePtr->fPassword);
                *theRealmAccessPtr = theTablePtr->fRealmAccessCode;
                thePasswordState = eRpPasswordDone;
                break;
            }
            else {
                theTablePtr++;
            }
        }
    }

    return thePasswordState;
}

#endif  /* RomPagerExternalPassword */
