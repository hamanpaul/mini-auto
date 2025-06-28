/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	SSL Handshake Layer - Key Exchange Messages

* Interface Spec		:	None.

* Description			:	This file contains the functions, for implementing the 
							SSL Handshake Key Exchange messages.

* Author Name			:	M.K.Saravanan

* Revision				:	1.0

* Known Bugs			:	None

******************************************************************************************/

#define EXTERN_SSL_MEMORY
#include <SSLCommon.h>

/*****************************************************************************************
* Function Name	:	SSLProcessRSAClientKeyEx
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pHData - (IN) The RSA client key exchange message
					INT32 lDataSize - (IN) The RSA client key exchange message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function processes RSA client key exchange handshake message 
******************************************************************************************/
#if (SSL_CFG_RSA_ENABLED || SSL_CFG_RSA_EXPORT_ENABLED)
static INT32 SSLProcessRSAClientKeyEx(SSL *pSession, UINT8 *pHData, INT32 lDataSize)
{
	UINT8 *pCipherText;
	INT32 lCipherLen;
	RSA	  *pRSA;
	UINT8 *pOutput;
	INT32 lRetVal;
#if SSL_CFG_RSA_EXPORT_ENABLED
	extern RSA *pTempRsa;
#endif /* SSL_CFG_RSA_EXPORT_ENABLED */

	/* get the pointer to RSA encypted data based on SSL version */
	if(pSession->sessionCache.version == SSL_VERSION3) {
		pCipherText = pHData;
		lCipherLen = lDataSize;
	} else {
		DECR_VALIDATE_LEN(lDataSize, TLS_CLIENT_KEYEX_LEN_SIZE);
		pCipherText = &pHData[TLS_CLIENT_KEYEX_LEN_SIZE];
		lCipherLen = SSLReadUint16(pHData);

		if(lCipherLen != lDataSize) {
			return SSL_FAILURE;
		}
	}

	/* check RSA encypted data length is greater than or equal to master secret */
	if(lCipherLen < MASTER_SECRET_SIZE) {
		return SSL_FAILURE;
	}

	/* memory for decrypted output */
	pOutput = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lCipherLen);

	if(pOutput == NULL) {
		return SSL_FAILURE;
	}

	if(pSession->sessionCache.pCipherMapping->kxAlgorithm == SSL_KX_RSA_EXPORT && 
		CertGetPublicKeySize() > SSL_RSA_EXPORT_KEY_LEN) {
		/* decrypt the message */
#if SSL_CFG_RSA_EXPORT_ENABLED
		lCipherLen = RSA_private_decrypt(lCipherLen,pCipherText,pOutput,pTempRsa,RSA_PKCS1_PADDING);
#endif /* SSL_CFG_RSA_EXPORT_ENABLED */
	} else {
		/* create a new RSA object */
		pRSA = RSA_new();
		if(pRSA == NULL) {
			OS_ADPT_Deallocate_Memory(pOutput);
			return SSL_FAILURE;
		}

		/* sets the RSA private key and modulus parametsr which are needed for decryption */
		CertGetRSAParams(&pRSA->n, &pRSA->d);

		/* decrypt the message */
		lCipherLen = RSA_private_decrypt(lCipherLen,pCipherText,pOutput,pRSA,RSA_PKCS1_PADDING);

		/* free the RSA object */
		pRSA->n = NULL;
		pRSA->d = NULL;
		RSA_free(pRSA);
	}

	if(lCipherLen < 0 || lCipherLen != MASTER_SECRET_SIZE) {
		/* in case RSA decryption fails, continue by choosing a random value as premaster
		secret Here the client is not informed and this is done to make SSL servers behave 
		same in case correctly formatted RSA and incorrectly formatted RSA blocks */
		SSLRand(pOutput, MASTER_SECRET_SIZE);
	} else {
		/* successfully decrypted, check the SSL version in first two bytes */
		/**** BEGIN - Added/Modified by YKS - 05/04/2004 ****/
#if 1	//Now after modification
		if(pSession->sessionCache.clientVersion == SSL_VERSION3 &&
			(pOutput[0] != SSL3_VER_MAJOR ||
			pOutput[1] != SSL3_VER_MINOR)) 
		{
			OS_ADPT_Deallocate_Memory(pOutput);
			return SSL_FAILURE;
		}
		if (pSession->sessionCache.clientVersion == TLS_VERSION1 && 
			(pOutput[0] != TLS1_VER_MAJOR ||
			pOutput[1] != TLS1_VER_MINOR)) 
		{
			OS_ADPT_Deallocate_Memory(pOutput);
			return SSL_FAILURE;
		}
#else	//Earlier... before modification
		if(pSession->sessionCache.version == SSL_VERSION3 &&
			(pOutput[0] != SSL3_VER_MAJOR ||
			pOutput[1] != SSL3_VER_MINOR)) {
			OS_ADPT_Deallocate_Memory(pOutput);
			return SSL_FAILURE;
		}
		if (pSession->sessionCache.version == TLS_VERSION1 && 
			(pOutput[0] != TLS1_VER_MAJOR ||
			pOutput[1] != TLS1_VER_MINOR)) {
			OS_ADPT_Deallocate_Memory(pOutput);
			return SSL_FAILURE;
		}
#endif
		/**** END - Added/Modified by YKS - 05/04/2004 ****/
	}

	/* compute master secret from the premaster secret */
	lRetVal = SSLPremasterToMaster(pSession, pOutput, MASTER_SECRET_SIZE);
		
	OS_ADPT_Deallocate_Memory(pOutput);
	return lRetVal;
}
#endif /* (SSL_CFG_RSA_ENABLED || SSL_CFG_RSA_EXPORT_ENABLED) */

/*****************************************************************************************
* Function Name	:	SSLProcessClientKeyEx
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pHData - (IN) The client key exchange message
					INT32 lDataSize - (IN) The client key exchange message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function processes client key exchange handshake message 
******************************************************************************************/
INT32 SSLProcessClientKeyEx(SSL *pSession, UINT8 *pHData, INT32 lDataSize)
{
	INT32 lRetVal;
	switch(pSession->sessionCache.pCipherMapping->kxAlgorithm) {

#if SSL_CFG_RSA_ENABLED
	case SSL_KX_RSA:
		/* process a RSA client key exchange message */
		lRetVal = SSLProcessRSAClientKeyEx(pSession, pHData, lDataSize);
		break;
#endif /* SSL_CFG_RSA_ENABLED */

#if SSL_CFG_RSA_EXPORT_ENABLED
	case SSL_KX_RSA_EXPORT:
		/* process a RSA export client key exchange message */
		lRetVal = SSLProcessRSAClientKeyEx(pSession, pHData, lDataSize);
		break;
#endif /* SSL_CFG_RSA_EXPORT_ENABLED */

#if (SSL_CFG_DH_ENABLED & SSL_CFG_DSS_ENABLED)
	case SSL_KX_DHE_DSS:
		/* process a DHE client key exchange message */
		lRetVal = SSLProcessDHEClientKeyEx(pSession, pHData, lDataSize);
		break;
#endif /* (SSL_CFG_DH_ENABLED & SSL_CFG_DSS_ENABLED) */

	default:
		lRetVal = SSL_FAILURE;
		break;
	}
	return lRetVal;
}

/*****************************************************************************************
* Function Name	:	SSLGenerateServerKeyEx
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 **pHData - (IN) The generated server key exchange message
					INT32 *pHDataSize - (IN) The server key exchange message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates server key exchnage message for this session
******************************************************************************************/
INT32 SSLGenerateServerKeyEx(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize)
{
	INT32 lRetVal;
	switch(pSession->sessionCache.pCipherMapping->kxAlgorithm) {

#if (SSL_CFG_DH_ENABLED & SSL_CFG_DSS_ENABLED)
	case SSL_KX_DHE_DSS:
		lRetVal = SSLGenerateDHServerKeyEx(pSession, pHData, pHDataSize);
		break;
#endif /* (SSL_CFG_DH_ENABLED & SSL_CFG_DSS_ENABLED) */

#if SSL_CFG_RSA_EXPORT_ENABLED
	case SSL_KX_RSA_EXPORT:
		lRetVal = SSLGenerateRSAExportServerKeyEx(pSession, pHData, pHDataSize);
		break;
#endif /* SSL_CFG_RSA_EXPORT_ENABLED */

#if SSL_CFG_RSA_ENABLED
	case SSL_KX_RSA:
#endif /* SSL_CFG_RSA_ENABLED */

	default:
		lRetVal = SSL_FAILURE;
		break;
	}
	return lRetVal;
}



/*****************************************************************************************
* Function Name	:	SSLServerKeyExNeeded
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_TRUE(1) if required
						  - SSL_FALSE(0) if not required
* Description	:	This function returns whether server key exchnage is required for the 
					selected key exchnage mechanism
******************************************************************************************/
INT32 SSLServerKeyExNeeded(SSL *pSession)
{
	INT32 lRetVal;
	switch(pSession->sessionCache.pCipherMapping->kxAlgorithm) {
#if SSL_CFG_RSA_ENABLED
	case SSL_KX_RSA:
		/* server key exchange not needed in case of RSA */
		lRetVal = SSL_FALSE;
		break;
#endif /* SSL_CFG_RSA_ENABLED */

#if SSL_CFG_RSA_EXPORT_ENABLED
	case SSL_KX_RSA_EXPORT:
		/* server key exchange needed in case of RSA public key length in the certificate exceeds
		export restriction 512 bits */
		if(CertGetPublicKeySize() > SSL_RSA_EXPORT_KEY_LEN) {
			lRetVal = SSL_TRUE;
		} else {
			lRetVal = SSL_FALSE;
		}
		break;
#endif /* SSL_CFG_RSA_ENABLED */

#if (SSL_CFG_DH_ENABLED & SSL_CFG_DSS_ENABLED)
	case SSL_KX_DHE_DSS:
		/* server key exchange needed in case of DHE_DSS */
		lRetVal = SSL_TRUE;
		break;
#endif /* (SSL_CFG_DH_ENABLED & SSL_CFG_DSS_ENABLED) */
	
	default:
		lRetVal = SSL_FALSE;
		break;
	}
	return lRetVal;
}

/*****************************************************************************************
* Function Name	:	SSLServerCertNeeded
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_TRUE(1) if required
						  - SSL_FALSE(0) if not required
* Description	:	This function returns whether server certificate is required for the 
					selected key exchnage mechanism
******************************************************************************************/
INT32 SSLServerCertNeeded(SSL *pSession)
{
	INT32 lRetVal;
	switch(pSession->sessionCache.pCipherMapping->kxAlgorithm) {

#if SSL_CFG_RSA_ENABLED
	case SSL_KX_RSA:
		/* server Cert needed in case of RSA */
		lRetVal = SSL_TRUE;
		break;
#endif /* SSL_CFG_RSA_ENABLED */

#if SSL_CFG_RSA_EXPORT_ENABLED
	case SSL_KX_RSA_EXPORT:
		/* server Cert needed in case of RSA */
		lRetVal = SSL_TRUE;
		break;
#endif /* SSL_CFG_RSA_EXPORT_ENABLED */

#if (SSL_CFG_DH_ENABLED & SSL_CFG_DSS_ENABLED)
	case SSL_KX_DHE_DSS:
		/* server Cert needed in case of DHE_DSS */
		lRetVal = SSL_TRUE;
		break;
#endif /* (SSL_CFG_DH_ENABLED & SSL_CFG_DSS_ENABLED) */

	default:
		lRetVal = SSL_FALSE;
		break;
	}
	return lRetVal;
}

#if SSL_CFG_REQUEST_CERTIFICATE
/*****************************************************************************************
* Function Name	:	SSLCertVerifyNeeded
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_TRUE(1) if required
						  - SSL_FALSE(0) if not required
* Description	:	This function returns whether to expect certificate verify message is 
					expected from the client.
******************************************************************************************/
INT32 SSLCertVerifyNeeded(SSL *pSession)
{
	INT32 lRetVal;

	if(pSession->connectState.pClientCert == NULL) {
		return SSL_FALSE;
	}

	switch(pSession->connectState.pClientCert->Algorithm) {

#if (SSL_CFG_RSA_ENABLED | SSL_CFG_RSA_EXPORT_ENABLED)
	case ALGO_RSA:
		/* Cert verify needed in case of RSA certificates */
		lRetVal = SSL_TRUE;
		break;
#endif /* (SSL_CFG_RSA_ENABLED | SSL_CFG_RSA_EXPORT_ENABLED) */

#if SSL_CFG_DSS_ENABLED
	case ALGO_DSA:
		/* Cert verify needed in case of DSS certificates */
		lRetVal = SSL_TRUE;
		break;
#endif /* SSL_CFG_DSS_ENABLED */

	default:
		lRetVal = SSL_FALSE;
		break;
	}
	return lRetVal;
}
#endif /* SSL_CFG_REQUEST_CERTIFICATE */



/*****************************************************************************************
* Function Name	:	SSLGenerateRSAExportServerKeyEx
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 **pHData - (OUT) The RSA server key exchange message
					INT32 *pHDataSize - (IN) The RSA server key exchange message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates a RSA server key exchange handshake message 
******************************************************************************************/
#if SSL_CFG_RSA_EXPORT_ENABLED
static INT32 SSLGenerateRSAExportServerKeyEx(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize)
{
	UINT16 modulusLen;
	UINT16 exponentLen;
	INT32 lRSAParamsLen;
	INT32 lDataPtr;
	UINT8 *pRSAParams;
	extern RSA *pTempRsa;
	INT32 lSignatureLen;
	UINT8 *pSignature;
	UINT8 *pSignData;
	INT32 lRetVal;
	UINT8 *pData;
	INT32 lDataSize;


	/* get the modulus and exponent sizes */
	modulusLen = BN_num_bytes(pTempRsa->n);
	exponentLen = BN_num_bytes(pTempRsa->e);

	/* calculate total params size */
	lRSAParamsLen = modulusLen + exponentLen + 2*SSL_RSA_PARAMS_LEN_SIZE;
	pRSAParams = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lRSAParamsLen);

	if(pRSAParams == NULL) {
		return SSL_FAILURE;
	}

	/* writes modulus length in the rsa params data */
	SSLWriteUint16(modulusLen, pRSAParams);
	lDataPtr = SSL_RSA_PARAMS_LEN_SIZE;

	/* copies modulus in the rsa params data */
	BN_bn2bin(pTempRsa->n, &pRSAParams[lDataPtr]);
	lDataPtr += modulusLen;

	/* writes exponent length in the rsa params data */
	SSLWriteUint16(exponentLen, &pRSAParams[lDataPtr]);
	lDataPtr += SSL_RSA_PARAMS_LEN_SIZE;

	/* copies exponent in the rsa params data */
	BN_bn2bin(pTempRsa->e, &pRSAParams[lDataPtr]);
	lDataPtr += exponentLen;

	/* allocate memory for signature input */
	pSignData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, SSL_RANDOM_SIZE*2 + lRSAParamsLen);

	if(pSignData == NULL) {
		OS_ADPT_Deallocate_Memory(pRSAParams);
		return SSL_FAILURE;
	}
	
	/* concatenate signature input */
	memcpy(pSignData, pSession->secParams.clientRandom, SSL_RANDOM_SIZE);
	memcpy(&pSignData[SSL_RANDOM_SIZE], pSession->secParams.serverRandom, SSL_RANDOM_SIZE);
	memcpy(&pSignData[SSL_RANDOM_SIZE*2], pRSAParams, lRSAParamsLen);

	/* generate corresponding signature */
	lRetVal = SSLGenerateSignature(pSession, pSignData, SSL_RANDOM_SIZE*2 + lRSAParamsLen,
		&pSignature, &lSignatureLen);

	OS_ADPT_Deallocate_Memory(pSignData);

	if(lRetVal != SSL_SUCCESS || lSignatureLen == 0) {
		OS_ADPT_Deallocate_Memory(pRSAParams);
		return SSL_FAILURE;
	}

	/* calculate key ex message length */
	lDataSize = HANDSHAKE_HEADER_SIZE+ lRSAParamsLen + SSL_SIGNATURE_LEN_SIZE + lSignatureLen;
	pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lDataSize);

	if(pData == NULL) {
		OS_ADPT_Deallocate_Memory(pRSAParams);
		OS_ADPT_Deallocate_Memory(pSignature);
		return SSL_FAILURE;
	}

	/* copies rsa params and signature to the key ex message */
	lDataPtr = HANDSHAKE_HEADER_SIZE;
	memcpy(&pData[lDataPtr], pRSAParams, lRSAParamsLen);
	lDataPtr += lRSAParamsLen;

	SSLWriteUint16((UINT16)lSignatureLen, &pData[lDataPtr]);
	lDataPtr += SSL_SIGNATURE_LEN_SIZE;
	memcpy(&pData[lDataPtr], pSignature, lSignatureLen);
	
	OS_ADPT_Deallocate_Memory(pSignature);
	OS_ADPT_Deallocate_Memory(pRSAParams);

	*pHData = pData;
	*pHDataSize = lDataSize;
	return SSL_SUCCESS;
}
#endif /* SSL_CFG_RSA_EXPORT_ENABLED */

/*****************************************************************************************
* Function Name	:	SSLGenerateDHServerKeyEx
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 **pHData - (OUT) The DH server key exchange message
					INT32 *pHDataSize - (IN) The DH server key exchange message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates a DH server key exchange handshake message 
******************************************************************************************/
#if SSL_CFG_DH_ENABLED
static INT32 SSLGenerateDHServerKeyEx(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize)
{
	UINT8 *pDHParams;
	INT32 lDHParamsLen;
	UINT16 primeLen;
	UINT16 generatorLen;
	UINT16 publicValLen;
	INT32 lDataPtr;
	UINT8 *pData;
	INT32 lDataSize;
	INT32 lRetVal;
	INT32 lSignatureLen;
	UINT8 *pSignature;
	UINT8 *pSignData;

	/* create DH parameters */
	pSession->keyExParams.KeyExAlgorithm.DH.prime = BN_new();
	pSession->keyExParams.KeyExAlgorithm.DH.generator = BN_new();
	pSession->keyExParams.KeyExAlgorithm.DH.x = BN_new();
	pSession->keyExParams.KeyExAlgorithm.DH.X = BN_new();

	if(pSession->keyExParams.KeyExAlgorithm.DH.prime == NULL ||
		pSession->keyExParams.KeyExAlgorithm.DH.generator == NULL ||
		pSession->keyExParams.KeyExAlgorithm.DH.x == NULL ||
		pSession->keyExParams.KeyExAlgorithm.DH.X == NULL) {
		SSLFreeDHParams(pSession);
		return SSL_FAILURE;
	}
	
	/* generate DH prime and generator */
	lRetVal = GenerateDHParams(pSession->keyExParams.KeyExAlgorithm.DH.prime, 
		pSession->keyExParams.KeyExAlgorithm.DH.generator);

	if(lRetVal) {
		SSLFreeDHParams(pSession);
		return SSL_FAILURE;
	}

	/* generates a random private value and calculates a public value */
	lRetVal = CalculateDHPublicValue(pSession->keyExParams.KeyExAlgorithm.DH.x, 
		pSession->keyExParams.KeyExAlgorithm.DH.X, 
		pSession->keyExParams.KeyExAlgorithm.DH.prime, 
		pSession->keyExParams.KeyExAlgorithm.DH.generator);

	if(lRetVal) {
		SSLFreeDHParams(pSession);
		return SSL_FAILURE;
	}

	/* get the sizes of DH params */
	primeLen = BN_num_bytes(pSession->keyExParams.KeyExAlgorithm.DH.prime);
	generatorLen = BN_num_bytes(pSession->keyExParams.KeyExAlgorithm.DH.generator);
	publicValLen = BN_num_bytes(pSession->keyExParams.KeyExAlgorithm.DH.X);

	/* calculate DH params data size */
	lDHParamsLen = primeLen + generatorLen + publicValLen + 3*SSL_DH_PARAMS_LEN_SIZE;

	pDHParams = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lDHParamsLen);

	if(pDHParams == NULL) {
		SSLFreeDHParams(pSession);
		return SSL_FAILURE;
	}

	/* writes prime length to DH params data */
	SSLWriteUint16(primeLen, pDHParams);
	lDataPtr = SSL_DH_PARAMS_LEN_SIZE;

	/* copies prime value to DH params data */
	BN_bn2bin(pSession->keyExParams.KeyExAlgorithm.DH.prime, &pDHParams[lDataPtr]);
	lDataPtr += primeLen;

	/* writes generator length to DH params data */
	SSLWriteUint16(generatorLen, &pDHParams[lDataPtr]);
	lDataPtr += SSL_DH_PARAMS_LEN_SIZE;

	/* copies generator value to DH params data */
	BN_bn2bin(pSession->keyExParams.KeyExAlgorithm.DH.generator, &pDHParams[lDataPtr]);
	lDataPtr += generatorLen;

	/* writes DH public length to DH params data */
	SSLWriteUint16(publicValLen, &pDHParams[lDataPtr]);
	lDataPtr += SSL_DH_PARAMS_LEN_SIZE;

	/* copies DH public value to DH params data */
	BN_bn2bin(pSession->keyExParams.KeyExAlgorithm.DH.X, &pDHParams[lDataPtr]);
	lDataPtr += publicValLen;

	/* allocate memory for signature input data */
	pSignData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, SSL_RANDOM_SIZE*2 + lDHParamsLen);

	if(pSignData == NULL) {
		OS_ADPT_Deallocate_Memory(pDHParams);
		SSLFreeDHParams(pSession);
		return SSL_FAILURE;
	}

	/* concatenates signature data */
	memcpy(pSignData, pSession->secParams.clientRandom, SSL_RANDOM_SIZE);
	memcpy(&pSignData[SSL_RANDOM_SIZE], pSession->secParams.serverRandom, SSL_RANDOM_SIZE);
	memcpy(&pSignData[SSL_RANDOM_SIZE*2], pDHParams, lDHParamsLen);

	/* generate corresponding signature */
	lRetVal = SSLGenerateSignature(pSession, pSignData, SSL_RANDOM_SIZE*2 + lDHParamsLen,
		&pSignature, &lSignatureLen);

	OS_ADPT_Deallocate_Memory(pSignData);

	if(lRetVal != SSL_SUCCESS) {
		OS_ADPT_Deallocate_Memory(pDHParams);
		SSLFreeDHParams(pSession);
		return SSL_FAILURE;
	}

	/* calculate key ex message length */
	if(lSignatureLen != 0) {
		lDataSize = HANDSHAKE_HEADER_SIZE+ lDHParamsLen + SSL_SIGNATURE_LEN_SIZE + lSignatureLen;
	} else {
		lDataSize = HANDSHAKE_HEADER_SIZE + lDHParamsLen;
	}
	pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lDataSize);

	if(pData == NULL) {
		SSLFreeDHParams(pSession);
		OS_ADPT_Deallocate_Memory(pDHParams);
		if(lSignatureLen) {
			OS_ADPT_Deallocate_Memory(pSignature);
		}
		return SSL_FAILURE;
	}
	
	/* copies DH params data and signature to the key ex message */
	lDataPtr = HANDSHAKE_HEADER_SIZE;
	memcpy(&pData[lDataPtr], pDHParams, lDHParamsLen);
	lDataPtr += lDHParamsLen;

	if(lSignatureLen != 0) {
		SSLWriteUint16((UINT16)lSignatureLen, &pData[lDataPtr]);
		lDataPtr += SSL_SIGNATURE_LEN_SIZE;
		memcpy(&pData[lDataPtr], pSignature, lSignatureLen);
		OS_ADPT_Deallocate_Memory(pSignature);
	}
	
	OS_ADPT_Deallocate_Memory(pDHParams);

	*pHData = pData;
	*pHDataSize = lDataSize;
	return SSL_SUCCESS;
}
#endif /* SSL_CFG_DH_ENABLED */

/*****************************************************************************************
* Function Name	:	SSLGenerateSignature
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pHData - (IN) The data to be signed
					INT32 lDataSize - (IN) The data size
					UINT8 **pSignature - (OUT) The output signature
					INT32 *pSignatureLen - (OUT) The signature length
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates a RSA or DSA signatures based on key ex. 
******************************************************************************************/
static INT32 SSLGenerateSignature(SSL *pSession, UINT8 *pData, INT32 lDataSize, 
						   UINT8 **pSignature, INT32 *pSignatureLen)
{
	UINT8 digest[SSL_SHA1_DIGEST_LEN+SSL_MD5_DIGEST_LEN];
	INT32 lRetVal;
#if SSL_CFG_DSS_ENABLED
	DSA *pDSA;
#endif /* SSL_CFG_DSS_ENABLED */

#if SSL_CFG_RSA_EXPORT_ENABLED
	RSA *pRSA;
#endif /* SSL_CFG_RSA_EXPORT_ENABLED */

	switch(pSession->sessionCache.pCipherMapping->kxAlgorithm) {
#if SSL_CFG_DSS_ENABLED
		case SSL_KX_DHE_DSS:
			/* creates new DSA object */
			pDSA = DSA_new();
			if(pDSA == NULL) {
				return SSL_FAILURE;
			}
			
			/* get DSA certificate params */
			CertGetDSAParams(&pDSA->p, &pDSA->q, &pDSA->g, &pDSA->priv_key);

			/* get the signature length */
			*pSignatureLen = DSA_size(pDSA);
			*pSignature = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, *pSignatureLen);
			if(*pSignature == NULL) {
				pDSA->p = NULL;
				pDSA->q = NULL;
				pDSA->g = NULL;
				pDSA->priv_key = NULL;
				DSA_free(pDSA);
				return SSL_FAILURE;
			}
			
			/* gets the SHA digest of signature data */
			SHA1Sign(pData, lDataSize, digest);
			
			/* makes a DSA signature */
			lRetVal = DSA_sign(0,digest,SSL_SHA1_DIGEST_LEN, *pSignature, (unsigned int *)pSignatureLen, 
				pDSA);

			pDSA->p = NULL;
			pDSA->q = NULL;
			pDSA->g = NULL;
			pDSA->priv_key = NULL;
			DSA_free(pDSA);

			if(lRetVal) {
				return SSL_SUCCESS;
			} else {
				OS_ADPT_Deallocate_Memory(*pSignature);
				return SSL_FAILURE;
			}
			break;
#endif /* SSL_CFG_DSS_ENABLED */

#if SSL_CFG_RSA_EXPORT_ENABLED
		case SSL_KX_RSA_EXPORT:
			/* creates a new RSA object */
			pRSA = RSA_new();
			if(pRSA == NULL) {
				return SSL_FAILURE;
			}

			/* gets RSA certificate params */			
			CertGetRSAParams(&pRSA->n, &pRSA->d);

			/* gets RSA signature length */
			*pSignatureLen = RSA_size(pRSA);
			*pSignature = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, *pSignatureLen);
			if(*pSignature == NULL) {
				pRSA->n = NULL;
				pRSA->d = NULL;
				RSA_free(pRSA);
				return SSL_FAILURE;
			}
			
			/* takes MD5 and SHA1 digest of signature data */
			MD5Sign(pData, lDataSize, digest);
			SHA1Sign(pData, lDataSize, &digest[SSL_MD5_DIGEST_LEN]);

			/* makes RSA signature */
			lRetVal = RSA_sign(NID_md5_sha1,digest,SSL_SHA1_DIGEST_LEN+SSL_MD5_DIGEST_LEN, 
				*pSignature, (unsigned int *)pSignatureLen, pRSA);

			pRSA->n = NULL;
			pRSA->d = NULL;
			RSA_free(pRSA);

			if(lRetVal) {
				return SSL_SUCCESS;
			} else {
				OS_ADPT_Deallocate_Memory(*pSignature);
				return SSL_FAILURE;
			}
			break;
#endif /* SSL_CFG_RSA_EXPORT_ENABLED  */

#if SSL_CFG_RSA_ENABLED 
		case SSL_KX_RSA:
#endif /* SSL_CFG_RSA_ENABLED  */

		default:
			return SSL_FAILURE;
	}
}

/*****************************************************************************************
* Function Name	:	SSLProcessDHEClientKeyEx
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pHData - (IN) The DHE client key ex message
					INT32 lDataSize - (IN) The data size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function processes a DHE client key exchane message. 
******************************************************************************************/
#if SSL_CFG_DH_ENABLED
static INT32 SSLProcessDHEClientKeyEx(SSL *pSession, UINT8 *pHData, INT32 lDataSize)
{
	INT32 lDataPtr;
	INT32 lLength;
	UINT16 dh_YcLen;
	BIGNUM *pdh_Yc;
	BIGNUM *pKey;
	UINT8 *pPremaster;
	INT32 lPremasterSize;
	INT32 lRetVal;

	/* checks the DH client public value length */
	lLength = lDataSize;
	DECR_VALIDATE_LEN(lLength, SSL_CLIENT_DH_LEN_SIZE);
	dh_YcLen = SSLReadUint16(pHData);
	lDataPtr = SSL_CLIENT_DH_LEN_SIZE;

	DECR_VALIDATE_LEN(lLength, dh_YcLen);

	/* converts client public value to bn format */
	pdh_Yc = BN_bin2bn(&pHData[lDataPtr], dh_YcLen, NULL);
	pKey = BN_new();

	if(pdh_Yc == NULL || pKey == NULL) {
		return SSL_FAILURE;
	}

	/* claculate the DH secret key */
	CalculateDHKey(pKey, pdh_Yc, pSession->keyExParams.KeyExAlgorithm.DH.x,
		pSession->keyExParams.KeyExAlgorithm.DH.prime);

	BN_free(pdh_Yc);

	/* calculated key forms the pre-master secret */
	lPremasterSize = BN_num_bytes(pKey);
	pPremaster = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lPremasterSize);

	if(pPremaster == NULL) {
		BN_free(pKey);
		return SSL_FAILURE;
	}
	
	/* converts key from bn to binary data */
	BN_bn2bin(pKey, pPremaster);

	BN_free(pKey);

	/* compute master secret from the premaster secret */
	lRetVal = SSLPremasterToMaster(pSession, pPremaster, lPremasterSize);

	OS_ADPT_Deallocate_Memory(pPremaster);
	return lRetVal;
}
#endif

/*****************************************************************************************
* Function Name	:	SSLPremasterToMaster
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pPremaster - (IN) The premaster secret
					INT32 lPremasterSize - (IN) The premaster secret size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function converts a premaster secret into master secret. 
******************************************************************************************/
static INT32 SSLPremasterToMaster(SSL *pSession, UINT8 *pPremaster, INT32 lPremasterSize)
{
	UINT8 random[2*MAX_RANDOM_SIZE];
	INT32 lRetVal;
	INT8 *pLabel;

	/* concatenates client and server random */
	memcpy(random,pSession->secParams.clientRandom,MAX_RANDOM_SIZE);
	memcpy(&random[MAX_RANDOM_SIZE],pSession->secParams.serverRandom,MAX_RANDOM_SIZE);

	lRetVal = SSL_FAILURE;
	if(pSession->sessionCache.version == SSL_VERSION3) {
		/* computes ssl 3 master secret */
		lRetVal = SSL3GenerateRandom(pPremaster,lPremasterSize, random , 2*MAX_RANDOM_SIZE , 
			pSession->sessionCache.masterSecret, MASTER_SECRET_SIZE);

	} else {
		/* computes tls 1 master secret */
		pLabel = TLS_MASTER_SECRET_LABEL;
		lRetVal = SSLGeneratePRF(pPremaster,lPremasterSize, pLabel, 
			MASTER_SECRET_LABEL_SIZE, random, 2*MAX_RANDOM_SIZE, 
			pSession->sessionCache.masterSecret, MASTER_SECRET_SIZE);
	}

	return lRetVal;
}
