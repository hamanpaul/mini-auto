/*
 * 
 * ------------------------------------------------------------------------
 * Device      :  pc code use for  DES3526 
 * Function    :  web page logout 
 * Parameters  :  
 * Html Pages  :  MntLogout.html  
 * FileName    :  sLogout.c
 * Header file :  
 * Modify      :  Arthur (00555)
 * Date        :  04-02-2003 rp 4.0 ResetTimer 
 * Date        :  02-05-2004 rp 4.3 
 * ------------------------------------------------------------------------
 */ 
#include "web_util.h"
#include "AsEngine.h"
#include "sRpWebID.h"


extern rpObjectDescription Pgstart;
static void	ResetTimer(rpData * );
int RpWebID_Reset_Login_Flag(char *l_RpWebID);
int RpWebID_Get_Data(char *l_RpWebID, CHALLENGE_LOGIN *data);
//----------------------------------------------------------------------------
void MainTainLogoutApply(void *theServerDataPtr, Signed16Ptr theIndexValuesPtr)
{
	rpData *theDataPtr=(rpData*)theServerDataPtr;
	rpHttpRequestPtr	theRequestPtr= theDataPtr->fCurrentHttpRequestPtr;
	CHALLENGE_LOGIN data;

	theDataPtr->fCurrentHttpRequestPtr->fObjectPtr = &Pgstart;
	

	RpWebID_Get_Data(theRequestPtr->fHttpCookies[0], (CHALLENGE_LOGIN *)&data);
	dlk_LogLogoutHistory(data.ip, data.username, 1);
	RpWebID_Reset_Login_Flag(theRequestPtr->fHttpCookies[0]);
	return;

}
static void	ResetTimer(rpData * theDataPtr) {

	rpRealmPtr			theRealmPtr;
	rpUserPtr			theUserPtr;
	Unsigned16  theCount;

	theRealmPtr = theDataPtr->fRealms;
	theCount = kRpNumberOfUsers;

	while (theCount > 0){

			
		if(theRealmPtr->fUserLock ==NULL){
			//theRealmPtr += 1;
			theRealmPtr++;
			theCount--;
			continue;
		}
		else
			theUserPtr=theRealmPtr->fUserLock;


			// ---- Reset all connection timer for Realm Pages
			theUserPtr->fHttpSessionTimer = (Signed16)0;
			theUserPtr->fIpAddress = (Unsigned32)0;
			

            //clear any realms lock for this user..
			theRealmPtr->fUserLock = (rpUserPtr) 0;
        

		#if RomPagerSecurityDigest
			*theRealmPtr->fNonce = '\0';
		#endif	/* RomPagerSecurityDigest */
		#if RomPagerExternalPassword
			theRealmPtr->fExternalPasswordState = eRpPasswordPending;
		#endif		
		
		//theRealmPtr += 1;
			theRealmPtr++;
			theCount--;
	}
}

//----------------------------------------------------------------------------
