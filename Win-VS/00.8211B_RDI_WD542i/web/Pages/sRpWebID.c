/*
 * 
 * ------------------------------------------------------------------------
 * Device      :  pc code use for  DES3526 
 * Function    :  Enable Admin
 
 * Parameters  :  (1)Super User       => Username: demo Password:demo
                  (2)Normal User      => Username: user Password: password
                  (3)Enable Admin User=> Username:      Password:admin
                  
 * Html Pages  :  AAA_Admin.html
                  EnableAdmin0.html
               
                * AAA Rule  :  (1)Authentication policy State
                   Disabled :
                   	Authentication policy is disabled ! 
                   Enabled:
                        1.This web management is already in "Admin" level. 	 
                        0.EnableAdmin0.html Run html_EnableAdminProcess process   
                  
                  
 * FileName    :  sRpWebID.c
 * Header file :  sRpWebID.h
 * Author      :  Arthur (00555)                   
 * Date        :  2004-02-03                  
                  
                     

 * Notes       :  the 4.3 web Kernel must modify 5 files (Iris Chu(04443) 02-05-0224 written notes)
                  (1)RpConfig.h (common\Rompager\Include)                  
                  (2)AsMain.c   (common\Engince\Source\)
                  (3)RpAccess.c (common\Rompager\Source\)
                  (4)RpData.c   (common\Rompager\Source\)
                  (5)RpHttpps.c (common\WebServer\Source\)
				  
                  the web page will modfiy 
                  (1)sLogout.c
                  (2)start.html 
                  
                  
 
 * ------------------------------------------------------------------------
 */ 
 
#include <stdio.h>
#include "web_util.h"
#include "AsEngine.h"

#include "sRpWebID.h"
// +++ 03/01/2004, jacob_shih
#if GM_INCLUDED
#include "sSIM.h"
#endif
// --- 03/01/2004, jacob_shih

CHALLENGE_LOGIN	        gChallenge[CHALLENGE_LOGIN_LEN];
int ST_WEB_Update_UserStat(int, CHALLENGE_LOGIN *);
int Web_Check_Idle_Timeout()
{
	int i=0;
	unsigned long current_time=dlk_GetCurrentSystemTimeValue();

	while (i<CHALLENGE_LOGIN_LEN)
	{
	  if ((strcmp(gChallenge[i].RpWebID, "XXXXXXXX"))&&((current_time-gChallenge[i].last_browse_time)>900))  // 900 secs = 15 mins
	  {
		strcpy(gChallenge[i].RpWebID, "XXXXXXXX");
		gChallenge[i].ip=0;
		gChallenge[i].last_browse_time=0;
		gChallenge[i].username[0]=0;
		gChallenge[i].password[0]=0;
		gChallenge[i].login_flag=0;
		gChallenge[i].access_right=0;
		gChallenge[i].enabled_admin=0;
                ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master

	  }
	  i++;
	}
	return 1;
}

int RpWebID_Need_New(char *l_RpWebID)
{
	int need_new=1;
	int stop=0;
	int i=0;

	if (!strcmp(l_RpWebID, "XXXXXXXX"))
		return need_new;

	while ((stop==0)&&(i<CHALLENGE_LOGIN_LEN))
	{
//		if ((!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))&&(gChallenge[i].enabled_admin==1))
		if (!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))
		{
			need_new=0;
			stop=1;
		}
		else
		{
			i++;
		}
	}
	return need_new;
}


char *html_GetStartPageProcess(void *theServerDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr)
{
	rpDataPtr		 theDataPtr = (rpDataPtr)theServerDataPtr;
	rpHttpRequestPtr theRequestPtr= theDataPtr->fCurrentHttpRequestPtr;
	rpConnectionPtr  theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	char l_RpWebID[9];

	Web_Check_Idle_Timeout();

	strcpy(l_RpWebID, theRequestPtr->fHttpCookies[0]);

	HPF(NULL);

	if (RpWebID_Need_New(l_RpWebID))
	{
		HPF("<script language='JavaScript'>\n");
		sprintf(l_RpWebID, "%08x", dlk_GetCurrentSystemTimeValue());
		HPF("document.cookie = 'RpWebID=%s';\n", l_RpWebID);
		HPF("</script>\n");
/*		
		HPF("<frameset framespacing='0' border='false' frameborder='0' cols='160,*'>\n");
// +++ 02/16/2004, jacob_shih
//	change the url of start.html from '/html/start.html' to '/' in rom object 
//	list by specifying the RpUrl=/ in RpPageHeader of start.html.
//	therefore, use downward relative url in frame source.
#if 0
		HPF("<frame name='fLogo' scrolling='no' noresize src='../html/Hlogo.html' marginwidth='0' marginheight='0'>\n");
		HPF("<frame name='fInfo' scrolling='auto' noresize src='../html/login.html' marginwidth='0' marginheight='0'>\n");
#else
		HPF("<frame name='fLogo' scrolling='no' noresize src='html/Hlogo.html' marginwidth='0' marginheight='0'>\n");
		HPF("<frame name='fInfo' scrolling='auto' noresize src='html/login.html' marginwidth='0' marginheight='0'>\n");
#endif
// --- 02/16/2004, jacob_shih
		HPF("<noframes>\n");
		HPF("<body bgcolor='#008080'>\n");
		HPF("<p>This page uses frames, but your browser doesn't support them.</p>\n");
		HPF("</body>\n");
		HPF("</noframes>\n");
		HPF("</frameset>\n");
*/		
		HPF("<script language='JavaScript'>\n");
		HPF("function JumpToHmain()");
		HPF("{location.replace('html/Hmain.html');}");
		HPF("window.setTimeout('JumpToHmain()',1);");	
		HPF("</script>\n");
	}
	else
	{
		HPF("<script language='JavaScript'>\n");
		HPF("function JumpToHmain()");
		HPF("{location.replace('html/Hmain.html');}");
		HPF("window.setTimeout('JumpToHmain()',1);");	
		HPF("</script>\n");
	}

	return HPF("\n");
}

void Initial_CHALLENGE_LOGIN_DataBase(void)
{
	int i=0;
	for (i=0; i<CHALLENGE_LOGIN_LEN; i++)
	{
		strcpy(gChallenge[i].RpWebID, "XXXXXXXX");
		gChallenge[i].ip=0;
		gChallenge[i].last_browse_time=0;
		gChallenge[i].username[0]=0;
		gChallenge[i].password[0]=0;
		gChallenge[i].login_flag=0;
		gChallenge[i].access_right=0;
		gChallenge[i].enabled_admin=0;
                ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master
		
		
	}
}

int RpWebID_Check_Login_Flag(char *l_RpWebID)
{
	int find=0;
	int i=0;
	while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
	{
		if (!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))
		{
			find=gChallenge[i].login_flag;
		}
		else
		{
			i++;
			find=0;
		}
	}
	return find;
}


int RpWebID_Check_Enable_Admin(char *l_RpWebID)
{
    int find=0;
    int i=0;
    while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
    {
        if (!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))
        {
            if (gChallenge[i].enabled_admin==1)
            {
                gChallenge[i].enabled_admin=2;
                gChallenge[i].login_flag = 1;
                find=1;
                ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master
            }
            else if (gChallenge[i].enabled_admin==2)
            {
                gChallenge[i].enabled_admin=2;
                gChallenge[i].login_flag = 1;
                find=2;
                ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master
            }
            else if (gChallenge[i].enabled_admin>2)
            {
                gChallenge[i].enabled_admin=3;
                gChallenge[i].login_flag = 2;
                find=2;
                ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master
            }
        }
        i++;
    }
    return find;
}


int RpWebID_Update_Browse_Time(char *l_RpWebID)
{
	int find=0;
	int i=0;
	while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
	{
		if ((strcmp(l_RpWebID, "XXXXXXXX"))&&(!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID)))
		{
			find=1;
		        gChallenge[i].last_browse_time=dlk_GetCurrentSystemTimeValue();
		        ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master

		}
		else
		{
			i++;
			find=0;
		}
	}
	return find;
}

int	RpWebID_Add_Account_Waiting_For_Login_Challenge(char *l_RpWebID)
{
	int find=0;
	int i=0;
	while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
	{
		if (gChallenge[i].login_flag==0)
		{
				strcpy(gChallenge[i].RpWebID, l_RpWebID);
				gChallenge[i].ip = 0;
        		        gChallenge[i].last_browse_time=0;
				gChallenge[i].username[0] = 0;
				gChallenge[i].password[0] = 0;
				gChallenge[i].login_flag = 1;
        		        gChallenge[i].access_right=0;
        		        gChallenge[i].enabled_admin=0;
				find=1;
				ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master
                                				
				                
		}
		else
		{
			i++;
			find=0;
		}
	}
	return find;
}

int	RpWebID_Set_Already_Challenged(char *l_RpWebID, unsigned long ip, char *username, char *password, int access_right)
{
	int find=0;
	int i=0;
	while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
	{
		if (!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))
		{
				gChallenge[i].login_flag = 2;
				gChallenge[i].ip = ip;
				strcpy(gChallenge[i].username, username);
				strcpy(gChallenge[i].password, password);
				gChallenge[i].access_right = access_right;				
                                ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master				
                                find=1;
		}
		else
		{
			i++;
			find=0;
		}
	}
	return find;
}

int RpWebID_Reset_Login_Flag(char *l_RpWebID)
{
	int find=0;
	int i=0;
	while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
	{
		if (!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))
		{
		strcpy(gChallenge[i].RpWebID, "XXXXXXXX");
		gChallenge[i].ip=0;
		gChallenge[i].last_browse_time=0;
		gChallenge[i].username[0]=0;
		gChallenge[i].password[0]=0;
		gChallenge[i].login_flag=0;
                gChallenge[i].access_right=0;
                gChallenge[i].enabled_admin=0;
                ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master				
		}
		else
		{
			i++;
			find=0;
		}
	}
	return find;
}

/*---------------------------------------------------------------------------
 * Function :html_MyProtectedObjectProcess
 * Purpose  : Get device IP 
  Notes    :      Get device IP 
  For example     10.44.43.1  (01 2B 2C 0A)  19606538
	          10.5.55.1                  20382986      
                  127.0.0.1   (localhost)    16777343;
            
 * Modified by : Aurthur  2004-02-04
 * --------------------------------------------------------------------------
 */

#include "sSSLCfg.h"
WEB_SSL_CONFIG_INFO webSSLCfgInfo;

char *html_MyProtectedObjectProcess(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr)
{
	rpDataPtr		theDataPtr = (rpDataPtr)theTaskDataPtr;
	rpHttpRequestPtr	theRequestPtr= theDataPtr->fCurrentHttpRequestPtr;

	RpWebID_Reset_Login_Flag(theRequestPtr->fHttpCookies[0]);

        
	HPF(NULL);
	HPF("<script>\n");
    #if SUPPORT_SSL
// +++ 2004/8/10 09:37上午, iris chu 04443 , modified Start
        fWeb_CfgSSL_Config_Get(&webSSLCfgInfo);
//        if (webSSLCfgInfo.status==3)//SSL                    
//           HPF("parent.location.replace('https://%s');\n", theRequestPtr->fHost);
//        else //Web           
//	   HPF("parent.location.replace('http://%s');\n", theRequestPtr->fHost);
// +++ 2004/8/10 09:37上午, iris chu 04443 , modified End	   
    #endif
	return HPF("</script>\n");
}


int	RpWebID_Set_Account_Waiting_For_Enable_Admin_Challenge(char *l_RpWebID)
{
	int find=0;
	int i=0;
	while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
	{
		if ((!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))&&(gChallenge[i].login_flag == 2))
		{
				gChallenge[i].enabled_admin = 1;
				ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master				
				find=1;
		}
		else
		{
			i++;
			find=0;
		}
	}
	return find;
}
//--------------EnableAdmin0.html--------------------------------------------
int	RpWebID_Get_Data(char *l_RpWebID, CHALLENGE_LOGIN *data);
char *html_EnableAdminProcess(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr)
{
	rpDataPtr		theDataPtr = (rpDataPtr)theTaskDataPtr;
	rpHttpRequestPtr	theRequestPtr= theDataPtr->fCurrentHttpRequestPtr;
	CHALLENGE_LOGIN data;
#if 0   /*Elsa modify for web*/
	WEB_SSL_CONFIG_INFO webSSLCfgInfo;
#endif

	RpWebID_Get_Data(theRequestPtr->fHttpCookies[0], (CHALLENGE_LOGIN *)&data);
	//dlk_LogLogoutHistory(data.ip, data.username, 1);
	RpWebID_Set_Account_Waiting_For_Enable_Admin_Challenge(theRequestPtr->fHttpCookies[0]);
    
#if 0   /*Elsa modify for web*/
	fWeb_CfgSSL_Config_Get(&webSSLCfgInfo);
#endif
	HPF(NULL);

	HPF("<script>\n");
#if 0   /*Elsa modify for web*/
	if (webSSLCfgInfo.status==3)//SSL      
		HPF("parent.location.replace('https://%s');\n", theRequestPtr->fHost);
	else
#endif
		HPF("parent.location.replace('http://%s');\n", theRequestPtr->fHost);
	return HPF("</script>\n");
}

int	RpWebID_Set_Account_Already_Admin_Challenge(char *l_RpWebID, char *username, char *password)
{
	int find=0;
	int i=0;
	while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
	{
		if ((!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))&&(gChallenge[i].enabled_admin == 2))
		{
				strcpy(gChallenge[i].username, username);
				strcpy(gChallenge[i].password, password);
				gChallenge[i].enabled_admin = 3;
				gChallenge[i].login_flag = 2;				
				find=1;
                                ST_WEB_Update_UserStat(i, &gChallenge[i]);// backup master								
		}
		else
		{
			i++;
			find=0;
		}
	}
	return find;
}

int	RpWebID_Get_Data(char *l_RpWebID, CHALLENGE_LOGIN *data)
{
	int find=0;
	int i=0;
	while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
	{
		if (!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))
		{
			memcpy(data, (char *)&gChallenge[i], sizeof(CHALLENGE_LOGIN));
			find=1;
		}
		else
		{
			i++;
			find=0;
		}
	}
	return find;
}

void dlk_LogLoginHistory(unsigned long , char *, int,int );
int Check_UserAccessRight(char *l_RpWebID, unsigned long ip, char *username, char *password, int enable_admin)
{
    int access_right=0; 
	CHALLENGE_LOGIN data;
	unsigned long   authen_method;
	int find=0;
	int i=0;

	if (enable_admin==2)
	{
		RpWebID_Get_Data(l_RpWebID, (CHALLENGE_LOGIN *)&data);
		access_right=dlk_Check_Enable_Admin(ip, data.username, password);
		if(access_right==2)
		{
            		//dlk_LogLoginHistory(ip, username, 1, 1);
			RpWebID_Set_Account_Already_Admin_Challenge(l_RpWebID, username, password);
		}

	}
	else
	{
		access_right=dlk_Check_UserAccessRight(ip, username, password, &authen_method);
        
		while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
		{
			if (!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))
			{
				gChallenge[i].authen_method = authen_method;
				find=1;
			}
			else
			{
				i++;
				find=0;
			}
    	}
	}

	return access_right;
}
//--------------EnableAdmi.html--------------------------------------------
char *DefineEnableAdminBtn(void)
{
	return	"<INPUT TYPE=\"BUTTON\" NAME=\"EnableAdmin\"   VALUE=\" Enable Admin \" onclick=\"onEnableAdmin()\">\n";
}

int RpWebID_GetAccessRight(char *l_RpWebID)
{
	int find=0;
	int i=0;
	int access_right=0;

	while ((find==0)&&(i<CHALLENGE_LOGIN_LEN))
	{
		if (!strcmp(&gChallenge[i].RpWebID[0], l_RpWebID))
		{
			find=1;
			access_right=gChallenge[i].access_right;
		}
		else
		{
			i++;
			find=0;
		}
	}
	return access_right;
}
/*---------------------------------------------------------------------------
 * Function  : html_RpWebID_GetAccessRight()
 * Purpose   : Get User Access Right
 * Notes     : 0:Read only
               1:Read /Write                      
 * Added by  :  Arthur (00555)  2004-02-04
 * --------------------------------------------------------------------------
 */
Unsigned8 html_RpWebID_GetAccessRight(void *theServerDataPtr, Signed16Ptr theIndexValuesPtr)
{
	rpDataPtr		 theDataPtr = (rpDataPtr)theServerDataPtr;
	rpHttpRequestPtr theRequestPtr= theDataPtr->fCurrentHttpRequestPtr;
	rpConnectionPtr  theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	char l_RpWebID[9];
	int access_right=0;
	int ret=0;

	strcpy(l_RpWebID, theRequestPtr->fHttpCookies[0]);
//+++ Arthur chow modify 2004/12/3 11:10上午
#if WIN32
	if (strstr(theRequestPtr->fAgent, "Allegro-Software-RemoteHost"))
		access_right = 2;
	else
		access_right=RpWebID_GetAccessRight(l_RpWebID);
#else		
	access_right=RpWebID_GetAccessRight(l_RpWebID);
#endif

// +++ 03/01/2004, jacob_shih
//	To grant the access right if the SIM role state is role_MS.
#if GM_INCLUDED
	if(1)
	{
		WEB_GM_ROLE_STATE_T role_state;
		web_GM_EPM_Get_Role_State(&role_state);
		if(role_state==role_MS)
		{
			if (strstr(theRequestPtr->fAgent, "Allegro-Software-RemoteHost"))
			{
				ret=1;
				return ret;
			}
		}
	}
#endif
// --- 03/01/2004, jacob_shih
	if (access_right>=2)    /* 1: user, 2:admin, 3:operator */
		ret=1;
	else
		ret=0;

	return ret;
}

/*---------------------------------------------------------------------------
 * Function  : html_RpWebID_GetAdminAccessRight()
 * Purpose   : Get User Access Right
 * Notes     : 0:Non admin user right
               1:Admin user right                      
 * Added by  :  Neil (05576)  2006-07-16
 * --------------------------------------------------------------------------
 */
Unsigned8 html_RpWebID_GetAdminAccessRight(void *theServerDataPtr, Signed16Ptr theIndexValuesPtr)
{
	rpDataPtr		 theDataPtr = (rpDataPtr)theServerDataPtr;
	rpHttpRequestPtr theRequestPtr= theDataPtr->fCurrentHttpRequestPtr;
	rpConnectionPtr  theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	char l_RpWebID[9];
	int access_right=0;
	int ret=0;

	strcpy(l_RpWebID, theRequestPtr->fHttpCookies[0]);
//+++ Arthur chow modify 2004/12/3 11:10上午
#if WIN32
	if (strstr(theRequestPtr->fAgent, "Allegro-Software-RemoteHost"))
		access_right = 2;
	else
		access_right=RpWebID_GetAccessRight(l_RpWebID);
#else		
	access_right=RpWebID_GetAccessRight(l_RpWebID);
#endif

// +++ 03/01/2004, jacob_shih
//	To grant the access right if the SIM role state is role_MS.
#if GM_INCLUDED
	if(1)
	{
		WEB_GM_ROLE_STATE_T role_state;
		web_GM_EPM_Get_Role_State(&role_state);
		if(role_state==role_MS)
		{
			if (strstr(theRequestPtr->fAgent, "Allegro-Software-RemoteHost"))
			{
				ret=1;
				return ret;
			}
		}
	}
#endif
// --- 03/01/2004, jacob_shih

	if (access_right==2)
		ret=1;
	else
		ret=0;

	return ret;
}


/*---------------------------------------------------------------------------
 * Function  : html_GetAAAStatus()
 * Purpose   : Get Authentication Policy Status
 * AAA Rule  :  (1)Authentication policy State
                   Disabled :
                   	Authentication policy is disabled ! 
                   Enabled:
                        1.This web management is already in "Admin" level. 	 
                        0.EnableAdmin0.html Run html_EnableAdminProcess process 
 	       
 * Notes     :	(1)html_GetAAAStatus 
                     0: AAA Disabled
                     1: AAA Enabled  
               
                (2)html_RpWebID_GetAccessRight
                     0:  Click "Enable Admin" button =>EnableAdmin0.html
                     1:  is alreay  is already in "Admin" level          
 * Added by  :  Iris Chu (04443)  2004-02-06
 *   :
 * --------------------------------------------------------------------------
 */
 Unsigned8 html_GetAAAStatus(void *theServerDataPtr, Signed16Ptr theIndexValuesPtr)
{

	int AAA_Status=0;
	int ret=0;



	AAA_Status=FWeb_GetAAAStatus();

	if (AAA_Status)//AAA Enabled
		ret=1;
	else
		ret=0;

	return ret;
 
}


