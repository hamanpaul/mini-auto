/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Description			:	This file is common header for SSL

* Author Name			:	M.K.Saravanan

******************************************************************************************/

#ifndef SSLCOMMON_H
#define SSLCOMMON_H

#include <SSL.h>
#include <SSLInternal.h>
#include <SSLPort.h>
#include <SSLAlgorithms.h>



#define SSL_RSA_WITH_RC4_128_MD5			{ 0x00,0x04 }
#define SSL_RSA_WITH_3DES_EDE_CBC_SHA		{ 0x00,0x0A }
#define SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA	{ 0x00,0x13 }
#define SSL_RSA_EXPORT_WITH_RC4_40_MD5		{ 0x00,0x03 }


#define CIPHER_SUITE_ENABLED	0x01

#ifdef DEFINE_CIPHER_LIST
CipherSuiteMappings cipherSuiteList[] = {
#if (SSL_CFG_RSA_ENABLED & SSL_CFG_RC4_128_ENABLED)
	{SSL_RSA_WITH_RC4_128_MD5, RSA_SIGN, SSL_KX_RSA, RC4_128_CIPHER, MD5_MAC, SSL_VERSION3, CIPHER_SUITE_ENABLED, NOT_EXPORTABLE},
#endif /* (SSL_CFG_RSA_ENABLED & SSL_CFG_RC4_128_ENABLED) */
#if (SSL_CFG_RSA_ENABLED & SSL_CFG_3DES_EDE_CBC_ENABLED)
	{SSL_RSA_WITH_3DES_EDE_CBC_SHA, RSA_SIGN, SSL_KX_RSA, TRIPLEDES_CIPHER, SHA1_MAC, SSL_VERSION3, CIPHER_SUITE_ENABLED, NOT_EXPORTABLE},
#endif /* (SSL_CFG_RSA_ENABLED & SSL_CFG_3DES_EDE_CBC_ENABLED) */
#if (SSL_CFG_DH_ENABLED & SSL_CFG_DSS_ENABLED & SSL_CFG_3DES_EDE_CBC_ENABLED)
	{SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA, DSS_SIGN, SSL_KX_DHE_DSS, TRIPLEDES_CIPHER, SHA1_MAC, SSL_VERSION3, CIPHER_SUITE_ENABLED, NOT_EXPORTABLE},
#endif /* (SSL_CFG_DH_ENABLED & SSL_CFG_DSS_ENABLED & SSL_CFG_3DES_EDE_CBC_ENABLED) */
#if (SSL_CFG_RSA_EXPORT_ENABLED & SSL_CFG_RC4_40_ENABLED)
	{SSL_RSA_EXPORT_WITH_RC4_40_MD5, RSA_SIGN, SSL_KX_RSA_EXPORT, RC4_40_CIPHER, MD5_MAC, SSL_VERSION3, CIPHER_SUITE_ENABLED, EXPORTABLE},
#endif /* (SSL_CFG_RSA_EXPORT_ENABLED & SSL_CFG_RC4_40_ENABLED) */
	{0}
};
#define CIPHER_SUITES_COUNT ((sizeof(cipherSuiteList)/sizeof(CipherSuiteMappings)) - 1)
CipherSuite	 gCipherPriority;

#if SSL_CFG_RSA_EXPORT_ENABLED
RSA *pTempRsa;
#endif

#endif /* DEFINE_CIPHER_LIST */


#ifdef EXTERN_CIPHER_LIST
extern CipherSuiteMappings cipherSuiteList[];
#endif

#endif /* SSLCOMMON_H */