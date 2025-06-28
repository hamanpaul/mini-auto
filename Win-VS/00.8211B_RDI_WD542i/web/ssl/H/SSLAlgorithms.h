/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Description			:	This file contains algorithm related information for SSL

* Author Name			:	M.K.Saravanan

******************************************************************************************/

#ifndef SSLALGORITHMS_H
#define SSLALGORITHMS_H


#define	SSL_MD5_DIGEST_LEN			16
#define	SSL_SHA1_DIGEST_LEN			20

#define SSL_3DES_KEY_SIZE			24
#define SSL_RC4_128_KEY_SIZE		16
#define SSL_RC4_40_FINAL_KEY_SIZE	16

#define SSL_RC4_40_EXPORT_KEY_SIZE	5


#define SSL_3DES_IV_SIZE			8

#define SSL_3DES_BLOCK_SIZE			8
#define SSL_STREAM_BLOCK_SIZE		0

#if SSL_CFG_RC4_128_ENABLED
#define SSL_RC4_STREAM_STATE_SIZE	sizeof(RC4Context)
#endif /* SSL_CFG_RC4_128_ENABLED */


#define	HMAC_EXTENDED_KEY_LEN		64

#define SSL3_MAC_MD5_PAD_SIZE		48
#define SSL3_MAC_SHA1_PAD_SIZE		40

#define TLS_MAC_DATA_FIXED_SIZE		13
#define SSL_MAC_DATA_FIXED_SIZE		11

#define MAX_SEED_SIZE				200

#define NO_OF_ALPHABETS				26


#endif /* SSLALGORITHMS_H */