/*
 * 
 * ------------------------------------------------------------------------
 * Device      :  pc code use for  DES3526 
 * Function    :  Enable Admin
 
 * Parameters  :  (1)Super User=> Username: demo Password:demo
                  (2)Normal User=> Username: usero Password: password
                  (3)Enable Admin User=> Username:      Password:admin
                  
 * Html Pages  :  EnableAdmin.html
                  EnableAdmin0.html
                  
 * FileName    :  fRpWebID.c
 * Header file :  sRpWebID.h
 * Author      :  Arthur (00555) 
 * Date        :  2004-02-03
 * ------------------------------------------------------------------------
 */ 

#include "sRpWebID.h"

#ifdef WIN32
#include <windows.h>
#include <time.h>
#endif

// ------------------------------------------------------------
unsigned long dlk_GetCurrentSystemTimeValue(void)
{
	unsigned long theCurrentTime;
	//time(&theCurrentTime);
	return  theCurrentTime;
}
// ------------------------------------------------------------
void dlk_LogLoginHistory(unsigned long ip, char *username, int success, int isLOGIN)


{
	if (success)
	{
/*
				printf("Login Success: %d.%d.%d.%d ",
					((ip&0x000000ff)>>0),
					((ip&0x0000ff00)>>8),
					((ip&0x00ff0000)>>16),
					((ip&0xff000000)>>24));
				printf("UserName: %s\n", username);
*/
	}
	else
	{
/*
				printf("Login Fail: %d.%d.%d.%d ",
					((ip&0x000000ff)>>0),
					((ip&0x0000ff00)>>8),
					((ip&0x00ff0000)>>16),
					((ip&0xff000000)>>24));
				printf("UserName: %s\n",username);
*/
	}
}
// ------------------------------------------------------------
void dlk_LogLogoutHistory(unsigned long ip, char *username, int success)
{
	if (success)
	{
/*
				printf("Logout Success: %d.%d.%d.%d ",
					((ip&0x000000ff)>>0),
					((ip&0x0000ff00)>>8),
					((ip&0x00ff0000)>>16),
					((ip&0xff000000)>>24));
				printf("UserName: %s\n", username);
*/
	}
	else
	{
/*
				printf("Logout Fail: %d.%d.%d.%d ",
					((ip&0x000000ff)>>0),
					((ip&0x0000ff00)>>8),
					((ip&0x00ff0000)>>16),
					((ip&0xff000000)>>24));
				printf("UserName: %s\n",username);
*/
	}
}
// ------------------------------------------------------------
int dlk_Is_TACACS_Enabled()
{
	int mode=1;

	return mode;
}
// ------------------------------------------------------------
int dlk_TACACS_Use_Local()
{
	int use_local=1;

	return use_local;
}
// ------------------------------------------------------------
int dlk_AAA_Auth(char *username, char *password)
{
	if (!dlk_Is_TACACS_Enabled())
		return 0;
	else
	if (dlk_TACACS_Use_Local())
		return 1;
	else
	if ((!strcmp(username, "test1"))&&(!strcmp(password, "111111")))
		return 2;
	else
		return 3;
}
// ------------------------------------------------------------
int dlk_Check_Enable_Admin(unsigned long ip, char *username, char *password)
{
			if (!strcmp(password, "admin"))
			{
				return 2; //Super User
			}
			return 0;
}
// ------------------------------------------------------------
int dlk_Check_UserAccessRight(unsigned long ip, char *username, char *password, unsigned long *authen_method)

{
    int result=dlk_AAA_Auth(username, password); 
    switch(result)
    {
    	case 2:
			dlk_LogLoginHistory(ip, username, 0, 1);
		
		    return 2; //Super User

    		break;
    		
    	case 3://enable but notauthorized
			      
			dlk_LogLoginHistory(ip, username, 0,0);
			return 0; //Invalid User
    		break;
    		
    	case 0://initialbox
    	case 1://enable but disconnect
    	default:
			if ((!strcmp(username, "demo"))&&(!strcmp(password, "demo")))
			{
			    dlk_LogLoginHistory(ip, username,0, 1);
				return 2; //Super User
			}
			else
			if ((!strcmp(username, "user"))&&(!strcmp(password, "password")))
			{
				dlk_LogLoginHistory(ip, username,0, 1);
				return 1; //Normal User
			}
			else
			{
				dlk_LogLoginHistory(ip, username, 0,1);
				return 0; //Invalid User
			}
    		break;
    }//end switch
}


/*---------------------------------------------------------------------------
 * Function  : FWeb_GetAAAStatus()
 * Purpose   : Get Authentication Policy Status
 * Notes     : 0: AAA Disabled => run AAADisabledEnableAdmin.html
 *             1: AAA Enabled  => run EnableAdmin.html
 * Added by  : Iris Chu (04443)  2004-02-06
 * --------------------------------------------------------------------------
 */
int FWeb_GetAAAStatus(void)
{
    int AAAStatus;

     AAAStatus=1;

	return AAAStatus;
}


/*------------------------------------------------------------------------
 * Function:    ST_WEB_Update_UserStat 
 * Purpose:     update user login status for web
 * Parameters:  
 *    Input:    index and login information
 *    Output:   None
 * returns:     TRUE : success FALSE : failure
 *------------------------------------------------------------------------
 */
int ST_WEB_Update_UserStat(int i, CHALLENGE_LOGIN *login)
{
    
    return 1;
}



