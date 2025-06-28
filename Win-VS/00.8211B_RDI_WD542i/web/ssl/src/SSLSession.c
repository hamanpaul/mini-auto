/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Interface Spec		:	None.

* Description			:	This file contains the session related functions of SSL.

* Author Name			:	M.K.Saravanan

* Revision				:	1.0

* Known Bugs			:	None

******************************************************************************************/
#define EXTERN_SSL_MEMORY
#define DEFINE_CIPHER_LIST
#include <SSLCommon.h>

extern void	  SslTimerRemoveDs(UINT8 *, UINT32);		//YKS & Debu... 13/01/2004
extern UINT32 SSLCacheTimeout(void);					//YKS & Debu... 16/01/2004

/*****************************************************************************************
* Function Name	:	SSLRestoreSession
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pSessionID - (IN) The Session ID of session to be resumed
					INT32 lSessionIDLen - (IN) The session ID length
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_FAILURE(0) on error
* Description	:	This function restores the cache data from the previous cached session 
******************************************************************************************/
INT32 SSLRestoreSession(SSL *pSession, UINT8 *pSessionID,INT32 lSessionIDLen)
{
	SSLSessionCache *pCache;
	if(pSession->connectState.pRetrFunc != NULL) {
		/* use the restore callback function for restoring the older session */
		pCache = (SSLSessionCache *)pSession->connectState.pRetrFunc(pSessionID, 
			lSessionIDLen);
		if(pCache != NULL) {
			/* use the pointer previously stored to retrieve the session values such 
			as master secret, version, cipher suite used */
			memcpy(pSession->sessionCache.masterSecret, pCache->masterSecret, MASTER_SECRET_SIZE);
			pSession->sessionCache.pCipherMapping = pCache->pCipherMapping;
			pSession->sessionCache.version = pCache->version;
			
			/**** BEGIN - Added by YKS - 05/04/2004 ****/
			pSession->sessionCache.clientVersion = pCache->clientVersion;
			/**** END - Added by YKS - 05/04/2004 ****/

			pSession->secParams.sessionId.pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,
				lSessionIDLen);
			if(pSession->secParams.sessionId.pData == NULL) {
				return SSL_FAILURE;
			}
			/* copy the old session id */
			memcpy(pSession->secParams.sessionId.pData, pSessionID, lSessionIDLen);
			pSession->secParams.sessionId.ulSize = lSessionIDLen;
			return SSL_SUCCESS;
		} else {
			return SSL_FAILURE;
		}
	}
	return SSL_FAILURE;
}

/*****************************************************************************************
* Function Name	:	SSLRemoveSession
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_FAILURE(0) on error
* Description	:	This function removes the previously cached session
******************************************************************************************/
INT32 SSLRemoveSession(SSL *pSession)
{
	if(pSession->connectState.pRemFunc != NULL) {
		/* use the session remove callback function for removing the older cached session */
		pSession->connectState.pRemFunc(pSession->secParams.sessionId.pData, 
			pSession->secParams.sessionId.ulSize);
		return SSL_SUCCESS;
	}
	return SSL_FAILURE;
}

/*****************************************************************************************
* Function Name	:	SSLRegisterSession
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_FAILURE(0) on error
* Description	:	This function stores the new SSL session in cache
******************************************************************************************/
INT32 SSLRegisterSession(SSL *pSession)
{
	if(pSession->connectState.pStoreFunc != NULL) {
		/* use the session register callback function for stroing the 
		new session in cache */
		if(pSession->connectState.pStoreFunc(pSession->secParams.sessionId.pData, 
			pSession->secParams.sessionId.ulSize, (UINT8 *)&pSession->sessionCache, 
			sizeof(pSession->sessionCache)) == 1)
		{
			/* Creating a timer here */ //YKS & Debu... 13/01/2004
			//Since the session is caches successfully we need to start the timer
			SslCreateTimer(pSession->secParams.sessionId.pData,		\
						   pSession->secParams.sessionId.ulSize,	\
						   SSLCacheTimeout(),						\
					       SslTimerRemoveDs);
		}

		return SSL_SUCCESS;
	}
	return SSL_FAILURE;
}

/*****************************************************************************************
* Function Name	:	SSLIsServerCaching
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_TRUE(1) if session is cached
							SSL_FALSE(0) if not
* Description	:	This function returns whether the SSL sessions are cached or not
******************************************************************************************/
INT32 SSLIsServerCaching(SSL *pSession)
{
	if(pSession->connectState.pRemFunc != NULL && pSession->connectState.pRetrFunc!= NULL && 
		pSession->connectState.pStoreFunc != NULL) {
		/* if all the session resuming callbacks are set for the session, 
		then the session is cached by the server */
		return SSL_TRUE;
	}
	return SSL_FALSE;
}

/*****************************************************************************************
* Function Name	:	SSLSelectV2CipherSuite
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pCipherPtr - (IN) The cipher suite data list
					INT32 cipherListSize - (IN) The cipher data list size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
							SSL_FAILURE(0) on error
* Description	:	This function selects a cipher suite from the v2 cipher list data
******************************************************************************************/
INT32 SSLSelectV2CipherSuite(SSL *pSession, UINT8 *pCipherPtr, INT32 cipherListSize)
{
	UINT8 *pData;
	INT32 lDataLen;
	INT32 lCount1, lCount2;
	INT32 lRetVal;

	/* v2 cipher spec is 3 bytes, SSLv3 and TLSv1 cipher specs are two bytes
	convert v2 cipher spec to v3 cipher spec */
	lDataLen = (cipherListSize/SSL2_CIPHERSPEC_SIZE)*SSL_CIPHER_SIZE;
	pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lDataLen);
	if(pData == NULL) {
		return SSL_FAILURE;
	}

	/* converting v2 cipher specs to v3 cipher specs */
	lDataLen = 0;
	lCount2 = 0;
	for (lCount1 = 0; lCount1 < cipherListSize; lCount1 += SSL2_CIPHERSPEC_SIZE) {
		/* v3 equivalent representation of v2 cipher specs start with 0x00 */
		if (pCipherPtr[lCount1] == 0x00) {
			/* copy the v3 cipher spec */
			memcpy( &pData[lCount2], &pCipherPtr[lCount1+1], SSL_CIPHER_SIZE);
			lCount2+=SSL_CIPHER_SIZE;
		}
	}

	lDataLen = lCount2;

	/* select a v3 cipher suite */
	lRetVal = SSLSelectCipherSuite(pSession, pData, lDataLen);

	OS_ADPT_Deallocate_Memory(pData);

	return lRetVal;
}

/*****************************************************************************************
* Function Name	:	SSLSelectCipherSuite
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pCipherPtr - (IN) The cipher suite data list
					INT32 cipherListSize - (IN) The cipher data list size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
							SSL_FAILURE(0) on error
* Description	:	This function selects a cipher suite from the cipher list data
******************************************************************************************/
INT32 SSLSelectCipherSuite(SSL *pSession, UINT8 *pCipherPtr, INT32 cipherListSize)
{
	INT32 lCount;
	INT32 lCipherCount;
	INT32 lMacCiphers;
	INT32 lFound;
	UINT16 uPriority;

	/* get the number of cipher suites available */
	lMacCiphers = CIPHER_SUITES_COUNT;

	uPriority = SSLReadUint16(gCipherPriority);
	lFound = SSL_FAILURE;

	if(uPriority) {
		/* prioroty is set */
		for(lCipherCount = 0;lCipherCount < lMacCiphers;lCipherCount++) {
			if((memcmp(cipherSuiteList[lCipherCount].cipherSuite, &gCipherPriority[0],
				SSL_CIPHER_SIZE) == 0) && 
				(cipherSuiteList[lCipherCount].cipherEnabled == CIPHER_SUITE_ENABLED))
				break;
		}

		if(lMacCiphers != lCipherCount) {
			for (lCount = 0; lCount < cipherListSize; lCount += SSL_CIPHER_SIZE) {
				/* check whether prioritised cipher suite is given in client's list */
				if(memcmp(&gCipherPriority[0], &pCipherPtr[lCount], SSL_CIPHER_SIZE) == 0){
					if(!CertCheckAvailablity(cipherSuiteList[lCipherCount].certType)) {
							pSession->sessionCache.pCipherMapping = &cipherSuiteList[lCipherCount];
							/* cipher suite match found */
							lFound = SSL_SUCCESS;
							break;
					}
				}
			}
		}
	}

	if(lFound) {
		return SSL_SUCCESS;
	}

	for (lCount = 0; lCount < cipherListSize && lFound != SSL_SUCCESS; lCount += SSL_CIPHER_SIZE) {
		for(lCipherCount = 0;lCipherCount < lMacCiphers;lCipherCount++) {
			/* compares cipher suite list with the client's list */
			if ((memcmp(cipherSuiteList[lCipherCount].cipherSuite, &pCipherPtr[lCount], SSL_CIPHER_SIZE) == 0) 
				&& (cipherSuiteList[lCipherCount].cipherEnabled == CIPHER_SUITE_ENABLED)) {
				if(!CertCheckAvailablity(cipherSuiteList[lCipherCount].certType)) {
					pSession->sessionCache.pCipherMapping = &cipherSuiteList[lCipherCount];
					/* cipher suite match found */
					lFound = SSL_SUCCESS;
					break;
				}
			}
		}
	}
	return lFound;
}

/*****************************************************************************************
* Function Name	:	SSLFreeDHParams
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	None.
* Description	:	This function frees allocated DH parameters during handshake
******************************************************************************************/
#if SSL_CFG_DH_ENABLED
void SSLFreeDHParams(SSL *pSession)
{
	/* frees the prime */
	if(pSession->keyExParams.KeyExAlgorithm.DH.prime != NULL) {
		BN_free(pSession->keyExParams.KeyExAlgorithm.DH.prime);
		pSession->keyExParams.KeyExAlgorithm.DH.prime = NULL;
	}

	/* frees the generator */
	if(pSession->keyExParams.KeyExAlgorithm.DH.generator != NULL) {
		BN_free(pSession->keyExParams.KeyExAlgorithm.DH.generator);
		pSession->keyExParams.KeyExAlgorithm.DH.generator = NULL;
	}

	/* frees the server's private value */
	if(pSession->keyExParams.KeyExAlgorithm.DH.x != NULL) {
		BN_free(pSession->keyExParams.KeyExAlgorithm.DH.x);
		pSession->keyExParams.KeyExAlgorithm.DH.x = NULL;
	}
	
	/* frees the server's public value */
	if(pSession->keyExParams.KeyExAlgorithm.DH.X != NULL) {
		BN_free(pSession->keyExParams.KeyExAlgorithm.DH.X);
		pSession->keyExParams.KeyExAlgorithm.DH.X = NULL;
	}
}
#endif /* SSL_CFG_DH_ENABLED */


/*****************************************************************************************
* Function Name	:	SSLHandshakeBuffersClear
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	None.
* Description	:	This function frees the handshake buffers
******************************************************************************************/
void SSLHandshakeBuffersClear(SSL *pSession)
{
	/* free handshake hash buffers */
	memset(&pSession->connectState.hShakeMd5Hash,0, 
		sizeof(pSession->connectState.hShakeMd5Hash));
	memset(&pSession->connectState.hShakeSha1Hash,0, 
		sizeof(pSession->connectState.hShakeSha1Hash));

	/* free handshake receive buffer */
	if(pSession->connectState.handshakeRecvBuffer.ulSize != 0) {
		SSLRetrieveBuffer(&pSession->connectState.handshakeRecvBuffer, NULL, 
			pSession->connectState.handshakeRecvBuffer.ulSize);
	}

	/* free handshake send buffer */
	if(pSession->connectState.handshakeSendBuffer.ulSize != 0) {
		SSLRetrieveBuffer(&pSession->connectState.handshakeSendBuffer, NULL, 
			pSession->connectState.handshakeSendBuffer.ulSize);
	}

#if SSL_CFG_DH_ENABLED
	SSLFreeDHParams(pSession);
#endif /* SSL_CFG_DH_ENABLED */

#if SSL_CFG_REQUEST_CERTIFICATE
	if(pSession->connectState.pClientCert) {
		FreeCert(pSession->connectState.pClientCert);
		pSession->connectState.pClientCert = NULL;
	}
#endif /* SSL_CFG_REQUEST_CERTIFICATE */

}


/*****************************************************************************************
* Function Name	:	SSLSendChangeCipherSpec
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case the socket would block for this 
							operation
* Description	:	This function sends a change cipher spec message for this session
******************************************************************************************/
INT32 SSLSendChangeCipherSpec( SSL *pSession)
{
	UINT8 data = TYPE_CHANGE_CIPHER_SPEC;

	/* sends change cipher spec message over record layer */
	return SSLRecordSend(pSession, CHANGE_CIPHER_SPEC, &data, CHANGE_CIPHER_SPEC_SIZE);
}

/*****************************************************************************************
* Function Name	:	SSLRecvChangeCipherSpec
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_CONNECTION_CLOSED(0) on TCP connection closed
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case this operation would block on 
							socket
* Description	:	This function receives a change cipher spec message for this session
******************************************************************************************/
INT32 SSLRecvChangeCipherSpec(SSL *pSession)
{
	UINT8 data;
	
	/* receives change cipher spec message from record layer */
	return SSLRecordRecv(pSession, CHANGE_CIPHER_SPEC, -1, &data, CHANGE_CIPHER_SPEC_SIZE);
}

/*****************************************************************************************
* Function Name	:	SSLSendAlert
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					AlertLevel level - (IN) The alert level (warning or fatal)
					AlertDescription desc - (IN) The alert code
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case the socket would block for this 
							operation
* Description	:	This function sends a specific alert with specified level
******************************************************************************************/
INT32 SSLSendAlert(SSL *pSession, AlertLevel level, AlertDescription desc)
{
	UINT8 data[ALERT_SIZE];
	
	data[ALERT_LEVEL_POS] = (UINT8) level;
	data[ALERT_CODE_POS] = (UINT8) desc;

	if(level == AL_FATAL) {
		pSession->connectState.resumable= RESUME_FALSE;
	}

	return SSLRecordSend(pSession,ALERT, data, ALERT_SIZE);
}

/*****************************************************************************************
* Function Name	:	SSLGenerateExportSessionKeys
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
							SSL_FAILURE(0) on error
* Description	:	This function generates the export keys, IVs, and MAC secrets for 
					the session in case of export cipher suites
******************************************************************************************/
INT32 SSLGenerateExportSessionKeys(SSL *pSession)
{
	UINT8 random[SSL_RANDOM_SIZE*2];
	UINT8 randomSwap[SSL_RANDOM_SIZE*2];
	INT32 lHashSize;
	INT32 lIVSize;
	INT32 lKeySize;
	INT32 lRetVal;
	INT32 lDataPtr;
	CipherType cipher;
	INT32 lExpTemp;
	INT32 lExportKeySize;
	UINT8 *pExportKey;
	MD5_CTX	md5;
	UINT8 md5Digest[SSL_MD5_DIGEST_LEN];

	
	/* get the hash, key and IV sizes to be generated */
	lHashSize = SSLGetMessageDigestSize(pSession->sessionCache.pCipherMapping->macAlgorithm);
	lKeySize = SSLGetCipherKeySize(pSession->sessionCache.pCipherMapping->cipherAlgorithm);

	if(lKeySize > SSL_MD5_DIGEST_LEN) {
		return SSL_FAILURE;
	}
	cipher = SSLGetCipherType(pSession->sessionCache.pCipherMapping->cipherAlgorithm);
	if(cipher == CIPHER_BLOCK){
		lIVSize = SSLGetCipherIVSize(pSession->sessionCache.pCipherMapping->cipherAlgorithm);
		if(lIVSize > SSL_MD5_DIGEST_LEN) {
			return SSL_FAILURE;
		}
	} else {
		/* in case of stream ciphers IVs represent state buffers. The IVs are not generated
		by this function for stream ciphers but initialied by corresponding
		cipher initialisation state */
		lIVSize = SSLGetStreamStateSize(pSession->sessionCache.pCipherMapping->cipherAlgorithm);
	}

	/* gets the total size of key block, mulitplied by 2 for read and write states */
	pSession->connectState.keyBlock.ulSize = 2*(lHashSize + lKeySize + lIVSize);

	pSession->connectState.keyBlock.pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, 
		pSession->connectState.keyBlock.ulSize);

	if(pSession->connectState.keyBlock.pData == NULL) {
		pSession->connectState.keyBlock.ulSize = 0;
		return SSL_FAILURE;
	}

	/* actual export key lengths */
	lExportKeySize = SSLGetExportCipherKeySize(pSession->sessionCache.pCipherMapping->cipherAlgorithm);
	lExpTemp = 2*(lHashSize + lExportKeySize);

	pExportKey = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lExpTemp);
	if(pExportKey == NULL) {
		OS_ADPT_Deallocate_Memory(pSession->connectState.keyBlock.pData);
		pSession->connectState.keyBlock.ulSize = 0;
		return SSL_FAILURE;
	}

	/* make the random concatenations */
	memcpy(random, pSession->secParams.serverRandom, SSL_RANDOM_SIZE);
	memcpy(&random[SSL_RANDOM_SIZE], pSession->secParams.clientRandom, SSL_RANDOM_SIZE);

	memcpy(randomSwap, pSession->secParams.clientRandom, SSL_RANDOM_SIZE);
	memcpy(&randomSwap[SSL_RANDOM_SIZE], pSession->secParams.serverRandom, SSL_RANDOM_SIZE);

	
	if(pSession->sessionCache.version == SSL_VERSION3) {
		/* generate key block using ssl 3 random */
		lRetVal = SSL3GenerateRandom(pSession->sessionCache.masterSecret, 
			MASTER_SECRET_SIZE, random, SSL_RANDOM_SIZE*2, 
			pExportKey, lExpTemp);
	} else {
		/* generate key block using TLS 1 PRF */
		lRetVal = SSLGeneratePRF(pSession->sessionCache.masterSecret, MASTER_SECRET_SIZE, 
			TLS_KEY_EXPANSION_LABEL, KEY_EXPANSION_LABEL_SIZE, random, SSL_RANDOM_SIZE*2, 
			pExportKey, lExpTemp);
	}

	if(lRetVal != SSL_SUCCESS) {
		OS_ADPT_Deallocate_Memory(pSession->connectState.keyBlock.pData);
		OS_ADPT_Deallocate_Memory(pExportKey);
		pSession->connectState.keyBlock.ulSize = 0;
		return SSL_FAILURE;
	}

	/* copies the hash secrets */
	if(lHashSize > 0) {
		memcpy(pSession->connectState.keyBlock.pData, pExportKey, 2*lHashSize);
	}

	/* generates final cipher keys from the export keys */
	if(lKeySize > 0) {
		if(pSession->sessionCache.version == SSL_VERSION3) {
			/* generate SSL 3 final keys */
			MD5_Init(&md5);
			MD5_Update(&md5,&pExportKey[2*lHashSize], lExportKeySize);
			MD5_Update(&md5,randomSwap, 2*SSL_RANDOM_SIZE);
			MD5_Final(md5Digest, &md5);
	
			memcpy(&pSession->connectState.keyBlock.pData[2*lHashSize],md5Digest,
				lKeySize);
			

			MD5_Init(&md5);
			MD5_Update(&md5,&pExportKey[2*lHashSize+lExportKeySize], lExportKeySize);
			MD5_Update(&md5,random, 2*SSL_RANDOM_SIZE);
			MD5_Final(md5Digest, &md5);

			memcpy(&pSession->connectState.keyBlock.pData[2*lHashSize+lKeySize],
				md5Digest, lKeySize);
	
		} else {
			/* generate TLS 1 final keys */
			lRetVal = SSLGeneratePRF(&pExportKey[2*lHashSize], lExportKeySize, 
				TLS_CLIENT_WRITE_KEY_LABEL, CLIENT_WRITE_KEY_LABEL_SIZE, 
				randomSwap,	SSL_RANDOM_SIZE*2, 
				&pSession->connectState.keyBlock.pData[2*lHashSize], lKeySize);
			lRetVal = SSLGeneratePRF(&pExportKey[2*lHashSize+lExportKeySize], lExportKeySize, 
				TLS_SERVER_WRITE_KEY_LABEL, SERVER_WRITE_KEY_LABEL_SIZE, 
				randomSwap, SSL_RANDOM_SIZE*2, 
				&pSession->connectState.keyBlock.pData[2*lHashSize+lKeySize], lKeySize);
		}
	}

	if(lRetVal != SSL_SUCCESS) {
		OS_ADPT_Deallocate_Memory(pSession->connectState.keyBlock.pData);
		OS_ADPT_Deallocate_Memory(pExportKey);
		pSession->connectState.keyBlock.ulSize = 0;
		return SSL_FAILURE;
	}

	/* generates export IVs in case of block ciphers */
	if(lIVSize > 0 && cipher == CIPHER_BLOCK) {
		if(pSession->sessionCache.version == SSL_VERSION3) {
			/* generate SSL 3 IVs */
			MD5Sign(randomSwap,SSL_RANDOM_SIZE*2,md5Digest);
			
			memcpy(&pSession->connectState.keyBlock.pData[2*(lHashSize+lKeySize)],md5Digest,
				lIVSize);
			
			MD5Sign(random,SSL_RANDOM_SIZE*2,md5Digest);
			memcpy(&pSession->connectState.keyBlock.pData[2*(lHashSize+lKeySize)+lIVSize],md5Digest,
				lIVSize);
		} else {
			/* generate TLS 1 IVs */
			lRetVal = SSLGeneratePRF(NULL, 0, TLS_IV_BLOCK_LABEL, IV_BLOCK_LABEL_SIZE,
				randomSwap, SSL_RANDOM_SIZE*2, 
				&pSession->connectState.keyBlock.pData[2*(lHashSize+lKeySize)], lIVSize*2);
			if(lRetVal != SSL_SUCCESS) {
				OS_ADPT_Deallocate_Memory(pSession->connectState.keyBlock.pData);
				OS_ADPT_Deallocate_Memory(pExportKey);
				pSession->connectState.keyBlock.ulSize = 0;
				return SSL_FAILURE;
			}
		}
	}

		/* the total memory for key block is kept in keyBlock, but only pointers are ponted to
	in mac secret, IV's , keys for both read and write states */
	
	lDataPtr = 0;

	/* points the read and write mac secrets in key block */
	if(lHashSize > 0) {
		pSession->secParams.readMacSecret.pData = pSession->connectState.keyBlock.pData;
		pSession->secParams.readMacSecret.ulSize = lHashSize;

		lDataPtr += lHashSize;

		pSession->secParams.writeMacSecret.pData = &pSession->connectState.keyBlock.pData[lDataPtr];
		pSession->secParams.writeMacSecret.ulSize = lHashSize;

		lDataPtr += lHashSize;
	}

	/* points the read and write cipher keys in key block */
	if(lKeySize > 0) {
		pSession->secParams.readKey.pData = &pSession->connectState.keyBlock.pData[lDataPtr];
		pSession->secParams.readKey.ulSize = lKeySize;

		lDataPtr += lKeySize;
		
		pSession->secParams.writeKey.pData = &pSession->connectState.keyBlock.pData[lDataPtr];
		pSession->secParams.writeKey.ulSize = lKeySize;
		
		lDataPtr += lKeySize;
	}


	if(lIVSize > 0 ) {
		/* points the read and write IVs in key block */
		pSession->secParams.readIV.pData = &pSession->connectState.keyBlock.pData[lDataPtr];
		pSession->secParams.readIV.ulSize = lIVSize;

		lDataPtr += lIVSize;

		pSession->secParams.writeIV.pData = &pSession->connectState.keyBlock.pData[lDataPtr];
		pSession->secParams.writeIV.ulSize = lIVSize;

		lDataPtr += lIVSize;

		/* in case of stream ciphers the IV's hold the state information and is initialised
		by the corresponding cipher init functions */
		if(cipher == CIPHER_STREAM) {
			SSLInitStreamCipherState(pSession);
		}
	}

	OS_ADPT_Deallocate_Memory(pExportKey);
	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLGenerateSessionKeys
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
							SSL_FAILURE(0) on error
* Description	:	This function generates the keys, IVs, and MAC secrets for the session
******************************************************************************************/
INT32 SSLGenerateSessionKeys(SSL *pSession)
{
	UINT8 random[SSL_RANDOM_SIZE*2];
	INT32 lHashSize;
	INT32 lIVSize;
	INT32 lKeySize;
	INT32 lRetVal;
	INT32 lDataPtr;
	CipherType cipher;

	if(pSession->sessionCache.pCipherMapping->export == EXPORTABLE) {
		lRetVal = SSLGenerateExportSessionKeys(pSession);
		return lRetVal;
	}
	/* get the hash, key and IV sizes to be generated */
	lHashSize = SSLGetMessageDigestSize(pSession->sessionCache.pCipherMapping->macAlgorithm);
	lKeySize = SSLGetCipherKeySize(pSession->sessionCache.pCipherMapping->cipherAlgorithm);

	cipher = SSLGetCipherType(pSession->sessionCache.pCipherMapping->cipherAlgorithm);
	if(cipher == CIPHER_BLOCK){
		lIVSize = SSLGetCipherIVSize(pSession->sessionCache.pCipherMapping->cipherAlgorithm);
	} else {
		/* in case of stream ciphers IVs represent state buffers. The IVs are not generated
		by this function for stream ciphers but initialied by corresponding
		cipher initialisation state */
		lIVSize = SSLGetStreamStateSize(pSession->sessionCache.pCipherMapping->cipherAlgorithm);
	}

	/* gets the total size of key block, mulitplied by 2 for read and write states */
	pSession->connectState.keyBlock.ulSize = 2*(lHashSize + lKeySize + lIVSize);

	pSession->connectState.keyBlock.pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, 
		pSession->connectState.keyBlock.ulSize);

	if(pSession->connectState.keyBlock.pData == NULL) {
		pSession->connectState.keyBlock.ulSize = 0;
		return SSL_FAILURE;
	}

	memcpy(random, pSession->secParams.serverRandom, SSL_RANDOM_SIZE);
	memcpy(&random[SSL_RANDOM_SIZE], pSession->secParams.clientRandom, SSL_RANDOM_SIZE);

	if(pSession->sessionCache.version == SSL_VERSION3) {
		if(cipher == CIPHER_BLOCK){
		/* generate key block using ssl 3 random */
		lRetVal = SSL3GenerateRandom(pSession->sessionCache.masterSecret, 
			MASTER_SECRET_SIZE, random, SSL_RANDOM_SIZE*2, 
			pSession->connectState.keyBlock.pData, pSession->connectState.keyBlock.ulSize);
		} else {
			/* generate key block using ssl 3 random,  IVs are not needed */
			lRetVal = SSL3GenerateRandom(pSession->sessionCache.masterSecret, 
			MASTER_SECRET_SIZE, random, SSL_RANDOM_SIZE*2, 
			pSession->connectState.keyBlock.pData, 
			pSession->connectState.keyBlock.ulSize - lIVSize);
		}
	} else {
		if(cipher == CIPHER_BLOCK){
			/* generate key block using TLSv1 PRF */
			lRetVal = SSLGeneratePRF(pSession->sessionCache.masterSecret, MASTER_SECRET_SIZE, 
				TLS_KEY_EXPANSION_LABEL, KEY_EXPANSION_LABEL_SIZE, random, 
				SSL_RANDOM_SIZE*2, pSession->connectState.keyBlock.pData, 
				pSession->connectState.keyBlock.ulSize);
		} else {
			/* generate key block using TLSv1 PRF , IV's are not needed */
			lRetVal = SSLGeneratePRF(pSession->sessionCache.masterSecret, MASTER_SECRET_SIZE, 
				TLS_KEY_EXPANSION_LABEL, KEY_EXPANSION_LABEL_SIZE, random, 
				SSL_RANDOM_SIZE*2, 	pSession->connectState.keyBlock.pData, 
				pSession->connectState.keyBlock.ulSize - lIVSize);
		}
	}

	if(lRetVal != SSL_SUCCESS) {
		return lRetVal;
	}

	/* the total memory for key block is kept in keyBlock, but only pointers are ponted to
	in mac secret, IV's , keys for both read and write states */
	
	lDataPtr = 0;

	/* points the read and write mac secrets in key block */
	if(lHashSize > 0) {
		pSession->secParams.readMacSecret.pData = pSession->connectState.keyBlock.pData;
		pSession->secParams.readMacSecret.ulSize = lHashSize;

		lDataPtr += lHashSize;

		pSession->secParams.writeMacSecret.pData = &pSession->connectState.keyBlock.pData[lDataPtr];
		pSession->secParams.writeMacSecret.ulSize = lHashSize;

		lDataPtr += lHashSize;
	}

	/* points the read and write cipher keys in key block */
	if(lKeySize > 0) {
		pSession->secParams.readKey.pData = &pSession->connectState.keyBlock.pData[lDataPtr];
		pSession->secParams.readKey.ulSize = lKeySize;

		lDataPtr += lKeySize;
		
		pSession->secParams.writeKey.pData = &pSession->connectState.keyBlock.pData[lDataPtr];
		pSession->secParams.writeKey.ulSize = lKeySize;
		
		lDataPtr += lKeySize;
	}


	if(lIVSize > 0 ) {
		/* points the read and write IVs in key block */
		pSession->secParams.readIV.pData = &pSession->connectState.keyBlock.pData[lDataPtr];
		pSession->secParams.readIV.ulSize = lIVSize;

		lDataPtr += lIVSize;

		pSession->secParams.writeIV.pData = &pSession->connectState.keyBlock.pData[lDataPtr];
		pSession->secParams.writeIV.ulSize = lIVSize;

		lDataPtr += lIVSize;

		/* in case of stream ciphers the IV's hold the state information and is initialised
		by the corresponding cipher init functions */
		if(cipher == CIPHER_STREAM) {
			SSLInitStreamCipherState(pSession);
		}
	}

	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLReadConnectionStateInit
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	None.
* Description	:	This function initializes the read side of the SSL connection after 
					handshake
******************************************************************************************/
void SSLReadConnectionStateInit(SSL *pSession)
{
	/* initilaises read sequence number */
	SSLZeroUint64(&pSession->secParams.readSequenceNumber);

	/* initilaises read cipher and mac algorithm */
	pSession->secParams.readBulkCipherAlgorithm = pSession->sessionCache.pCipherMapping->cipherAlgorithm;
	pSession->secParams.readMacAlgorithm = pSession->sessionCache.pCipherMapping->macAlgorithm;
}

/*****************************************************************************************
* Function Name	:	SSLWriteConnectionStateInit
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	None.
* Description	:	This function initializes the write side of the SSL connection after 
					handshake
******************************************************************************************/
void SSLWriteConnectionStateInit(SSL *pSession)
{
	/* initilaises write sequence number */
	SSLZeroUint64(&pSession->secParams.writeSequenceNumber);

	/* initilaises write cipher and mac algorithm */
	pSession->secParams.writeBulkCipherAlgorithm = pSession->sessionCache.pCipherMapping->cipherAlgorithm;
	pSession->secParams.writeMacAlgorithm = pSession->sessionCache.pCipherMapping->macAlgorithm;
}

/*****************************************************************************************
* Function Name	:	SSLGetNoOfCipherSuites
* Parameters	:	None
* Return Value	:	INT32 - No of Cipher Suites
* Description	:	This function returns the number of cipher suites supported
******************************************************************************************/
INT32 SSLGetNoOfCipherSuites(void)
{
	return CIPHER_SUITES_COUNT;
}

/*****************************************************************************************
* Function Name	:	SSLCreateSession
* Parameters	:	None.
* Return Value	:	SSL * pSession - New Session pointer on success
					NULL - If memory could not be allocated
* Description	:	This allocate the memory for new SSL session and initializes it.
******************************************************************************************/
SSL *SSLCreateSession(void)
{
	SSL *pSession;

	/* allocate memory for new SSL session */
	pSession = (SSL *)OS_ADPT_Allocate_Memory(SSL_Memory,sizeof(SSL));
	if(pSession == NULL) {
		return NULL;
	}
	memset(pSession,0,sizeof(SSL));

	/* initialise the handshake state to init state */
	pSession->connectState.hsState = INIT_STATE;

	/* initialises the default parameters of the session */
	pSession->connectState.resumable = RESUME_TRUE;
	pSession->connectState.resumed= RESUME_FALSE;
	pSession->connectState.valid= VALID_SESSION;
	pSession->secParams.entity = SSL_SERVER;
	pSession->secParams.readBulkCipherAlgorithm = NULL_CIPHER;
	pSession->secParams.readMacAlgorithm = NULL_MAC;
	pSession->secParams.writeBulkCipherAlgorithm = NULL_CIPHER;
	pSession->secParams.writeMacAlgorithm = NULL_MAC;
	pSession->sessionCache.version = TLS_VERSION1;

	/* initilises the sequence number to zero */
	SSLZeroUint64(&pSession->secParams.writeSequenceNumber);
	SSLZeroUint64(&pSession->secParams.readSequenceNumber);

	/* initialise the handshake hansh buffers */
	SSLHShakeHashInit(pSession);
	return pSession;
}

/*****************************************************************************************
* Function Name	:	SSLCloseSession
* Parameters	:	SSL *pSession - (IN) The SSL session pointer.
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case the socket would block for this 
							operation
* Description	:	This sends a close notify alert and frees the SSL session memory.
******************************************************************************************/
INT32 SSLCloseSession(SSL *pSession)
{
	INT32 lRetVal;
	
	/* if the handshake is complete, then the session is closed by sending an close notify 
	alert */
	if(pSession->connectState.hsState == CLOSE_STATE ||
		pSession->connectState.hsState == APPLICATION_STATE) {
		pSession->connectState.hsState = CLOSE_STATE;
		/* send close notify alert */
		lRetVal = SSLSendAlert(pSession, AL_WARNING, CLOSE_NOTIFY_ALERT);
		if(lRetVal == SSL_WOULD_BLOCK) {
			return lRetVal;
		}
		pSession->connectState.hsState = CLEAR_STATE;
	}

	/* if the session is not resumable, then remove it from session cache */
	if(pSession->connectState.resumable == RESUME_FALSE) {
		SSLRemoveSession(pSession);
	}

	/* free the handshake buffers */
	SSLHandshakeBuffersClear(pSession);
	

	/* frees the record layer application buffer */
	if(pSession->connectState.applicationData.ulSize!= 0) {
		SSLRetrieveBuffer(&pSession->connectState.applicationData, NULL, 
			pSession->connectState.applicationData.ulSize);
	}

	/* frees the record layer handshake buffer */
	if(pSession->connectState.handshakeData.ulSize!= 0) {
		SSLRetrieveBuffer(&pSession->connectState.handshakeData, NULL, 
			pSession->connectState.handshakeData.ulSize);
	}

	/* frees the key block generated for the session */
	if(pSession->connectState.keyBlock.ulSize != 0) {
		OS_ADPT_Deallocate_Memory(pSession->connectState.keyBlock.pData);
		pSession->connectState.keyBlock.ulSize = 0;
	}

	/* frees the record receive buffer */
	if(pSession->connectState.recordRecvBuffer.ulSize!= 0) {
		SSLRetrieveBuffer(&pSession->connectState.recordRecvBuffer, NULL, 
			pSession->connectState.recordRecvBuffer.ulSize);
	}

	/* frees the record send buffer */
	if(pSession->connectState.recordSendBuffer.ulSize != 0) {
		SSLRetrieveBuffer(&pSession->connectState.recordSendBuffer, NULL, 
			pSession->connectState.recordSendBuffer.ulSize);
	}
	
	/* frees session ID */
	if(pSession->secParams.sessionId.ulSize != 0) {
		OS_ADPT_Deallocate_Memory(pSession->secParams.sessionId.pData);
		pSession->secParams.sessionId.ulSize = 0;
	}

	/* frees the SSL session */
	OS_ADPT_Deallocate_Memory(pSession);
	return SSL_SUCCESS;
}


/*****************************************************************************************
* Function Name	:	SSLSetCipherSuitePriority
* Parameters	:	UINT16 cipherSuite - (IN) The cipher suite.
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
							SSL_FAILURE(0) on error
* Description	:	This sets the cipher suite priority.
******************************************************************************************/
INT32 SSLSetCipherSuitePriority(UINT16 cipherSuite)
{
	INT32 lRetVal;
	INT32 lCipherCount;
	INT32 lMacCiphers;
	CipherSuite cipher;

	SSLWriteUint16(cipherSuite, cipher);

	/* gets the maximum ciphers suites present */
	lMacCiphers = CIPHER_SUITES_COUNT;
	lRetVal = SSL_FAILURE;

	/* check the cipher suite is present in the list */
	for(lCipherCount = 0;lCipherCount < lMacCiphers;lCipherCount++) {
		if((memcmp(cipherSuiteList[lCipherCount].cipherSuite, &cipher[0], 
			SSL_CIPHER_SIZE) == 0) && 
			(cipherSuiteList[lCipherCount].cipherEnabled == CIPHER_SUITE_ENABLED)) {
			/* sets the cipher suite priority */
			memcpy(gCipherPriority, cipher, sizeof(CipherSuite));
			lRetVal = SSL_SUCCESS;
			break;
		}
	}
	return lRetVal;
}

/*****************************************************************************************
* Function Name	:	SSLSetSessionResumeFunc
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					SSL_RETR_FUNC *pRetrFunc - (IN) The session retrieval function
					SSL_STORE_FUNC *pStoreFunc - (IN) The session cache function
					SSL_REMOVE_FUNC *pRemFunc - (IN) The session remove function
* Return Value	:	None.
* Description	:	This sets the the application functions needed for session resuming.
******************************************************************************************/
void SSLSetSessionResumeFunc ( SSL *pSession, SSL_RETR_FUNC *pRetrFunc, 
							  SSL_STORE_FUNC *pStoreFunc, SSL_REMOVE_FUNC *pRemFunc)
{
	if(pRetrFunc != NULL && pStoreFunc != NULL && pRemFunc != NULL) {
		/* sets the callback functions needed for session resuming */
		pSession->connectState.pStoreFunc = pStoreFunc;
		pSession->connectState.pRemFunc = pRemFunc;
		pSession->connectState.pRetrFunc = pRetrFunc;
	}
}

/*****************************************************************************************
* Function Name	:	SSLSetTransportPtr
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					INT32 lSocketDescr - (IN) The socket descriptor for this session
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
							SSL_FAILURE(0) on error
* Description	:	This sets the the socket descriptor for this session.
******************************************************************************************/
INT32 SSLSetTransportPtr(SSL *pSession, INT32 lSocketDescr)
{
	if(lSocketDescr > 0) {
		/* sets the socket descriptor for this session */
		pSession->connectState.lSocketDescr = lSocketDescr;
		return SSL_SUCCESS;
	}
	return SSL_FAILURE;
}