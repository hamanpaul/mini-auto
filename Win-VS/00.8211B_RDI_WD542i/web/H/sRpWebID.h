/*
 * 
 * ------------------------------------------------------------------------
 * Device      :  pc code use for  DES3526 
 * Function    :  Enable Admin
 
 * Parameters  :  (1)Super User=> Username: demo Password:demo
                  (2)Normal User=> Username: user Password: password
                  (3)Enable Admin User=> Username:      Password:admin
                  
 * Html Pages  :  EnableAdmin.html
                  EnableAdmin0.html

  Purpose  : 
  Notes    :      Get device IP 
  For example     10.44.43.1  (01 2B 2C 0A)  19606538
	          10.5.55.1                  20382986
	          
 * Parameters  :  (1)Super User=> Username: demo Password:demo
                  (2)Normal User=> Username: user Password: password
                  (3)Enable Admin User=> Username:      Password:admin           
                          
 * FileName    :  sRpWebID.h
 * WEB ile     :  sRpWebID.c
 * Author      :  Arthur (00555) 
 * Date        :  2004-02-03
 * ------------------------------------------------------------------------
 */ 

#include "RpConfig.h"
#include <AsConfig.h>

// data structure declaration
#define CHALLENGE_LOGIN_LEN 256

typedef struct  _CHALLENGE_LOGIN
{
	unsigned long ip;
	unsigned long last_browse_time;
	char username[kRpMaxUserNameLength];
	char password[kRpMaxPasswordLength];
	int enabled_admin;// 0:Not_Enable_Admin  1:Waiting_for_Enable_Admin_State_1  2:Waiting_for_Enable_Admin_State_2  3:Already_Enable_Admin
	int access_right;// 0:Not_exist  1:Read_Only  2:Read_Write
	char RpWebID[9];
	int login_flag;//0:Account_Not_Used,  1:Account_Waiting_for_Challenge,  2:Account_Already_Challenged
	unsigned long   authen_method;  /* 0: local(or local_enable), 1: none, 2: TACACS, 3: XTACACS, 4: TACACS+, 5: RADIUS */
}CHALLENGE_LOGIN;

//---------------------  fRpWebID.c function ------------------------------------------
unsigned long dlk_GetCurrentSystemTimeValue(void);
int dlk_Check_UserAccessRight(unsigned long ip, char *username, char *password, unsigned long *authen_method);
int dlk_Check_Enable_Admin(unsigned long ip, char *username, char *password);
void	dlk_LogLogoutHistory(unsigned long ip, char *username, int success);
int FWeb_GetAAAStatus(void);
Boolean fWeb_Get_Support_Enable_Admin(unsigned long authen_method);