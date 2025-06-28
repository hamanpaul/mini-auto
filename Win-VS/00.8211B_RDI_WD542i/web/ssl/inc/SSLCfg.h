/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Description			:	This file is configuration header for SSL

* Author Name			:	M.K.Saravanan

******************************************************************************************/

#ifndef SSLCFG_H
#define SSLCFG_H

#define SSL_CFG_BIG_ENDIAN				1
#define SSL_CFG_LITTLE_ENDIAN			0

#define SSL_CFG_WINDOWS					0
#define SSL_CFG_NUCLEUS					1

#define	SSL_CFG_DEBUG_ENABLE			0//~jesson
#define SSL_CFG_REQUEST_CERTIFICATE		1

/* SSL protocol related configurations */
#define SSL_CFG_MAX_EMPTY_RECORDS		4
#define SSL_CFG_MAX_HASH_SIZE			20
#define SSL_CFG_MAX_CSUITE_SUPPORTED	10


/* algorithm related configurations */
#define SSL_CFG_RSA_ENABLED				1
#define SSL_CFG_RSA_EXPORT_ENABLED		1
#define SSL_CFG_DH_ENABLED				1
#define SSL_CFG_DSS_ENABLED				1


#define SSL_CFG_3DES_EDE_CBC_ENABLED	1
#define SSL_CFG_RC4_128_ENABLED			1
#define SSL_CFG_RC4_40_ENABLED			1



#endif /* SSLCFG_H */
