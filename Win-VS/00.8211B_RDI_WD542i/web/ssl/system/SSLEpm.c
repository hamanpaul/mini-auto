/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Interface Spec		:	None.

* Description			:	This file contains functions implementing SSL saving runtime
							SSL optons and CLI commands.

* Author Name			:	M.K.Saravanan

* Revision				:	1.0

* Known Bugs			:	None

******************************************************************************************/

#include <SSLEpm.h>
#include <dev/inc/epm_sec.h>
#include <utl/inc/xprintf.h>
#include <cli/inc/cli_reg.h>
#include <cli/h/cli_utl.h>

/* globals */
SSL_EPM_SECTION_T		*ssl_info;
EPM_SEC_Ctrl_T			ssl_info_ctrl;
static	BOOLEAN_T				IsSSLInitialized = 0;


/* Function Prototypes */  
#if 0//~jesson for ssl
static void SSL_EPM_Factory_Default (void *ptr);
void SSLCliEnableCipherSuite(void *pstrMenuState,CLI_CBK_VALUE *value, void *user_data);
void SSLCliEnableVersion(void *pstrMenuState,CLI_CBK_VALUE *value, void *user_data);
void SSLCliShowVersion(void *pstrMenuState,CLI_CBK_VALUE *value, void *user_data);
void SSLCliShowCipherSuite(void *pstrMenuState,CLI_CBK_VALUE *value, void *user_data);
void SSLCliEnableCertRequest(void *pstrMenuState,CLI_CBK_VALUE *value, void *user_data);

#else
static int SSLCliEnableCipherSuite(const int sessionID,CLI_VALUE_T *ptrCliValue,void *ptrUserData);
static int SSLCliShowVersion(const int sessionID,CLI_VALUE_T *ptrCliValue,void *ptrUserData);
static int SSLCliShowCipherSuite(const int sessionID,CLI_VALUE_T *ptrCliValue,void *ptrUserData);
static int SSLCliEnableCertRequest (const int sessionID, CLI_VALUE_T *ptrCliValue, void *ptrUserData);
#endif

/*****************************************************************************************
* Function Name	:	SSL_EPM_Factory_Default
* Parameters	:	void *ptr - (IN) The EEPROM Section base pointer
* Return Value	:	None.
* Description	:	This function resets the SSL runtime parameters to factory settings
******************************************************************************************/
static void SSL_EPM_Factory_Default(void *ptr)
{
	SSL_EPM_SECTION_T	*db_tmp;
	INT32 lCount;

	if (ptr == NULL) {
		return;
	}
	db_tmp = (SSL_EPM_SECTION_T*)ptr;

	/* get the semaphore */
	ssl_info_ctrl.lock_semaphore();
	
	/* zeroes the EEPROM section */
	memset(db_tmp, 0, sizeof(SSL_EPM_SECTION_T));

	/* sets SSL Version 3 and TLS Version 1 enabled */
	db_tmp->SSLV3Enabled = SSL_CONF_DISABLED;
	db_tmp->TLSV1Enabled = SSL_CONF_DISABLED;
	db_tmp->u32TimerTimeout = SSL_TIMER_TIMEOUT;
	db_tmp->lNoOfCiphers = SSLGetNoOfCipherSuites();

	/* copies the supported cipher suites and enables them */
	for(lCount = 0; lCount < db_tmp->lNoOfCiphers; lCount++) {
		memcpy(&db_tmp->Cipher[lCount], cipherSuiteList[lCount].cipherSuite,SSL_CIPHER_SIZE);
		db_tmp->CipherEnabled[lCount] = SSL_CONF_ENABLED;
	}

#if SSL_CFG_REQUEST_CERTIFICATE
	db_tmp->RequestCertificate = 0;
#endif
	
	/* unlock the semaphore */
	ssl_info_ctrl.unlock_semaphore();
}

/*****************************************************************************************
* Function Name	:	SSLLoadDataFromEpm
* Parameters	:	None.
* Return Value	:	None.
* Description	:	This function loads the cipher suite settings from EEPROM
******************************************************************************************/
void SSLLoadDataFromEpm(void)
{
	INT32 lCount;
	INT32 lNoOfCS;

	lNoOfCS = SSLGetNoOfCipherSuites();

	/* if the no of cipher suites and count in EEPROM changed reload the defaults */
	if(ssl_info->lNoOfCiphers != lNoOfCS) {
		SSL_EPM_Factory_Default(ssl_info);
	}

	/* if the supported cipher suites changed reload defaults */
	for(lCount = 0; lCount < lNoOfCS; lCount++) {
		if(memcmp(&ssl_info->Cipher[lCount], cipherSuiteList[lCount].cipherSuite,SSL_CIPHER_SIZE) != 0) {
			SSL_EPM_Factory_Default(ssl_info);
			break;
		}
	}

	/* sets the cipher suite enabled or disabled based on EEPROM settings */
	for(lCount = 0; lCount < lNoOfCS; lCount++) {
		cipherSuiteList[lCount].cipherEnabled = ssl_info->CipherEnabled[lCount];
	}
}

/*****************************************************************************************
* Function Name	:	SSLIsVersionEnabled
* Parameters	:	SSLVersion version
* Return Value	:	INT32 - SSL_TRUE(1) if enabled
						  - SSL_FALSE(0) if disabled
* Description	:	This function checks whether SSL version is enabled or disabled.
******************************************************************************************/
INT32 SSLIsVersionEnabled(SSLVersion version) {
	/* checks SSL V3 is enabled in EEPROM */
	if(version == SSL_VERSION3 && ssl_info->SSLV3Enabled) {
		return SSL_TRUE;
	}

	/* checks TLS V1 is enabled in EEPROM */
	if(version == TLS_VERSION1 && ssl_info->TLSV1Enabled) {
		return SSL_TRUE;
	}
	return SSL_FALSE;
}

//BEGIN - YKS & Debu... 16/01/2004
/*****************************************************************************************
* Function Name	:	SSLCacheTimeout
* Parameters	:	None
* Return Value	:	UINT32 - The cache timeout interval
* Description	:	This function returns the cache timeout interval configured in the 
					EEPROM.
******************************************************************************************/
UINT32 SSLCacheTimeout(void)
{
	return ssl_info->u32TimerTimeout;
}
//END - YKS & Debu

/*****************************************************************************************
* Function Name	:	SSL_EPM_Open_Data_Base
* Parameters	:	None.
* Return Value	:	None.
* Description	:	This opens the SSL EEPROM section.
******************************************************************************************/
void SSL_EPM_Open_Data_Base(void)
{
	EPM_SEC_Code_T	open_code;
	EPM_SEC_Info_T	user_info;

	if(IsSSLInitialized == SSL_TRUE) {
		return;
	}

	/* setup section id and section size */
	user_info.section_id = EPM_SEC_ID_SSL;
	user_info.section_size = sizeof(SSL_EPM_SECTION_T);
	user_info.factory_default = SSL_EPM_Factory_Default;

	/* Open NV_RAM data base and get section data pointer */
	open_code = EPM_SEC_Open_Section(&user_info, &ssl_info_ctrl);
	
	/* I can write my data into EEPROM by this pointer */
	ssl_info = (SSL_EPM_SECTION_T*)ssl_info_ctrl.section_base;
	
	switch (open_code) {
		case EPM_SEC_SUCCESS:
			break;

		case EPM_SEC_LENGTH_FAIL:
			SSL_EPM_Factory_Default(ssl_info);
			break;

		default:
			SSL_EPM_Factory_Default(ssl_info);
			break;
	}

	IsSSLInitialized = SSL_TRUE;
	return;
}
//#if 0   // don't declare cli here
#if 0//------------------------- jesson for 3250 ---------------------------------------
/*****************************************************************************************
* Function Name	:	SSL_CLI_CMD_Reg
* Parameters	:	None.
* Return Value	:	None.
* Description	:	This registers the SSL CLI commands.
******************************************************************************************/
void SSL_CLI_CMD_Reg(void)
{
	cli_cmd_registration("/security/ssl/enableciphersuite (ecs) -p CipherSuite <hex> -p Enable <int> ",
	SSLCliEnableCipherSuite,
	CLU_USER_SUPER,
	NULL);
	
	cli_cmd_registration("/security/ssl/enableversion (ev) -p Version <int> -p Enable <int> ",
	SSLCliEnableVersion,
	CLU_USER_SUPER,
	NULL);

	cli_cmd_registration("/security/ssl/showciphersuite (scs) ",
	SSLCliShowCipherSuite,
	CLU_USER_SUPER,
	NULL);
	
	cli_cmd_registration("/security/ssl/showversion (sv) ",
	SSLCliShowVersion,
	CLU_USER_SUPER,
	NULL);

#if SSL_CFG_REQUEST_CERTIFICATE
	cli_cmd_registration("/security/ssl/enablecertrequest (ecr) -p Enable <int> ",
	SSLCliEnableCertRequest,
	CLU_USER_SUPER,
	NULL);
#endif

}

#else
/*****************************************************************************************
* Function Name	:	SSL_CLI_CMD_Init
* Parameters	:	None.
* Return Value	:	None.
* Description	:	This registers the SSL CLI commands.
******************************************************************************************/
void SSL_CLI_CMD_Reg(void)
{

	
#if 0
	CLI_CMD_Reg_Command(CLI_CMD_V1,
	cli_cmd_registration("/security/ssl/enableversion (ev) -p Version <int> -p Enable <int> ",
	SSLCliEnableVersion,
	CLU_USER_SUPER,
	NULL);
#endif


}

#endif//-------------------------------------------------

/*****************************************************************************************
* Function Name	:	SSLCliEnableVersion
* Parameters	:	CLI Parameters
* Return Value	:	None.
* Description	:	CLI callback function which enables or disables SSL versions
******************************************************************************************/
#if 0//jesson

void SSLCliEnableVersion(void *pstrMenuState,CLI_CBK_VALUE *value, void *user_data)
{
	CLI_PARA_VALUE		*pstrCbkTemp;
	INT32 lVersion;
	INT32 lEnable;

	
	if (value == NULL || value->num_of_para != 2 || value->cli_pv == NULL) {
		return;
	}

	pstrCbkTemp = value->cli_pv;
	lVersion = *((INT32 *)(((CLI_VALUE_IND *)(pstrCbkTemp->cli_value_list->value->cli_value))->value));

	/* checks whether the versions are supported */
	if(lVersion != SSL_VERSION3 && lVersion != TLS_VERSION1) {
		xprintf("\n Invalid Version");
		return;
	}

	pstrCbkTemp = pstrCbkTemp->next;
	lEnable = *((INT32 *)(((CLI_VALUE_IND *)(pstrCbkTemp->cli_value_list->value->cli_value))->value));

	if(lEnable != SSL_CONF_DISABLED && lEnable != SSL_CONF_ENABLED) {
		xprintf("\n Invalid Second Parameter");
		return;
	}

	/* enable or disable the SSL versions */
	if(lVersion == SSL_VERSION3) {
		if(ssl_info->SSLV3Enabled == (INT8)lEnable) {
			if(ssl_info->SSLV3Enabled) {
				xprintf("\n SSL V3 already enabled");
			} else {
				xprintf("\n SSL V3 already disabled");
			}
		} else {
			ssl_info_ctrl.lock_semaphore();
			ssl_info->SSLV3Enabled = (INT8)lEnable;
			ssl_info_ctrl.unlock_semaphore();
			if(ssl_info->SSLV3Enabled) {
				xprintf("\n SSL V3 successfully enabled");
			} else {
				xprintf("\n SSL V3 successfully disabled");
			}
		}
	} else {
		if(ssl_info->TLSV1Enabled == (INT8)lEnable) {
			if(ssl_info->TLSV1Enabled) {
				xprintf("\n TLS V1 already enabled");
			} else {
				xprintf("\n TLS V1 already disabled");
			}
		} else {
			ssl_info_ctrl.lock_semaphore();
			ssl_info->TLSV1Enabled = (INT8)lEnable;
			ssl_info_ctrl.unlock_semaphore();
			if(ssl_info->TLSV1Enabled) {
				xprintf("\n TLS V1 successfully enabled");
			} else {
				xprintf("\n TLS V1 successfully disabled");
			}
		}
	}
}

#endif//~jesson end

#if 0
//BEGIN - YKS & Debu... 16/01/2004
/*****************************************************************************************
* Function Name	:	SSLShowCacheTimeout
* Parameters	:	CLI Parameters
* Return Value	:	None.
* Description	:	CLI callback function for showing the cache timeout
******************************************************************************************/
void SSLShowCacheTimeout(void *pstrMenuState,CLI_CBK_VALUE *value, void *user_data)
{
	if (value == NULL || value->num_of_para != 0) 
	{
		return;
	}

	/* Printing the value */
	xprintf("\n Cache timeout is %d seconds\n", ssl_info->u32TimerTimeout);
}

/*****************************************************************************************
* Function Name	:	SSLConfigCacheTimeout
* Parameters	:	CLI Parameters
* Return Value	:	None.
* Description	:	CLI callback function for configuring the cache timeout
******************************************************************************************/
void SSLConfigCacheTimeout(void *pstrMenuState,CLI_CBK_VALUE *value, void *user_data)
{
	/* Variable(s) Declaration */
	INT32	i32Timeout;

	if (value == NULL || value->num_of_para != 1) 
	{
		return;
	}

	/* Reading the value */
	i32Timeout = *((INT32 *)(((CLI_VALUE_IND *)(value->cli_pv->cli_value_list->value->cli_value))->value));

	/* Checking the range entered */
	if((i32Timeout < SSL_MIN_TIMEOUT) || (i32Timeout > SSL_MAX_TIMEOUT))
	{
		xprintf("\n Incorrect value, should be in between %d seconds & %d seconds\n", SSL_MIN_TIMEOUT, SSL_MAX_TIMEOUT);
		return;
	}

	/* Setting the value in the EEPROM */
	ssl_info->u32TimerTimeout = (UINT32)i32Timeout;
}
#endif
//END - YKS & Debu... 16/01/2004

INT8 SSLEpmReqCert(void)
{
	return(ssl_info->RequestCertificate);
}
