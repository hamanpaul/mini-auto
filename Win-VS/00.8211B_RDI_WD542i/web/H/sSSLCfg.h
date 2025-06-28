/* 
 * ------------------------------------------------------------------------
 * Device      :  use for  D-Link DHS-3224 Standalone Layer 2 Switch
 * Function    :  SSH Algorithm Settings
 * Html Pages  :  CfgSSHAlgorithm.html
 * Header file :  sSSHAlgr.c
 * Author      :  04443
 * Date        :  12-23-2002
 * ------------------------------------------------------------------------
 */ 

#define WEB_SSL_MESSAGE_LEN		8

typedef struct _WEB_SSL_CONFIG_INFO {
	int		RSA_with_RC4_128_MD5;
	int		RSA_with_3DES_EDE_CBC_SHA;	
	int		DHE_DSS_with_3DES_EDE_CBC_SHA;
	int		RSA_EXPORT_with_RC4_40_MD5;
	int     status;
	char	RSA_MD5[WEB_SSL_MESSAGE_LEN];
	char	RSA_SHA[WEB_SSL_MESSAGE_LEN];
	char	DHE_SHA[WEB_SSL_MESSAGE_LEN];
	char	RSA_EXPORT_MD5[WEB_SSL_MESSAGE_LEN];
	int		cache;
}WEB_SSL_CONFIG_INFO;

int fWeb_CfgSSL_Config_Apply(WEB_SSL_CONFIG_INFO data);
int fWeb_CfgSSL_Config_Get(WEB_SSL_CONFIG_INFO *data);