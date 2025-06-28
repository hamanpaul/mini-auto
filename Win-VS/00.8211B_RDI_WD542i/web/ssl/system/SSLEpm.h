/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Description			:	This file contains include information for SSL EEPROM and CLI

* Author Name			:	M.K.Saravanan

******************************************************************************************/
#ifndef SSL_EPM_H
#define SSL_EPM_H

#define EXTERN_CIPHER_LIST
#include <SSL.h>
#include <SSLCommon.h>
#include <dev/INC/epm_cpu.h>
//#include <cli/inc/cli_reg.h>
//#include <console/inc/con_root.h> 
//#include <console/cli/h/cli_cmd.h>//~jesson
//#include <console/cli/h/cli_reg.h>//~jesson

#define SSL_CONF_ENABLED	1
#define SSL_CONF_DISABLED	0

/* SSL EEPROM structure */
typedef struct
{
	INT8 SSLV3Enabled;			/* flag for SSL V3 */
	INT8 TLSV1Enabled;			/* flag for TLS V1 */
	CipherSuite Cipher[SSL_CFG_MAX_CSUITE_SUPPORTED];	/* SSL Cipher Suites */
	INT8 CipherEnabled[SSL_CFG_MAX_CSUITE_SUPPORTED];	/* flag for SSL SSL Cipher Suites */
	INT32 lNoOfCiphers;									/* no of cipher suites present */
	UINT32 u32TimerTimeout;		/* Timer timeout for session cache entry */
#if SSL_CFG_REQUEST_CERTIFICATE
	INT8 RequestCertificate;
#endif /* SSL_CFG_REQUEST_CERTIFICATE */
}SSL_EPM_SECTION_T;

/* function prototypes */
void SSL_EPM_Open_Data_Base(void);
void SSLLoadDataFromEpm(void);
INT32 SSLIsVersionEnabled(SSLVersion version);
void SSL_EPM_Open_Data_Base(void);
void SSL_CLI_CMD_Reg(void);
INT8 SSLEpmReqCert(void);

#endif /* SSL_EPM_H */

