/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	SSL Handshake Layer

* Interface Spec		:	None.

* Description			:	This file contains the functions, for implementing the 
							SSL Handshake Layer.

* Author Name			:	M.K.Saravanan

* Revision				:	1.0

* Known Bugs			:	None

******************************************************************************************/
#define EXTERN_SSL_MEMORY
#include <SSLCommon.h>

/*****************************************************************************************
* Function Name	:	SSLWriteHShakeBuffered
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					UINT8 *pHData - (IN) The handshake message to be sent
					INT32 lDataSize - (IN) The handshake message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case the socket would block for this 
							operation
* Description	:	This function sends the handshake message over SSL Record layer. 
					In case the previuos write was blocked this API will be called 
					with NULL parameters to flush the previous buffer.
******************************************************************************************/
static INT32 SSLWriteHShakeBuffered(SSL *pSession, UINT8 *pHData, INT32 lDataSize)
{
	INT32 lDataLeft;
	INT32 lRetVal;
	INT32 lReturnFlag;
	INT32 lBufferFlag;

	/* this function sends the data from handshake send buffer or the current data. 
	checks both does not present */
	if (pSession->connectState.handshakeSendBuffer.ulSize != 0 && lDataSize != 0) {
		return SSL_ERROR;
	}

	lBufferFlag = SSL_FALSE;
	if (lDataSize == 0) {
		/* gets the data and datasize to be sent from the handshake send buffer, 
		this is the buffer that got buffered from previous send */
		pHData = pSession->connectState.handshakeSendBuffer.pData;
		lDataSize = pSession->connectState.handshakeSendBuffer.ulSize;
		lBufferFlag = SSL_TRUE;
	}

	/* no data to write */
	if (lDataSize == 0) {
		return SSL_ERROR;
	}

	lDataLeft = lDataSize;
	lReturnFlag = SSL_FALSE;

	/* loop until all the data is sent out. In case of 
	SSL_ERROR(-1) return immediately
	SSL_WOULD_BLOCK(-2) save the remaining data in handshake send buffer and return immeditealy
	if SSLRecordSend return less no of bytes continue in loop to send the remaining 
	data */

	while (lDataLeft > 0) {
		/* send data to record layer */
		lRetVal = SSLRecordSend(pSession,HANDSHAKE, &pHData[lDataSize-lDataLeft], 
			lDataLeft);

		if (lRetVal == SSL_WOULD_BLOCK) {
			if(lBufferFlag) {
				/* the data is sent from the handshake send buffer, so remove whatever data
				that is sent from the buffer */
				SSLRetrieveBuffer(&pSession->connectState.handshakeSendBuffer,NULL,
					lDataSize-lDataLeft);
			} else {
				/* the current data is sent , so put the remaining data into the 
				handshake send buffer */
				if(!SSLInsertBuffer(&pSession->connectState.handshakeSendBuffer,
					&pHData[lDataSize-lDataLeft], lDataLeft)) {
					lRetVal = SSL_ERROR;
				}
			}
			lReturnFlag = SSL_TRUE;
			break;
		} else if (lRetVal == SSL_ERROR) {
			lReturnFlag = SSL_TRUE;
			break;
		}
		/* decrement the data sent now from the data left */
		lDataLeft -= lRetVal;
	}

	if(lReturnFlag) {
		return lRetVal;
	}

	if(lBufferFlag) {
		/* all the data is sent from the handshake send buffer, so empty the buffer */
		SSLRetrieveBuffer(&pSession->connectState.handshakeSendBuffer, NULL, lDataSize);
	}

	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLReadHShakeBuffered
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					HandshakeType hType - (IN) The handshake message type
					UINT8 *pData - (OUT) The pointer where handshake data is copied
					INT32 lDataSize - (IN) The data size
					INT32 lPeek -(IN) whether to peek the data or remove the data from 
					the handshake read buffer
* Return Value	:	INT32 - The lDataSize on Success
							SSL_CONNECTION_CLOSED(0) on TCP connection closed
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case this operation would block on 
							socket
* Description	:	This function reads the requested no. of bytes from SSL record layer 
					and the data is not removed from the handshake read buffer in 
					case of peek.
******************************************************************************************/
static INT32 SSLReadHShakeBuffered(SSL *pSession, HandshakeType hType, UINT8 *pData, 
							INT32 lDataSize, INT32 lPeek)
{
	INT32	lBufferSize;
	UINT8	*pReadBuffer;
	INT32	lReadSize;
	INT32	lDataLeft;
	INT32	lReturnVal;
	INT32	lReturnFlag;

	/* get the size of handshake recv buffer */
	lBufferSize = SSLGetBufferSize(&pSession->connectState.handshakeRecvBuffer);

	if(lBufferSize >= lDataSize) {
		/* in case the required data is less than or equal to the buffered data ,
		send data in the buffer */
		if(lPeek == SSL_NO_PEEK) {
			SSLRetrieveBuffer(&pSession->connectState.handshakeRecvBuffer,pData,lDataSize);
		} else {
			SSLPeekBuffer(&pSession->connectState.handshakeRecvBuffer,pData,lDataSize);
		}
		return lDataSize;
	} else {
		/* in case the required data is greater than buffered data ,
		read more data from record layer */

		/* calculate the read size based on existing recv buffer */
		pReadBuffer = &pData[lBufferSize];
		lReadSize = lDataSize-lBufferSize;

		lDataLeft = lReadSize;
		lReturnFlag = SSL_FALSE;

		/* read until required data is read, and return in case of socket errors */
		while(lDataLeft > 0) {
			/* reads data from record layer */
			lReturnVal = SSLRecordRecv(pSession, HANDSHAKE, hType,  
				&pReadBuffer[lReadSize-lDataLeft], lDataLeft);
			if(lReturnVal <= 0) {
				if(lReturnVal == SSL_WOULD_BLOCK) {
					/* in case socket would block, insert whatever is read and return */
					if(!SSLInsertBuffer(&pSession->connectState.handshakeRecvBuffer,pReadBuffer,
						lReadSize - lDataLeft)) {
						lReturnVal = SSL_ERROR;
					}
				}
				lReturnFlag = SSL_TRUE;
				break;
			}
			/* decrement the data to be read based on this number of bytes read now */
			lDataLeft -= lReturnVal;
		}

		if(lReturnFlag) {
			return lReturnVal;
		}
		

		if(lBufferSize != 0) {
			/* get the remaining data from the buffer, in case of peek just leave 
			the data in recv buffer */
			if(lPeek == SSL_NO_PEEK) {
				SSLRetrieveBuffer(&pSession->connectState.handshakeRecvBuffer,pData,
					pSession->connectState.handshakeRecvBuffer.ulSize);
			} else {
				SSLPeekBuffer(&pSession->connectState.handshakeRecvBuffer,pData,
					pSession->connectState.handshakeRecvBuffer.ulSize);
			}
		}

		if(lPeek == SSL_PEEK) {
			/* in case of peek insert all the data that is read from record layer */
			if(!SSLInsertBuffer(&pSession->connectState.handshakeRecvBuffer,pReadBuffer,
				lReadSize)) {
				return SSL_ERROR;
			}
		}
		return lDataSize;
	}
}

/*****************************************************************************************
* Function Name	:	SSLGenerateFinished
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 **pHData - (OUT) The generated handshake finished message
					INT32 *pHDataSize - (OUT) The handshake finished message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates handshake finished message for this session
******************************************************************************************/
static INT32 SSLGenerateFinished(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize)
{
	UINT8 *pData;
	INT32 lDataSize;
	INT32 lRetVal;

	if(pSession->sessionCache.version == SSL_VERSION3) {
		lDataSize = SSL3_FINISHED_SIZE + HANDSHAKE_HEADER_SIZE;

		/* allocates memory for finished handshake message */
		pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lDataSize);
		if(pData == NULL) {
			return SSL_FAILURE;
		}

		/* generate SSL v3 finished message */
		lRetVal = SSLv3Finished(pSession, SSL_SERVER, &pData[HANDSHAKE_HEADER_SIZE]);
	} else {
		lDataSize = TLS_FINISHED_SIZE + HANDSHAKE_HEADER_SIZE;
		
		/* allocates memory for finished handshake message */
		pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lDataSize);
		if(pData == NULL) {
			return SSL_FAILURE;
		}

		/* generate TLS v1 finished message */
		lRetVal = SSLTlsv1Finished(pSession, SSL_SERVER, &pData[HANDSHAKE_HEADER_SIZE]);
	}

	if(lRetVal != SSL_SUCCESS) {
		OS_ADPT_Deallocate_Memory(pData);
		return SSL_FAILURE;
	} else {
		*pHData = pData;
		*pHDataSize = lDataSize;
		return SSL_SUCCESS;
	}
}

/*****************************************************************************************
* Function Name	:	SSLGenerateCertificate
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 **pHData - (OUT) The generated handshake certificate message
					INT32 *pHDataSize - (OUT) The handshake certificate message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates handshake certificate message for this session
******************************************************************************************/
static INT32 SSLGenerateCertificate(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize)
{
	UINT8 *pData;
	INT32 lDataSize;
	INT32 lCertSize;

	/* gets the size of the certificate */
	lCertSize = CertGetSize(pSession->sessionCache.pCipherMapping->certType);

	lDataSize = lCertSize + HANDSHAKE_HEADER_SIZE + CERTIFICATE_MSG_LEN;

	/* allocate memory for certificate handshake message */
	pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lDataSize);

	if(pData == NULL) {
		return SSL_FAILURE;
	}

	/* set the certificate list length */
	SSLWriteUint24(lCertSize, &pData[HANDSHAKE_HEADER_SIZE]);

	if(lCertSize != 0) {
		/* get ASN.1 encoded certificate */
		CertGetCertificate(pSession->sessionCache.pCipherMapping->certType, 
			&pData[HANDSHAKE_HEADER_SIZE+CERTIFICATE_MSG_LEN]);
	}
	*pHData = pData;
	*pHDataSize = lDataSize;
	return SSL_SUCCESS;
}


/*****************************************************************************************
* Function Name	:	SSLGenerateServerHello
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 **pHData - (OUT) The generated handshake server hello message
					INT32 *pHDataSize - (OUT) The handshake server hello message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates handshake server hello message for this session
******************************************************************************************/
static INT32 SSLGenerateServerHello(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize)
{
	INT32 lDataSize;
	UINT8 *pData;
	INT32 lDataPtr;

	/* implements 
     version + random + session id + cipher_suite + compression method
	 */

	/* calculate the server hello handshake message size */
	lDataSize = HANDSHAKE_HEADER_SIZE + PROTOCOL_VERSION_SIZE + 
		pSession->secParams.sessionId.ulSize + SESSION_ID_LENGTH_SIZE + 
		SSL_RANDOM_SIZE + SSL_CIPHER_SIZE + SSL_COMP_SIZE;

	/* allocate memory for server hello handshake message */
	pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lDataSize);

	if(pData == NULL) {
		return SSL_FAILURE;
	}

	lDataPtr = HANDSHAKE_HEADER_SIZE;

	/* fills up the version fields in the message */
	if(pSession->sessionCache.version == SSL_VERSION3) {
		pData[lDataPtr++] = SSL3_VER_MAJOR;
		pData[lDataPtr++] = SSL3_VER_MINOR;

	} else if(pSession->sessionCache.version == TLS_VERSION1) {
		pData[lDataPtr++] = TLS1_VER_MAJOR;
		pData[lDataPtr++] = TLS1_VER_MINOR;
	}

	/* generate the server random value */
	SSLRand(pSession->secParams.serverRandom,SSL_RANDOM_SIZE);
	memcpy(&pData[lDataPtr], pSession->secParams.serverRandom, SSL_RANDOM_SIZE);

	lDataPtr += SSL_RANDOM_SIZE;

	/* set the session id length and session ID */
	pData[lDataPtr++] = (UINT8)pSession->secParams.sessionId.ulSize;

	if (pSession->secParams.sessionId.ulSize > 0) {
		memcpy(&pData[lDataPtr], pSession->secParams.sessionId.pData, 
			pSession->secParams.sessionId.ulSize);
		lDataPtr += pSession->secParams.sessionId.ulSize;
	}

	/* sets the selected cipher suite */
	memcpy(&pData[lDataPtr], pSession->sessionCache.pCipherMapping->cipherSuite, 
		SSL_CIPHER_SIZE);
	lDataPtr += SSL_CIPHER_SIZE;

	/* sets compression to no compresssion */
	pData[lDataPtr] = NULL_COMPRESSION;

	*pHData = pData;
	*pHDataSize = lDataSize;
	return SSL_SUCCESS;
}

#if SSL_CFG_REQUEST_CERTIFICATE
/*****************************************************************************************
* Function Name	:	SSLGenerateCertRequest
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 **pHData - (OUT) The generated handshake certificate request
					INT32 *pHDataSize - (OUT) The handshake server hello message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates handshake server hello message for this session
******************************************************************************************/
static INT32 SSLGenerateCertRequest(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize)
{
	INT32 lDataSize;
	UINT8 *pData;
	INT32 lDataPtr;
	UINT8 *pDNList;
	UINT16 DNSize;

	/* implements 
     ClientCertificateType certificate_types<1..2^8-1> + 
     DistinguishedName certificate_authorities<3..2^16-1>
	*/

	CertGetDNList(&pDNList, &DNSize);

	/* calculate the server certificate request handshake message size */
	lDataSize = HANDSHAKE_HEADER_SIZE + 3 + SSL_DN_LEN_SIZE + DNSize;

	/* allocate memory for server certificate request handshake message */
	pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lDataSize);

	if(pData == NULL) {
		return SSL_FAILURE;
	}

	lDataPtr = HANDSHAKE_HEADER_SIZE;
	pData[lDataPtr++] = 2;	/* 2 certificates supported */
	pData[lDataPtr++] = RSA_SIGN;
	pData[lDataPtr++] = DSS_SIGN;

	/* copies the distinguished name size */
	SSLWriteUint16(DNSize, &pData[lDataPtr]);
	lDataPtr += SSL_DN_LEN_SIZE;

	if(DNSize) {
		memcpy(&pData[lDataPtr], pDNList, DNSize);
	}

	*pHData = pData;
	*pHDataSize = lDataSize;
	return SSL_SUCCESS;
}


/*****************************************************************************************
* Function Name	:	SSLProcessCertificate
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pHData - (IN) The handshake certificate message
					INT32 lDataSize - (IN) The handshake certificate message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function processes handshake certificate message for this session
******************************************************************************************/
static INT32 SSLProcessCertificate(SSL *pSession, UINT8 *pHData, INT32 lDataSize)
{
	INT32 lRetVal;
	INT32 lCertListLength;
	INT32 lFirstCertLength;
	INT32 lLength;
	INT32 lDataPtr;

	pSession->connectState.pClientCert = NULL;

#if SSL_CFG_DEBUG_ENABLE
	SSL_DEBUG("\n Received a Client Certificate message");
#endif

	/* if certificate validation is not mandatory just return without processing */
	if(!RUNTIME_CLIENTCERT_MANDATORY) {
		lLength = lDataSize;
		DECR_VALIDATE_LEN(lLength, CERTIFICATE_MSG_LEN);
		
		lCertListLength = SSLReadUint24(pHData);
		lDataPtr = CERTIFICATE_MSG_LEN;
		
		if(lLength != lCertListLength) {
			return SSL_FAILURE;
		}

		if(lCertListLength == 0) {
			return SSL_SUCCESS;
		}

		DECR_VALIDATE_LEN(lLength, CERTIFICATE_MSG_LEN);

		lFirstCertLength = SSLReadUint24(&pHData[lDataPtr]);
		lDataPtr += CERTIFICATE_MSG_LEN;

		if(lFirstCertLength > lLength || lFirstCertLength == 0) {
			return SSL_FAILURE;
		}


		pSession->connectState.pClientCert = (struct Certificate *)OS_ADPT_Allocate_Memory(SSL_Memory,
			sizeof(struct Certificate));
		if(pSession->connectState.pClientCert == NULL) {
			return SSL_FAILURE;
		}
		memset(pSession->connectState.pClientCert, 0, sizeof(struct Certificate));
		if(DecodeCertData(pSession->connectState.pClientCert, &pHData[lDataPtr], lFirstCertLength) == CERT_SUCCESS) {
			return SSL_SUCCESS;
		} else {
			FreeCert(pSession->connectState.pClientCert);
			pSession->connectState.pClientCert = NULL;
			return SSL_FAILURE;
		}
	}

	if(lDataSize <= CERTIFICATE_MSG_LEN) {
		/* if the certificate message length is less than or actual length or if there is
		no certificate return error */
		lRetVal = SSL_ERROR;
	} else {
		lCertListLength = SSLReadUint24(pHData);
		if(lCertListLength == (lDataSize - CERTIFICATE_MSG_LEN)) {
			/* validate the certificate */
			lRetVal = CertValidateCertificate(pHData+CERTIFICATE_MSG_LEN, 
				lDataSize - CERTIFICATE_MSG_LEN, &pSession->connectState.pClientCert);
		} else {
			/* the length encoded is wrong */
			lRetVal = SSL_ERROR;
		}
	}

	/* send appropriate alerts in case of errors */
	switch(lRetVal) {
	case BAD_CERTIFICATE:
		SSLSendAlert(pSession, AL_FATAL, BAD_CERTIFICATE_ALERT);
		lRetVal = SSL_FAILURE;
		break;

	case CERTIFICATE_REVOKED:
		SSLSendAlert(pSession, AL_FATAL, CERTIFICATE_REVOKED_ALERT);
		lRetVal = SSL_FAILURE;
		break;

	case UNSUPPORTED_CERTIFICATE:
		SSLSendAlert(pSession, AL_FATAL, UNSUPPORTED_CERTIFICATE_ALERT);
		lRetVal = SSL_FAILURE;
		break;

	case CERTIFICATE_EXPIRED:
		SSLSendAlert(pSession, AL_FATAL, CERTIFICATE_EXPIRED_ALERT);
		lRetVal = SSL_FAILURE;
		break;

	case UNKNOWN_CA:
		SSLSendAlert(pSession, AL_FATAL, UNKNOWN_CA_ALERT);
		lRetVal = SSL_FAILURE;
		break;

	case CERT_SUCCESS:
		if(pSession->connectState.pClientCert) {
			lRetVal = SSL_SUCCESS;
		} else {
			lRetVal = SSL_FAILURE;
		}
		break;
	
	case CERTIFICATE_UNKNOWN:
	
		SSLSendAlert(pSession, AL_FATAL, CERTIFICATE_UNKNOWN_ALERT);
		lRetVal = SSL_FAILURE;
		break;

	case SSL_ERROR:
	default:
		SSLSendAlert(pSession, AL_FATAL, HANDSHAKE_FAILURE_ALERT);
		lRetVal = SSL_FAILURE;
		break;

	}

	return lRetVal;
}

/*****************************************************************************************
* Function Name	:	SSLProcessCertVerify
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pHData - (IN) The handshake certificate verify message
					INT32 lDataSize - (IN) The handshake certificate verify message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function processes handshake cert verify message for this session
******************************************************************************************/
static INT32 SSLProcessCertVerify(SSL *pSession, UINT8 *pHData, INT32 lDataSize)
{
	INT32 lLength;
	INT32 lRetVal;
	UINT16 SignatureLen;

#if SSL_CFG_DEBUG_ENABLE
	SSL_DEBUG("\n Received a Certificate verify message");
#endif

	if(pSession->connectState.pClientCert == NULL) {
		return SSL_SUCCESS;
	}

	lLength = lDataSize;

	DECR_VALIDATE_LEN(lLength, SSL_SIGNATURE_LEN_SIZE);

	SignatureLen = SSLReadUint16(pHData);
	DECR_VALIDATE_LEN(lLength, SignatureLen);

	lRetVal = SSLVerifySignature(pSession, &pHData[SSL_SIGNATURE_LEN_SIZE], SignatureLen);

	FreeCert(pSession->connectState.pClientCert);
	pSession->connectState.pClientCert = NULL;

	return lRetVal;
}

static void SSLGetCertVerifyDigest(SSL *pSession, UINT8 *pDigest)
{
	MD5_CTX md5;
	SHA_CTX sha;
	UINT8 pad1[SSL3_MAC_MD5_PAD_SIZE];
	UINT8 pad2[SSL3_MAC_MD5_PAD_SIZE];

	/* implements 
		md5_hash[16] + sha_hash[20]
		
		md5_hash       MD5(master_secret + pad2 +
							MD5(handshake_messages + 
								master_secret + pad1));
		sha_hash       SHA(master_secret + pad2 + 
			                SHA(handshake_messages + 
								master_secret + pad1));

		pad_1			The character 0x36 repeated 48 times for MD5 or 40 times for SHA.
		pad_2           The character 0x5c repeated 48 times for MD5 or 40 times for SHA.

	*/

	/* copying the hash of handshake messages to the local hash buffers */

	if(pSession->sessionCache.version == SSL_VERSION3) {
		memcpy(&sha, &pSession->connectState.hShakeSha1Hash, sizeof(SHA_CTX));

		/* setting pad1 and pad2 */
		memset(pad1,0x36,SSL3_MAC_MD5_PAD_SIZE);
		memset(pad2,0x5c,SSL3_MAC_MD5_PAD_SIZE);


		/* takes first SHA1 hash */
		SHA1_Update(&sha,pSession->sessionCache.masterSecret,MASTER_SECRET_SIZE);
		SHA1_Update(&sha,pad1,SSL3_MAC_SHA1_PAD_SIZE);
		SHA1_Final(&pDigest[SSL_MD5_DIGEST_LEN], &sha);


		/* takes second SHA1 hash */
		SHA1_Init(&sha);
		SHA1_Update(&sha, pSession->sessionCache.masterSecret,MASTER_SECRET_SIZE);
		SHA1_Update(&sha, pad2, SSL3_MAC_SHA1_PAD_SIZE);
		SHA1_Update(&sha, &pDigest[SSL_MD5_DIGEST_LEN], SSL_SHA1_DIGEST_LEN);


		if(pSession->connectState.pClientCert->Algorithm == ALGO_RSA) {
		
			memcpy(&md5, &pSession->connectState.hShakeMd5Hash, sizeof(MD5_CTX));
			
			/* takes first MD5 hash */
			MD5_Update(&md5,pSession->sessionCache.masterSecret,MASTER_SECRET_SIZE);
			MD5_Update(&md5, pad1, SSL3_MAC_MD5_PAD_SIZE);
			MD5_Final(pDigest, &md5);

			/* takes second MD5 hash */
			MD5_Init(&md5);
			MD5_Update(&md5, pSession->sessionCache.masterSecret,MASTER_SECRET_SIZE);
			MD5_Update(&md5, pad2, SSL3_MAC_MD5_PAD_SIZE);
			MD5_Update(&md5, pDigest, SSL_MD5_DIGEST_LEN);
			MD5_Final(pDigest, &md5);

			SHA1_Final(&pDigest[SSL_MD5_DIGEST_LEN], &sha);

		} else {
			SHA1_Final(pDigest, &sha);
		}
	} else {
		/* copying the SHA1 hash of handshake messages to the local hash buffer */
		memcpy(&sha, &pSession->connectState.hShakeSha1Hash, sizeof(SHA_CTX));

		if(pSession->connectState.pClientCert->Algorithm == ALGO_RSA) {
			/* copying the md5 hash of handshake messages to the local hash buffer */
			memcpy(&md5, &pSession->connectState.hShakeMd5Hash, sizeof(MD5_CTX));
			/* finalising the SHA1 and MD5 hashes of handshake messages */
			MD5_Final(pDigest, &md5);
			SHA1_Final(&pDigest[SSL_MD5_DIGEST_LEN], &sha);
		} else {
				SHA1_Final(pDigest, &sha);
		}
	}
}

/*****************************************************************************************
* Function Name	:	SSLVerifySignature
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pSignature - (IN) The signature to be verified
					INT32 lSignatureLen - (IN) The signature length
* Return Value	:	INT32 - SSL_SUCCESS(1) on successful verification
						  - SSL_FAILURE(0) on error
* Description	:	This function verifies a RSA or DSA signatures based on key. 
******************************************************************************************/
static INT32 SSLVerifySignature(SSL *pSession, UINT8 *pSignature, INT32 lSignatureLen)
{
	UINT8 digest[SSL_SHA1_DIGEST_LEN+SSL_MD5_DIGEST_LEN];
	INT32 lRetVal;
#if SSL_CFG_DSS_ENABLED
	DSA *pDSA;
#endif /* SSL_CFG_DSS_ENABLED */

#if SSL_CFG_RSA_EXPORT_ENABLED
	RSA *pRSA;
#endif /* SSL_CFG_RSA_EXPORT_ENABLED */

	switch(pSession->connectState.pClientCert->Algorithm) {
#if SSL_CFG_DSS_ENABLED
		case ALGO_DSA:
			/* creates new DSA object */
			pDSA = DSA_new();
			if(pDSA == NULL) {
				return SSL_FAILURE;
			}

			/* get DSA certificate params */
			pDSA->p = pSession->connectState.pClientCert->pDSAP;
			pDSA->q = pSession->connectState.pClientCert->pDSAQ;
			pDSA->g = pSession->connectState.pClientCert->pDSAG;
			pDSA->pub_key = pSession->connectState.pClientCert->pDSAPub;
			
			SSLGetCertVerifyDigest(pSession,digest);
			
			/* makes a DSA signature */
			lRetVal = DSA_verify(NID_sha1,digest,SSL_SHA1_DIGEST_LEN, pSignature, 
				lSignatureLen, pDSA);


			pDSA->p = NULL;
			pDSA->q = NULL;
			pDSA->g = NULL;
			pDSA->pub_key = NULL;
			DSA_free(pDSA);

			if(lRetVal) {
				return SSL_SUCCESS;
			} else {
				return SSL_FAILURE;
			}
			break;
#endif /* SSL_CFG_DSS_ENABLED */

#if (SSL_CFG_RSA_EXPORT_ENABLED | SSL_CFG_RSA_ENABLED)
		case ALGO_RSA:
			/* creates a new RSA object */
			pRSA = RSA_new();
			if(pRSA == NULL) {
				return SSL_FAILURE;
			}

			pRSA->n = pSession->connectState.pClientCert->pModulus;
			pRSA->e = pSession->connectState.pClientCert->pPublicExponent;

			SSLGetCertVerifyDigest(pSession,digest);
			
			/* verifies RSA signature */
			lRetVal = RSA_verify(NID_md5_sha1,digest,SSL_SHA1_DIGEST_LEN+SSL_MD5_DIGEST_LEN, 
				pSignature, lSignatureLen, pRSA);

			pRSA->n = NULL;
			pRSA->e = NULL;
			RSA_free(pRSA);

			if(lRetVal) {
				return SSL_SUCCESS;
			} else {
				return SSL_FAILURE;
			}
			break;
#endif /* (SSL_CFG_RSA_EXPORT_ENABLED | SSL_CFG_RSA_ENABLED) */

		default:
			return SSL_FAILURE;
	}
}

#endif /* SSL_CFG_REQUEST_CERTIFICATE */

/*****************************************************************************************
* Function Name	:	SSLProcessFinished
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pHData - (IN) The handshake finished message
					INT32 lDataSize - (IN) The handshake finished message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function processes handshake finished message for this session
******************************************************************************************/
static INT32 SSLProcessFinished(SSL *pSession, UINT8 *pHData, INT32 lDataSize)
{
	UINT8 data[SSL3_FINISHED_SIZE];
	INT32 lDataLength;
	INT32 lRetVal;

	/* validates the finished message received */
	if(pSession->sessionCache.version == SSL_VERSION3) {
		lDataLength = SSL3_FINISHED_SIZE;
		if(lDataSize != lDataLength) {
			return SSL_FAILURE;
		}

		/* generate the v3 finished */
		lRetVal = SSLv3Finished(pSession, SSL_CLIENT, data);
	} else {
		lDataLength = TLS_FINISHED_SIZE;
		if(lDataSize != lDataLength) {
			return SSL_FAILURE;
		}

		/* generate TLS v1 finished */
		lRetVal = SSLTlsv1Finished(pSession, SSL_CLIENT, data);
	}

	if(lRetVal != SSL_SUCCESS) {
		return SSL_FAILURE;
	}

	/* check the finsihed data received and generated are same */
	if(memcmp(pHData, data, lDataLength) != 0) {
		return SSL_FAILURE;
	} else {
		return SSL_SUCCESS;
	}
}

/*****************************************************************************************
* Function Name	:	SSLProcessV2ClientHello
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pHData - (IN) The handshake SSL v2 client hello message
					INT32 lDataSize - (IN) The handshake SSL v2 client hello message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function processes handshake SSL v2 client hello message 
******************************************************************************************/
static INT32 SSLProcessV2ClientHello(SSL *pSession, UINT8 *pHData, INT32 lDataSize)
{
	INT32 lLength;
	INT32 lDataPtr;
	UINT16 cipherSpecLength;
	UINT16 sessionIDLength;
	UINT16 challengeLength;
	UINT8 *pCipherPtr;
	
	lDataPtr = 0;
	lLength = lDataSize;

#if SSL_CFG_DEBUG_ENABLE
	SSL_DEBUG("\n\n Received a SSL v2 Client Hello");
#endif

	/* implements processiong of 
	msg_type(1) + version(2) + cipher_spec_length(2) + session_id_length(2) + 
	challenge_length(2) + cipher_specs[cipher_spec_length] + session_id[session_id_length] + 
	+ challenge
	*/

	DECR_VALIDATE_LEN(lLength, SSL_MSG_TYPE_LEN);

	/* check the message type is client hello or not */
	if(pHData[lDataPtr++] != CLIENT_HELLO) {
		return SSL_FAILURE;
	}

	DECR_VALIDATE_LEN(lLength, PROTOCOL_VERSION_SIZE);
	
	/* check the versions supported */
	if(pHData[lDataPtr] == SSL3_VER_MAJOR &&
		pHData[lDataPtr+1] == SSL3_VER_MINOR && RUNTIME_VERSION_CHECK(SSL_VERSION3)) 
	{
		/**** BEGIN - Added by YKS - 05/04/2004 ****/
		pSession->sessionCache.clientVersion = SSL_VERSION3;
		/**** END - Added by YKS - 05/04/2004 ****/

		pSession->sessionCache.version = SSL_VERSION3;
	} 
	else if (pHData[lDataPtr] == TLS1_VER_MAJOR &&
		pHData[lDataPtr+1] == TLS1_VER_MINOR) 
	{
		/**** BEGIN - Added by YKS - 05/04/2004 ****/
		pSession->sessionCache.clientVersion = TLS_VERSION1;
		/**** END - Added by YKS - 05/04/2004 ****/

		if(RUNTIME_VERSION_CHECK(TLS_VERSION1)) 
		{
			pSession->sessionCache.version = TLS_VERSION1;
		} 
		else if(RUNTIME_VERSION_CHECK(SSL_VERSION3)) 
		{
			pSession->sessionCache.version = SSL_VERSION3;
		} 
		else 
		{
			SSLSendAlert(pSession, AL_FATAL, PROTOCOL_VERSION_ALERT);
			return SSL_FAILURE;
		}
	} 
	else 
	{
		SSLSendAlert(pSession, AL_FATAL, PROTOCOL_VERSION_ALERT);
		return SSL_FAILURE;
	}

	lDataPtr += PROTOCOL_VERSION_SIZE;

	/* read the cipher spec length */
	DECR_VALIDATE_LEN(lLength, SSL2_LENGTH_SIZE);
	cipherSpecLength = SSLReadUint16(&pHData[lDataPtr]);
	lDataPtr += SSL2_LENGTH_SIZE;
	if(cipherSpecLength == 0 || cipherSpecLength%SSL2_CIPHERSPEC_SIZE != 0) {
		return SSL_FAILURE;
	}

	/* read the session id length */
	DECR_VALIDATE_LEN(lLength, SSL2_LENGTH_SIZE);
	sessionIDLength = SSLReadUint16(&pHData[lDataPtr]);
	lDataPtr += SSL2_LENGTH_SIZE;

	if(sessionIDLength != 0 && sessionIDLength != SSL2_MAX_SESSIONID_LEN) {
		return SSL_FAILURE;
	}

	/* read the challenge length */
	DECR_VALIDATE_LEN(lLength, SSL2_LENGTH_SIZE);
	challengeLength = SSLReadUint16(&pHData[lDataPtr]);
	lDataPtr += SSL2_LENGTH_SIZE;

	if(challengeLength < SSL2_MIN_CHALLENGE_LEN) {
		return SSL_FAILURE;
	}

	DECR_VALIDATE_LEN(lLength, cipherSpecLength);
	pCipherPtr = &pHData[lDataPtr];
	lDataPtr += cipherSpecLength;

	DECR_VALIDATE_LEN(lLength, sessionIDLength);
	lDataPtr += sessionIDLength;

	/* if server sessions are cached generate a session id */
	if(SSLIsServerCaching(pSession)) {
		pSession->secParams.sessionId.pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,
			SSL_MAX_SESSION_ID_SIZE);
		if(pSession->secParams.sessionId.pData == NULL) {
			return SSL_FAILURE;
		}
		pSession->secParams.sessionId.ulSize = SSL_MAX_SESSION_ID_SIZE;
		pSession->connectState.resumed = RESUME_FALSE;
		/* generate a random session id */
		SSLRand(pSession->secParams.sessionId.pData,SSL_MAX_SESSION_ID_SIZE);
	}

	DECR_VALIDATE_LEN(lLength, challengeLength);

	/* copy the client random */
	if(challengeLength <= SSL_RANDOM_SIZE) {
		memset(pSession->secParams.clientRandom,0,SSL_RANDOM_SIZE);
		memcpy(&pSession->secParams.clientRandom[SSL_RANDOM_SIZE-challengeLength], 
			&pHData[lDataPtr], challengeLength);
	} else {
		lDataPtr += (challengeLength - SSL_RANDOM_SIZE);
		memcpy(pSession->secParams.clientRandom, 
			&pHData[lDataPtr], SSL_RANDOM_SIZE);
	}

	/* select the cipher suite from ssl v2 cipher specs */
	if(SSLSelectV2CipherSuite(pSession, pCipherPtr, cipherSpecLength)) {
		return SSL_SUCCESS;
	} else {
		SSLSendAlert(pSession, AL_FATAL, HANDSHAKE_FAILURE_ALERT);
		return SSL_FAILURE;
	}
}

/*****************************************************************************************
* Function Name	:	SSLProcessClientHello
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					UINT8 *pHData - (IN) The handshake client hello message
					INT32 lDataSize - (IN) The handshake client hello message size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function processes handshake client hello message 
******************************************************************************************/
static INT32 SSLProcessClientHello(SSL *pSession, UINT8 *pHData, INT32 lDataSize)
{
	INT32 lLength;
	INT32 lSessionIDLen;
	INT32 lReturnVal;
	UINT16 cipherSuiteSize;
	UINT8 *pCipherPtr;
	UINT8 compSize;
	INT32 lDataPtr;

	lDataPtr=0;
	lLength = lDataSize;
	DECR_VALIDATE_LEN(lLength, PROTOCOL_VERSION_SIZE);

	/* implements processing of      
	version + random + session_id + cipher_suites<2..2^16-1> + 
	compression_methods<1..2^8-1> */

	/* checks the protocol version */
	if(pHData[lDataPtr] == SSL3_VER_MAJOR &&
		pHData[lDataPtr+1] == SSL3_VER_MINOR && RUNTIME_VERSION_CHECK(SSL_VERSION3)) 
	{
		/**** BEGIN - Added by YKS - 05/04/2004 ****/
		pSession->sessionCache.clientVersion = SSL_VERSION3;
		/**** END - Added by YKS - 05/04/2004 ****/

		pSession->sessionCache.version = SSL_VERSION3;
	} 
	else if (pHData[lDataPtr] == TLS1_VER_MAJOR &&
		pHData[lDataPtr+1] == TLS1_VER_MINOR) 
	{
		/**** BEGIN - Added by YKS - 05/04/2004 ****/
		pSession->sessionCache.clientVersion = TLS_VERSION1;
		/**** END - Added by YKS - 05/04/2004 ****/

		if(RUNTIME_VERSION_CHECK(TLS_VERSION1)) 
		{
			pSession->sessionCache.version = TLS_VERSION1;
		} 
		else if(RUNTIME_VERSION_CHECK(SSL_VERSION3)) 
		{
			pSession->sessionCache.version = SSL_VERSION3;
		} 
		else 
		{
			SSLSendAlert(pSession, AL_FATAL, PROTOCOL_VERSION_ALERT);
			return SSL_FAILURE;
		}
	} 
	else 
	{
		SSLSendAlert(pSession, AL_FATAL, PROTOCOL_VERSION_ALERT);
		return SSL_FAILURE;
	}

	lDataPtr += PROTOCOL_VERSION_SIZE;

	DECR_VALIDATE_LEN(lLength, SSL_RANDOM_SIZE);

	/* copy the client random */
	memcpy(pSession->secParams.clientRandom, &pHData[lDataPtr],SSL_RANDOM_SIZE);
	lDataPtr += SSL_RANDOM_SIZE;


	DECR_VALIDATE_LEN(lLength, SESSION_ID_LENGTH_SIZE);
	lSessionIDLen = pHData[lDataPtr++];

	if (lSessionIDLen > SSL_MAX_SESSION_ID_SIZE) {
		return SSL_FAILURE;
	}

	DECR_VALIDATE_LEN(lLength, lSessionIDLen);
	lReturnVal = SSL_FAILURE;

	if(lSessionIDLen > 0) {
		/* if session id is non-empty, then client wants to restore the older session */
		lReturnVal = SSLRestoreSession(pSession, &pHData[lDataPtr], lSessionIDLen);
	}

	lDataPtr += lSessionIDLen;

	if(lReturnVal == SSL_SUCCESS) {

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n\n Older Session Resumed");
#endif
		/* successfully retrieved cahced session data */
		pSession->connectState.resumed = RESUME_TRUE;
	} else {

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n\n New Session Created");
#endif

		/* session is new, generate session id if session is cached */
		if(SSLIsServerCaching(pSession)) {
			pSession->secParams.sessionId.pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,
				SSL_MAX_SESSION_ID_SIZE);
			if(pSession->secParams.sessionId.pData == NULL) {
				return SSL_FAILURE;
			}
			pSession->secParams.sessionId.ulSize = SSL_MAX_SESSION_ID_SIZE;
			pSession->connectState.resumed = RESUME_FALSE;
			SSLRand(pSession->secParams.sessionId.pData,SSL_MAX_SESSION_ID_SIZE);
		}
	}

	/* read cipher list size */
	DECR_VALIDATE_LEN(lLength, CIPHER_LIST_LENGTH_SIZE);
	cipherSuiteSize = SSLReadUint16(&pHData[lDataPtr]);
	lDataPtr += CIPHER_LIST_LENGTH_SIZE;

	/* read compression list size */
	DECR_VALIDATE_LEN(lLength, cipherSuiteSize);
	pCipherPtr = &pHData[lDataPtr];
	lDataPtr += cipherSuiteSize;

	DECR_VALIDATE_LEN(lLength, COMP_LIST_LENGTH_SIZE);
	compSize = pHData[lDataPtr++];

	DECR_VALIDATE_LEN(lLength, compSize);

	/* check null compression is available in client's list */
	while(compSize) {
		if(pHData[lDataPtr++] == NULL_COMPRESSION) {
			break;
		}
		compSize--;
	}

	if(compSize == 0) {
		/* null compression is not present in client's list */
		return SSL_FAILURE;
	}

	/* if session is not resumed, select a cipher suite */
	if(pSession->connectState.resumed == RESUME_TRUE) {
		return SSL_SUCCESS;
	} else if(SSLSelectCipherSuite(pSession, pCipherPtr, cipherSuiteSize)) {
		return SSL_SUCCESS;
	} else {
		SSLSendAlert(pSession, AL_FATAL, HANDSHAKE_FAILURE_ALERT);
		return SSL_FAILURE;
	}
}

/*****************************************************************************************
* Function Name	:	SSLSendHandshake
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					HandshakeType hType - (IN) The handshake message type
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case the socket would block for this 
							operation
* Description	:	This function sends a specified handshake message for this session
******************************************************************************************/
static INT32 SSLSendHandshake(SSL *pSession, HandshakeType hType)
{
	INT32 lReturnVal;
	INT32 lHDataSize;
	UINT8 *pHData;
	
	/* this function maintains the state of the handshake send buffer. If the previuos
	send did not complete, then it flushes the buffer */

	if(pSession->connectState.handshakeSendBuffer.ulSize != 0) {
		/* flush the previous buffer */
		lReturnVal = SSLWriteHShakeBuffered(pSession, NULL, 0);
		if(lReturnVal < 0) {
			return lReturnVal;
		}
		/* return the previous handshake message size before encryption */
		lReturnVal = pSession->connectState.lhandshakeSendPrevSize;
		pSession->connectState.lhandshakeSendPrevSize = 0;
	} else {
		/* generate a new handshake message of hType and send it */
		switch (hType) {
		
		case SERVER_HELLO:
			
			/* generate a server hello message */
			lReturnVal = SSLGenerateServerHello(pSession, &pHData, &lHDataSize);
			break;

		case CERTIFICATE:
			
			/* generate a server certificate message */
			lReturnVal = SSLGenerateCertificate(pSession, &pHData, &lHDataSize);
			break;


		case SERVERKEYEX:
		
			/* generate a server key exchange message */
			lReturnVal = SSLGenerateServerKeyEx(pSession, &pHData, &lHDataSize);
			break;
		case HELLO_DONE:
		
			/* hello done is an empty handshake message, 
			just send only the handshake header */
			lHDataSize = HANDSHAKE_HEADER_SIZE;
			pHData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,HANDSHAKE_HEADER_SIZE);
			if(pHData != NULL) {
				lReturnVal = SSL_SUCCESS;
			} else {
				lReturnVal = SSL_FAILURE;
			}
			break;

		case FINISHED:
		
			/* generate a finsihed message */
			lReturnVal = SSLGenerateFinished(pSession, &pHData, &lHDataSize);
			break;

		case CERTIFICATE_REQUEST:
#if SSL_CFG_REQUEST_CERTIFICATE
			/* generate a certificate request message */
			lReturnVal = SSLGenerateCertRequest(pSession, &pHData, &lHDataSize);
			break;
#endif
			/* these messages are related to SSL clients */
		case CLIENT_HELLO:
		case CLIENTKEYEX:
		case CERTIFICATE_VERIFY:
		default:
			lReturnVal = SSL_ERROR;
			break;
		}

		
		if(lReturnVal == SSL_SUCCESS) {
			/* fills the handshake header */
			pHData[HSHAKE_TYPE_POS] = hType;
			SSLWriteUint24(lHDataSize-HANDSHAKE_HEADER_SIZE, &pHData[HSHAKE_LENGTH_POS]);

			/* updates the handshake hash */
			SSLHShakeHashUpdate(pSession, hType, pHData, lHDataSize);

			/* send the handshake message to the record layer */
			lReturnVal = SSLWriteHShakeBuffered(pSession, pHData, lHDataSize);

			/* frees the memory allocated within generate functions */
			OS_ADPT_Deallocate_Memory(pHData);
			if(lReturnVal < 0) {
				return lReturnVal;
			}		
		}
		
	}

	if(lReturnVal != SSL_SUCCESS) {
		return SSL_ERROR;
	} else {
		return SSL_SUCCESS;
	}
}


/*****************************************************************************************
* Function Name	:	SSLRecvHandshake
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					HandshakeType hType - (IN) The handshake message type
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_CONNECTION_CLOSED(0) on TCP connection closed
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case this operation would block on 
							socket
* Description	:	This function receives a specified handshake message for this session
******************************************************************************************/
static INT32 SSLRecvHandshake(SSL *pSession, HandshakeType hType)
{
	INT32 lReturnVal;
	UINT8 hShakeHeader[HANDSHAKE_HEADER_SIZE];
	INT32 lLength;
	UINT8 *pHData;
	HandshakeType recvType;

	/* reads the handshake header */
	if((lReturnVal = SSLReadHShakeBuffered(pSession, hType, &hShakeHeader[0], 
		HANDSHAKE_HEADER_SIZE, SSL_PEEK)) != HANDSHAKE_HEADER_SIZE) {
		return lReturnVal;
	}

	if(hType == CLIENT_HELLO && pSession->connectState.v2Hello) {

		/* SSL v2 client hello is received */
		lLength = pSession->connectState.v2Hello;
		pHData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lLength);
		
		if (pHData == NULL) {
			return SSL_ERROR;
		}

		/* read the whole v2 client hello without peek */
		if((lReturnVal = SSLReadHShakeBuffered(pSession, hType, pHData, 
			lLength, SSL_NO_PEEK)) != lLength) {
			OS_ADPT_Deallocate_Memory(pHData);
			return lReturnVal;
		}
	} else {

		recvType = hShakeHeader[HSHAKE_TYPE_POS];

		/* check the received handshake type */
		if(hType != recvType) {
			return SSL_ERROR;
		}

		/* read the handshake message length */
		lLength = SSLReadUint24(&hShakeHeader[HSHAKE_LENGTH_POS]);

		if (lLength > 0) {
			/* allocate memory for whole message + header */
			pHData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lLength+HANDSHAKE_HEADER_SIZE);
		} else if (recvType != HELLO_DONE) {
			return SSL_ERROR;
		}

		if (pHData == NULL && lLength > 0) {
			return SSL_ERROR;
		}

		if (lLength > 0) {
			/* read the whole handshke message and header without peek */
			if((lReturnVal = SSLReadHShakeBuffered(pSession, hType, pHData, 
				lLength+HANDSHAKE_HEADER_SIZE, SSL_NO_PEEK)) != lLength+HANDSHAKE_HEADER_SIZE) {
				OS_ADPT_Deallocate_Memory(pHData);
				return lReturnVal;
			}
		}
	}


	switch (hType) {
	case CLIENT_HELLO:
		if(pSession->connectState.v2Hello) {
			/* reset v2hello flag */
			pSession->connectState.v2Hello = 0;
			/* v2 client hello is received */
			lReturnVal = SSLProcessV2ClientHello(pSession, pHData,lLength);
		} else {
			/* process SSL v3 or TLS v1 client hello */
			lReturnVal = SSLProcessClientHello(pSession, pHData+HANDSHAKE_HEADER_SIZE,
				lLength);
			lLength += HANDSHAKE_HEADER_SIZE;
		}
		break;
	case CLIENTKEYEX:
		/* process client key exchange */
		lReturnVal = SSLProcessClientKeyEx(pSession, pHData+HANDSHAKE_HEADER_SIZE,
			lLength);
		lLength += HANDSHAKE_HEADER_SIZE;
		break;
	case FINISHED:
		/* process finished message */
		lReturnVal = SSLProcessFinished(pSession, pHData+HANDSHAKE_HEADER_SIZE,
			lLength);
		lLength += HANDSHAKE_HEADER_SIZE;
		break;

	case CERTIFICATE:
#if SSL_CFG_REQUEST_CERTIFICATE
		/* process certificate message */
		lReturnVal = SSLProcessCertificate(pSession, pHData+HANDSHAKE_HEADER_SIZE,
			lLength);
		lLength += HANDSHAKE_HEADER_SIZE;
		break;
#endif

	case CERTIFICATE_VERIFY:
#if SSL_CFG_REQUEST_CERTIFICATE
		/* process certificate verify message */
		lReturnVal = SSLProcessCertVerify(pSession, pHData+HANDSHAKE_HEADER_SIZE,
			lLength);
		lLength += HANDSHAKE_HEADER_SIZE;
		break;
#endif

		/* client side messages */
	case SERVER_HELLO:
	case HELLO_DONE:
	case SERVERKEYEX:
	case CERTIFICATE_REQUEST:
	default:
		lReturnVal = SSL_ERROR;
	}

	/* update the handshake hash */
	SSLHShakeHashUpdate(pSession, hType, pHData, lLength);

	/* deallocate the handshake message */
	OS_ADPT_Deallocate_Memory(pHData);

	if(lReturnVal != SSL_SUCCESS) {
		return SSL_ERROR;
	} else {
		return SSL_SUCCESS;
	}
}


/*****************************************************************************************
* Function Name	:	SSLHandshakeFinal
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_CONNECTION_CLOSED(0) on TCP connection closed
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case this operation would block on 
							socket
* Description	:	This function performs sending and receiving of handshake finsihed
					and change cipher spec messages
******************************************************************************************/
static INT32 SSLHandshakeFinal(SSL *pSession)
{
	INT32 lReturnVal;

	/* for resumed and unresumed sessions the order of finished and change cipher spec
	messages differs */

	if (pSession->connectState.resumed == RESUME_FALSE) {
		if(pSession->connectState.hsState == RECV_CHANGE_CIPHER_STATE) {
			/* receives change cipher spec message */
			lReturnVal = SSLRecvChangeCipherSpec(pSession);
			CHECK_RETURN_VALUE(lReturnVal);
			
#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Received Chnage Cipher Spec Successfully");
#endif

			/* initialise read state, starts decryption */
			SSLReadConnectionStateInit(pSession);
			pSession->connectState.hsState = RECV_FINISHED_STATE;
		}
	

		if(pSession->connectState.hsState == RECV_FINISHED_STATE) {
			/* receives finished message */
			lReturnVal = SSLRecvHandshake(pSession,FINISHED);
			CHECK_RETURN_VALUE(lReturnVal);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Received Finished Message Successfully");
#endif

			pSession->connectState.hsState = SEND_CHANGE_CIPHER_STATE;
		}

		if(pSession->connectState.hsState == SEND_CHANGE_CIPHER_STATE) {
			/* sends chnage cipher spec messages */
			lReturnVal = SSLSendChangeCipherSpec(pSession);
			CHECK_RETURN_VALUE(lReturnVal);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Sent Change Cipher Spec Successfully");
#endif
			/* initialise write state, starts encryption */
			SSLWriteConnectionStateInit(pSession);
			pSession->connectState.hsState = SEND_FINISHED_STATE;
		}

		if(pSession->connectState.hsState == SEND_FINISHED_STATE) {
			/* sends finished message */
			lReturnVal = SSLSendHandshake(pSession, FINISHED);
			CHECK_RETURN_VALUE(lReturnVal);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Sent Finished Message Successfully");
#endif

		}
	} else {

		if(pSession->connectState.hsState == SEND_CHANGE_CIPHER_STATE) {
			/* sends chnage cipher spec messages */
			lReturnVal = SSLSendChangeCipherSpec(pSession);
			CHECK_RETURN_VALUE(lReturnVal);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Sent Change Cipher Spec Successfully");
#endif
			/* initialise write state, starts encryption */
			SSLWriteConnectionStateInit(pSession);
			pSession->connectState.hsState = SEND_FINISHED_STATE;
		}
	
		if(pSession->connectState.hsState == SEND_FINISHED_STATE) {
			/* sends finished message */
			lReturnVal = SSLSendHandshake(pSession, FINISHED);
			CHECK_RETURN_VALUE(lReturnVal);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Sent Finished Message Successfully");
#endif

			pSession->connectState.hsState = RECV_CHANGE_CIPHER_STATE;
		}

		if(pSession->connectState.hsState == RECV_CHANGE_CIPHER_STATE) {
			/* receives change cipher spec message */
			lReturnVal = SSLRecvChangeCipherSpec(pSession);
			CHECK_RETURN_VALUE(lReturnVal);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Received Chnage Cipher Spec Successfully");
#endif

			/* initialise read state, starts decryption */
			SSLReadConnectionStateInit(pSession);
			pSession->connectState.hsState = RECV_FINISHED_STATE;
		}
	

		if(pSession->connectState.hsState == RECV_FINISHED_STATE) {
			/* receives finished message */
			lReturnVal = SSLRecvHandshake(pSession,FINISHED);
			CHECK_RETURN_VALUE(lReturnVal);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Received Finished Message Successfully");
#endif
		}
	}
	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLServerHandshake
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_CONNECTION_CLOSED(0) on TCP connection closed
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case this operation would block on 
							socket
* Description	:	This function performs SSL Server handshake over the specified TCP
					connection.
******************************************************************************************/
INT32 SSLServerHandshake(SSL *pSession)
{
	INT32	lReturnValue;

	/* this function performs the functinality of the SSL server handshake. When to send 
	and recv appropriate handshake message is controlled here. In case of SSL_WOULD_BLOCK 
	the same state is maintained and the data is maintained in the handshake send and 
	recv buffers. So this function works when this is called again and again when it returns
	SSL_WOULD_BLOCK error. In this case it starts from the state where it left */

	if (pSession->connectState.hsState == INIT_STATE) {
		/* receive a client hello handshake */
		lReturnValue  = SSLRecvHandshake(pSession, CLIENT_HELLO);
		CHECK_RETURN_VALUE(lReturnValue);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Client Hello Received Successfully");
#endif

		pSession->connectState.hsState = SERVERHELLO_STATE;
	}

	if(pSession->connectState.hsState == SERVERHELLO_STATE) {
		/* sends a server hello handshake */
		lReturnValue  = SSLSendHandshake(pSession, SERVER_HELLO);
		CHECK_RETURN_VALUE(lReturnValue);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Server Hello Sent Successfully");
#endif

		if(pSession->connectState.resumed == RESUME_TRUE) {
			/* in case of resumed sessions move to change cipher state */
			pSession->connectState.hsState = SEND_CHANGE_CIPHER_STATE;

			/* generate the session keys */
			if(!SSLGenerateSessionKeys(pSession)) {
				return SSL_ERROR;
			}
		} else {
			/* session is not resumed, key exchange is required */
			if(SSLServerCertNeeded(pSession) == SSL_TRUE) {
				/* certificate message is needed */
				pSession->connectState.hsState = CERTIFICATE_STATE;
			} else {
				/* certificate not required, directly move to server key exchange */
				pSession->connectState.hsState = SERVERKEYEX_STATE;
			}
		}
	}

	if(pSession->connectState.hsState == CERTIFICATE_STATE)
	{
		/* sends certificate key exchange message */
		lReturnValue  = SSLSendHandshake(pSession, CERTIFICATE);
		CHECK_RETURN_VALUE(lReturnValue);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Server Certificate Sent Successfully");
#endif
		/* check if server key exchange is needed, else move to hello done state */
		if(SSLServerKeyExNeeded(pSession) == SSL_TRUE) {
			pSession->connectState.hsState = SERVERKEYEX_STATE;
		} else {
#if SSL_CFG_REQUEST_CERTIFICATE
			if(RUNTIME_REQUEST_CERT) {
				pSession->connectState.hsState = REQUEST_CERT_STATE;
			} else 
#endif
			{
				pSession->connectState.hsState = HELLODONE_STATE;
			}
		}
	}

	if(pSession->connectState.hsState == SERVERKEYEX_STATE) {
		/* sends a server key exchange message */
		lReturnValue  = SSLSendHandshake(pSession, SERVERKEYEX);
		CHECK_RETURN_VALUE(lReturnValue);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Server Key Exchange Sent Successfully");
#endif
		
#if SSL_CFG_REQUEST_CERTIFICATE
			if(RUNTIME_REQUEST_CERT) {
				pSession->connectState.hsState = REQUEST_CERT_STATE;
			} else 
#endif
			{
				pSession->connectState.hsState = HELLODONE_STATE;
			}
	}

#if SSL_CFG_REQUEST_CERTIFICATE
	if(pSession->connectState.hsState == REQUEST_CERT_STATE) {
		/* sends a server key exchange message */
		lReturnValue  = SSLSendHandshake(pSession, CERTIFICATE_REQUEST);
		CHECK_RETURN_VALUE(lReturnValue);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Certificate Request Sent Successfully");
#endif
		pSession->connectState.certRequested = SSL_TRUE;
		pSession->connectState.hsState = HELLODONE_STATE;
	}
#endif /* SSL_CFG_REQUEST_CERTIFICATE */

	if(pSession->connectState.hsState == HELLODONE_STATE) {
		/* sends a hello done message */
		lReturnValue  = SSLSendHandshake(pSession, HELLO_DONE);
		CHECK_RETURN_VALUE(lReturnValue);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Server Hello Done Sent Successfully");
#endif
		
#if SSL_CFG_REQUEST_CERTIFICATE
		if(pSession->connectState.certRequested) {
			pSession->connectState.hsState = CLIENT_CERT_STATE;
		} else 
#endif
		{
			pSession->connectState.hsState = CLIENTKEYEX_STATE;
		}
	}

#if SSL_CFG_REQUEST_CERTIFICATE
	if(pSession->connectState.hsState == CLIENT_CERT_STATE) {
		/* receives a client certificate message */
		lReturnValue  = SSLRecvHandshake(pSession, CERTIFICATE);
		if(lReturnValue == SSL_ERROR && pSession->connectState.lastAlert == SSL3_NO_CERTIFICATE_ALERT && 
			pSession->sessionCache.version == SSL_VERSION3) {
			if(!RUNTIME_CLIENTCERT_MANDATORY) {
				lReturnValue = SSL_SUCCESS;
			} else {
				SSLSendAlert(pSession, AL_FATAL, HANDSHAKE_FAILURE_ALERT);
			}
		}
		CHECK_RETURN_VALUE(lReturnValue);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Client Certificate State crossed Successfully");
#endif
		pSession->connectState.hsState = CLIENTKEYEX_STATE;
	}
#endif /* SSL_CFG_REQUEST_CERTIFICATE */



	if(pSession->connectState.hsState == CLIENTKEYEX_STATE) {
		/* receives a client key exchange message */
		lReturnValue  = SSLRecvHandshake(pSession, CLIENTKEYEX);
		CHECK_RETURN_VALUE(lReturnValue);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Client Key Exchange Received Successfully");
#endif

#if SSL_CFG_REQUEST_CERTIFICATE
		if(pSession->connectState.certRequested && SSLCertVerifyNeeded(pSession)) {
			pSession->connectState.hsState = CERT_VERIFY_STATE;
		} else 
#endif
		{
			pSession->connectState.hsState = RECV_CHANGE_CIPHER_STATE;

			/* generate the session keys */
			if(!SSLGenerateSessionKeys(pSession)) {
				return SSL_ERROR;
			}
		}
	}

#if SSL_CFG_REQUEST_CERTIFICATE
	if(pSession->connectState.hsState == CERT_VERIFY_STATE) {
		/* receives a client certificate verify message */
		lReturnValue  = SSLRecvHandshake(pSession, CERTIFICATE_VERIFY);
		CHECK_RETURN_VALUE(lReturnValue);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Client certificate verify message received successfully");
#endif

		pSession->connectState.hsState = RECV_CHANGE_CIPHER_STATE;
		/* generate the session keys */
		if(!SSLGenerateSessionKeys(pSession)) {
			return SSL_ERROR;
		}
	}
#endif /* SSL_CFG_REQUEST_CERTIFICATE */

	/* sends and receives finsihed and chnage cipher spec messages */
	lReturnValue  = SSLHandshakeFinal(pSession);
	
	CHECK_RETURN_VALUE(lReturnValue);

#if SSL_CFG_DEBUG_ENABLE
		SSL_DEBUG("\n Reached Application Data Transfer State ");
#endif

	/* session is ready for application data transfer */
	pSession->connectState.hsState = APPLICATION_STATE;
	
	/* store the SSL session in cache */
	SSLRegisterSession(pSession);

	/* frees the handshake buffers */
	SSLHandshakeBuffersClear(pSession);
	return SSL_SUCCESS;
}



