/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	SSL Record Layer

* Interface Spec		:	None.

* Description			:	This file contains the functions, for implementing the 
							SSL Record Layer.

* Author Name			:	M.K.Saravanan

* Revision				:	1.0

* Known Bugs			:	None

******************************************************************************************/


#define EXTERN_SSL_MEMORY
#include <SSLCommon.h>

/*****************************************************************************************
* Function Name	:	SSLWriteRecordBuffered
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					UINT8 *pData - (IN) The data to be sent
					INT32 lDataSize - (IN) The data size
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case the socket would block for this 
							operation
* Description	:	This function sends the input data over transport layer. In case the
					previuos write was blocked this API will be called with NULL parameters
					to flush the previous buffer.
******************************************************************************************/

static INT32 SSLWriteRecordBuffered(SSL *pSession, UINT8 *pData, INT32 lDataSize)
{
	INT32 lDataLeft;
	INT32 lRetVal;
	INT32 lReturnFlag;
	INT32 lBufferFlag;

	/* this function sends the data from record send buffer or the current data. 
	checks both does not present */
	if (pSession->connectState.recordSendBuffer.ulSize != 0 && lDataSize != 0) {
		return SSL_ERROR;
	}

	lBufferFlag = SSL_FALSE;
	if(lDataSize == 0) {
		/* gets the data and datasize to be sent from the record send buffer, 
		this is the buffer that got buffered from previous send */
		pData = pSession->connectState.recordSendBuffer.pData;
		lDataSize = pSession->connectState.recordSendBuffer.ulSize;
		lBufferFlag = SSL_TRUE;
	}


	lDataLeft = lDataSize;
	lReturnFlag = SSL_FALSE;

	/* loop until all the data is sent out. In case of 
	SSL_ERROR(-1) return immediately
	SSL_WOULD_BLOCK(-2) save the remaining data in record send buffer and return immeditealy
	if SSLWriteToTransport return less no of bytes continue in loop to send the remaining 
	data */

	while (lDataLeft > 0) {
		/* send data to TCP */
		lRetVal = SSLWriteToTransport(pSession->connectState.lSocketDescr, 
			&pData[lDataSize-lDataLeft], lDataLeft);
		
		if (lRetVal == SSL_WOULD_BLOCK) {
			if(lBufferFlag) {
				/* the data is sent from the record send buffer, so remove whatever data
				that is sent from the buffer */
				SSLRetrieveBuffer(&pSession->connectState.recordSendBuffer,NULL,
					lDataSize-lDataLeft);
			} else {
				/* the current data is sent , so put the remaining data into the 
				record send buffer */
				if(!SSLInsertBuffer(&pSession->connectState.recordSendBuffer,
					&pData[lDataSize-lDataLeft], lDataLeft)) {
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
		return (lRetVal);
	}

	if(lBufferFlag) {
		/* all the data is sent from the record send buffer, so empty the buffer */
		SSLRetrieveBuffer(&pSession->connectState.recordSendBuffer, NULL, lDataSize);
	}

	return SSL_SUCCESS;
}


/*****************************************************************************************
* Function Name	:	SSLGenerateCipherText
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					ContentType cType - (IN) The record payload type.
					UINT8 *pData - (IN) The data to be sent
					INT32 lDataSize - (IN) The data size
					UINT8 *pCipherText - (OUT) The cipher text generated
					INT32 lMaxCipherLength - (IN) The cipher text length
* Return Value	:	INT32 - The cipher length on Success
							SSL_ERROR(-1) in case of socket error
* Description	:	This function genrates the cipher text which is sent as the record
					protocol payload.
******************************************************************************************/

static INT32 SSLGenerateCipherText(SSL *pSession,ContentType cType,void *pData,INT32 lDataSize,
							UINT8 *pCiphertext,INT32 lMaxCipherLength)
{
	UINT8	*pDataToEncrypt;
	INT32	lDataToEncryptLen;
	INT32	lDataPtr;
	VarData macData;
	UINT8	padLength;
	INT32	lResult;

	/* implements
	data[length] + MAC[hash_size] + padding[padding_length] + padding_length
	and encrpyts it */


	macData.ulSize = SSLGetMessageDigestSize(pSession->secParams.writeMacAlgorithm);

	/* this function returns the pad length in case of block ciphers only, in case 
	of stream ciphers it returns 0 */
	padLength = SSLGetPaddingLength(pSession->secParams.writeBulkCipherAlgorithm, 
		lDataSize + macData.ulSize);
	
	/* calculate the total length of data to be encrypted */
	lDataToEncryptLen = lDataSize + macData.ulSize + padLength;

	if(lMaxCipherLength < lDataToEncryptLen) {
		return SSL_ERROR;
	}

	/* allocate memory for data to be encrypted */
	pDataToEncrypt = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lDataToEncryptLen);
	if(pDataToEncrypt == NULL) {
		return SSL_ERROR;
	}

	if(lDataSize != 0) {
		/* copy the actual data first */
		memcpy(pDataToEncrypt, pData, lDataSize);
	}

	lDataPtr = lDataSize;
	
	
	
	if(macData.ulSize != 0) {
		/* generate MAC for this record and copy it to the data to be encrypted */
		macData.pData = &pDataToEncrypt[lDataPtr];
		if(!SSLGenerateMac (pSession->secParams.writeMacAlgorithm,
			&pSession->secParams.writeMacSecret,
			&pSession->secParams.writeSequenceNumber, cType, pSession->sessionCache.version, 
			pData, lDataSize, &macData)) {
			OS_ADPT_Deallocate_Memory(pDataToEncrypt);
			return SSL_ERROR;
		}
		memcpy(&pDataToEncrypt[lDataPtr], macData.pData, macData.ulSize);

		lDataPtr += macData.ulSize;
	}

	if(padLength) {
		/* if padding is needed, in case of block cipher, copy the pad data */
		memset(&pDataToEncrypt[lDataPtr], (padLength - 1), padLength);
	}

	/* encrypt the total data + MAC + pad data */
	
	lResult = SSLEncrypt(pSession->secParams.writeBulkCipherAlgorithm,pDataToEncrypt, 
		lDataToEncryptLen, &pSession->secParams.writeKey,&pSession->secParams.writeIV,
		pCiphertext,lMaxCipherLength);

	OS_ADPT_Deallocate_Memory(pDataToEncrypt);


	if(lResult) {
		return lDataToEncryptLen;
	} else {
		return SSL_ERROR;
	}
}


/*****************************************************************************************
* Function Name	:	SSLRecordSend
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					ContentType cType - (IN) The record payload type.
					UINT8 *pData - (IN) The data to be sent
					INT32 lSize - (IN) The data size
* Return Value	:	INT32 - The data sent on Success
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case this operation would block on 
							socket
* Description	:	This function send the data over the record layer using the 
					SSL session
******************************************************************************************/

INT32 SSLRecordSend (SSL * pSession, ContentType type, void *pData, INT32 lSize)
{
	INT32	lDataToSend;
	INT32	lCipherSize;
	INT32	lRecordSize;
	UINT8	*pRecord;
	INT32	lReturnValue;

	if(lSize == 0 || pData == NULL) {
		/* in case data to be sent is zero size, returns 0 as no of bytes sent */ 
		return 0;
	}

	/* check whether this session is valid */
	if(pSession->connectState.valid != VALID_SESSION) {
		return SSL_ERROR;
	}

	if(pSession->connectState.recordSendBuffer.ulSize != 0) {
		/* in case record send buffer is not empty then, flush the previous buffer */
		lReturnValue = SSLWriteRecordBuffered(pSession, NULL, 0);
		if(lReturnValue < 0) {
			return lReturnValue;
		}
		/* succesfully flushed out prevoius buffer, get the previous buffers data size
		before encyption to return to the application */
		lReturnValue = pSession->connectState.lRecordSendUserSize;
		/* make the previous buffers application data size 0 as the buffer is flushed out */
		pSession->connectState.lRecordSendUserSize = 0; 
	} else {

		/* if application data is greater than maximum record size, then send only
		the maximum record size */
		if(lSize > MAX_RECORD_SIZE) {
			lDataToSend = MAX_RECORD_SIZE;
		} else {
			lDataToSend = lSize;
		}
		
		/* allocate memory for the total record size */
		lRecordSize = lDataToSend + MAX_RECORD_OVERHEAD;
		pRecord = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lRecordSize);

		if(pRecord == NULL) {
			return SSL_ERROR;
		}

		/* fillup the record headers */
		pRecord[CONTENT_TYPE_POS] = type;

		if(pSession->sessionCache.version == SSL_VERSION3) {
			pRecord[VERSION_MAJOR_POS] = SSL3_VER_MAJOR;
			pRecord[VERSION_MINOR_POS] = SSL3_VER_MINOR;

		} else if(pSession->sessionCache.version == TLS_VERSION1) {
			pRecord[VERSION_MAJOR_POS] = TLS1_VER_MAJOR;
			pRecord[VERSION_MINOR_POS] = TLS1_VER_MINOR;

		} else 	{
			OS_ADPT_Deallocate_Memory(pRecord);
			return SSL_ERROR;
		}

		/* generate the cipher text which form the payload of this record */
		
		lCipherSize = SSLGenerateCipherText(pSession, type, pData,lDataToSend,
			&pRecord[RECORD_HEADER_SIZE], lRecordSize-RECORD_HEADER_SIZE);

		if(lCipherSize < 0) {
			OS_ADPT_Deallocate_Memory(pRecord);
			return SSL_ERROR;
		}

		/* write the length of payload in record header */
		SSLWriteUint16((UINT16)lCipherSize,&pRecord[RECORD_LENGTH_POS]);

		/* increment the write sequence number */
		if(SSLIncrementUint64(&pSession->secParams.writeSequenceNumber) != SSL_SUCCESS) {
			pSession->connectState.valid = INVALID_SESSION;
			//pSession->connectState.resumable= RESUME_FALSE;		//YKS & Debu... 13/01/2004
			pSession->connectState.resumable= RESUME_TRUE;			//YKS & Debu... 13/01/2004
			OS_ADPT_Deallocate_Memory(pRecord);
			return SSL_ERROR;
		}


		/* write the data to TCP, in case the data is not fully transmitted it is 
		buffered in record send buffer */
		lReturnValue = SSLWriteRecordBuffered(pSession, pRecord, 
			lCipherSize + RECORD_HEADER_SIZE);
		if(lReturnValue == SSL_SUCCESS) {
			lReturnValue = lDataToSend;
		} else if (lReturnValue == SSL_WOULD_BLOCK) {
			/* store the application data size for the next write flush, which 
			will be returned to the application */
			pSession->connectState.lRecordSendUserSize = lDataToSend;
		}

		OS_ADPT_Deallocate_Memory(pRecord);
	}


	if(lReturnValue == SSL_ERROR) {
		pSession->connectState.valid = INVALID_SESSION;
		//pSession->connectState.resumable= RESUME_FALSE;		//YKS & Debu... 13/01/2004
		pSession->connectState.resumable= RESUME_TRUE;			//YKS & Debu... 13/01/2004
	}

	return lReturnValue;
}


/*****************************************************************************************
* Function Name	:	SSLReadRecordBuffered
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					UINT8 *pData - (OUT) The pointer where data is copied
					INT32 lDataSize - (IN) The data size
					INT32 lPeek -(IN) whether to peek the data or remove the data from 
					the record read buffer
* Return Value	:	INT32 - The lDataSize on Success
							SSL_CONNECTION_CLOSED(0) on TCP connection closed
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case this operation would block on 
							socket
* Description	:	This function reads the requested no. of bytes and the data is not 
					removed from the record read buffer in case of peek.
******************************************************************************************/
static INT32 SSLReadRecordBuffered(SSL *pSession, UINT8 *pData, INT32 lDataSize, INT32 lPeek)
{
	INT32	lBufferSize;
	UINT8	*pReadBuffer;
	INT32	lReadSize;
	INT32	lDataLeft;
	INT32	lReturnVal;
	INT32	lReturnFlag;

	/* get the size of record recv buffer */
	lBufferSize = SSLGetBufferSize(&pSession->connectState.recordRecvBuffer);

	if(lBufferSize >= lDataSize) {
		/* in case the required data is less than or equal to the buffered data ,
		send data in the buffer */
		if(lPeek == SSL_NO_PEEK) {
			SSLRetrieveBuffer(&pSession->connectState.recordRecvBuffer,pData,lDataSize);
		} else {
			SSLPeekBuffer(&pSession->connectState.recordRecvBuffer,pData,lDataSize);
		}
		return lDataSize;
	} else {
		/* in case the required data is greater than buffered data ,
		read more data from TCP */

		/* calculate the read size based on existing recv buffer */
		pReadBuffer = &pData[lBufferSize];
		lReadSize = lDataSize-lBufferSize;

		lDataLeft = lReadSize;

		lReturnFlag = SSL_FALSE;
		/* read until required data is read, and return in case of socket errors */
		while(lDataLeft > 0) {
			/* reads data from TCP */
			lReturnVal = SSLReadFromTransport(pSession->connectState.lSocketDescr,
				&pReadBuffer[lReadSize-lDataLeft], lDataLeft);
			if(lReturnVal <= 0) {
				if(lReturnVal == SSL_WOULD_BLOCK) {
					/* in case socket would block, insert whatever is read and return */
					if(!SSLInsertBuffer(&pSession->connectState.recordRecvBuffer,pReadBuffer,
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
				SSLRetrieveBuffer(&pSession->connectState.recordRecvBuffer,pData,
					pSession->connectState.recordRecvBuffer.ulSize);
			} else {
				SSLPeekBuffer(&pSession->connectState.recordRecvBuffer,pData,
					pSession->connectState.recordRecvBuffer.ulSize);
			}
		}

		if(lPeek == SSL_PEEK) {
			/* in case of peek insert all the data that is read from TCP */
			if(!SSLInsertBuffer(&pSession->connectState.recordRecvBuffer,pReadBuffer,
				lReadSize)) {
				return SSL_ERROR;
			}
		}
		return lDataSize;
	}
}

/*****************************************************************************************
* Function Name	:	SSLCheckRecordHeader
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					UINT8 *pHeaders - (IN) the pointer to the 
					ContentType cType - (IN) The record payload type
					HandshakeType hType - (IN) The handshake message type
					ContentType *pRecvCType - (OUT) The received content type
					SSLVersion *pVersion - (OUT) The version of the message received
					INT32 *pLength - (OUT) The length of the record payload
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_FAILURE(0) on error
* Description	:	This validates the fileds in record header and ouputs the record header
					fileds in case of success
******************************************************************************************/
static INT32 SSLCheckRecordHeader(SSL *pSession, UINT8 *pHeaders, ContentType cType, 
						   HandshakeType hType, ContentType *pRecvCType, 
						   SSLVersion *pVersion, UINT16 *pLength, INT32 *pRecordHdrSize)
{
	/* checks the received record header */
	if(hType == CLIENT_HELLO && cType == HANDSHAKE && pHeaders[0] > 127) {
		/* accept SSL v2 only if the client hello is expected */
		
		/* fills all the values required for v2 client hello processing */
		*pRecordHdrSize = SSL2_RECORD_HEADER_SIZE;
		*pLength = (((pHeaders[0] & 0x7f) << 8)) | pHeaders[1];
		*pRecvCType = HANDSHAKE;
		pSession->connectState.v2Hello = *pLength;
	} else {
		/* validates SSL v3 or TLS v1 record headers */
		
		*pRecvCType = pHeaders[CONTENT_TYPE_POS];

		/* checks the record content type */
		switch(*pRecvCType) {
		case CHANGE_CIPHER_SPEC:
		case ALERT:
		case HANDSHAKE:
		case APPLICATION_DATA:
			break;
		default:
			return SSL_FAILURE;
		}

		/* gets SSL version in record header */
		if(pHeaders[VERSION_MAJOR_POS] == SSL3_VER_MAJOR &&
			pHeaders[VERSION_MINOR_POS] == SSL3_VER_MINOR) {
			*pVersion = SSL_VERSION3;
		} else if (pHeaders[VERSION_MAJOR_POS] == TLS1_VER_MAJOR &&
			pHeaders[VERSION_MINOR_POS] == TLS1_VER_MINOR) {
			*pVersion = TLS_VERSION1;
		} else {
			*pVersion = SSL_VERSION_UNKNOWN;
		}

		/* checks SSL version in record header, in case of client hello the version is
		not validated, as version is determined in handshake processiong of client hello */
		if(hType != CLIENT_HELLO && hType != SERVER_HELLO && 
			*pVersion != pSession->sessionCache.version) {
			pSession->connectState.valid = INVALID_SESSION;
			return SSL_FAILURE;
		}

		if(hType == CLIENT_HELLO) {
			pSession->sessionCache.version = *pVersion;

			/**** BEGIN - Added by YKS - 05/04/2004 ****/
			pSession->sessionCache.clientVersion = *pVersion;
			/**** END - Added by YKS - 05/04/2004 ****/
		}

		/* gets record payload length and validates it */
		*pLength = SSLReadUint16(&pHeaders[RECORD_LENGTH_POS]);

		if(*pLength > MAX_RECV_SIZE) {
			pSession->connectState.valid = INVALID_SESSION;
			//pSession->connectState.resumable = RESUME_FALSE;		//YKS & Debu... 13/01/2004
			pSession->connectState.resumable= RESUME_TRUE;			//YKS & Debu... 13/01/2004
			return SSL_FAILURE;
		}

		*pRecordHdrSize = RECORD_HEADER_SIZE;
	}

	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLReadRecord
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					ContentType cType - (IN) The record payload type
					HandshakeType hType - (IN) The handshake message type
					UINT8 **pData - (OUT) The pointer where data is allocated & copied
					INT32 *pSize- (OUT) The output record size
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_CONNECTION_CLOSED(0) on TCP connection closed
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case this operation would block on 
							socket
* Description	:	This function reads the record header and record payload on by one.
					If the required data is not available and it would block it buffers
					the data in record read buffer and returns. Returns both record header
					and record payload.
******************************************************************************************/

static INT32 SSLReadRecord(SSL *pSession, ContentType cType, ContentType *pRevType, 
					HandshakeType hType, void **pData, INT32 *pSize, INT32 *pRecordHdrSize)
{
	UINT8	recordHeader[RECORD_HEADER_SIZE];
	SSLVersion	version;
	UINT16		length;
	INT32		lReturnVal;
	INT32		lRecordHdrSize;

	/* read the record header with peek */
	if((lReturnVal = SSLReadRecordBuffered(pSession,recordHeader, 
		RECORD_HEADER_SIZE, SSL_PEEK)) != RECORD_HEADER_SIZE) {
		if(lReturnVal == SSL_WOULD_BLOCK) {
			return lReturnVal;
		}
		pSession->connectState.valid = INVALID_SESSION;
		if (cType==ALERT) {
			return lReturnVal; /* we were expecting close notify alert */
		}
		//pSession->connectState.resumable = RESUME_FALSE;		//YKS & Debu... 13/01/2004
		pSession->connectState.resumable= RESUME_TRUE;			//YKS & Debu... 13/01/2004
		return lReturnVal;
	}

	/* validates the contents of record header */
	if(SSLCheckRecordHeader(pSession,recordHeader,cType,hType,pRevType,&version,&length, 
		&lRecordHdrSize)
		!= SSL_SUCCESS) {
		return SSL_ERROR;
	}
	
	/* the record header size is variable to make v2 record header compatibility */

	if(length > 0) {
		/* if record is not empty allocate memory to read the required data */
		*pData = OS_ADPT_Allocate_Memory(SSL_Memory,length+lRecordHdrSize);
		if(*pData == NULL) {
			return SSL_ERROR;
		}

		/* read the record payload + record header using no peek */
		if((lReturnVal = SSLReadRecordBuffered(pSession,*pData, 
			length+lRecordHdrSize, SSL_NO_PEEK)) != length+lRecordHdrSize) {
			OS_ADPT_Deallocate_Memory(*pData);
			if(lReturnVal == SSL_WOULD_BLOCK){
				return SSL_WOULD_BLOCK;
			}
			pSession->connectState.valid = INVALID_SESSION;
			if (cType==ALERT) {
				return SSL_ERROR; /* we were expecting close notify */
			}
			//pSession->connectState.resumable = RESUME_FALSE;		//YKS & Debu... 13/01/2004
			pSession->connectState.resumable= RESUME_TRUE;			//YKS & Debu... 13/01/2004
			return SSL_ERROR;
		}
	}

	/* set the output record size and record header size */
	*pSize = length+lRecordHdrSize;
	*pRecordHdrSize = lRecordHdrSize;
	return SSL_SUCCESS;
}


/*****************************************************************************************
* Function Name	:	SSLGeneratePlainText
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					UINT8 *pCipherText - (IN) The input cipher text
					INT32 lCipherSize - (IN) The input cipher length
					UINT8 *pData - (OUT) The plain text generated
					INT32 lDataSize- (IN) The data size
					ContentType cType - (IN) The content type
* Return Value	:	INT32 - Plain text Length on Success
							SSL_ERROR(-1) in case of error
* Description	:	This function generates the plain text data from the record payload
					cipher text
******************************************************************************************/
static INT32 SSLGeneratePlainText(SSL *pSession, UINT8 *pCipherText, INT32 lCipherSize,
					 UINT8 *pData, INT32 lDataSize, ContentType cType)
{
	INT32	lBlockSize;
	INT32	lReturnVal;
	INT32	lHashSize;
	INT32	lDataLength;
	UINT8	padLength;
	UINT8	padCount;
	VarData OutputMac;
	INT32	lReturnFlag;
	UINT8	Mac[SSL_CFG_MAX_HASH_SIZE];


	if(lCipherSize == 0) {
		/* in case empty data is passed return 0 as size of plain text */
		return 0;
	}
	
	/* get the size of MAC used */
	lHashSize = SSLGetMessageDigestSize(pSession->secParams.readMacAlgorithm);

	/* decrypt and process based on stream or block cipher */
	switch(SSLGetCipherType(pSession->secParams.readBulkCipherAlgorithm)) {
	case CIPHER_BLOCK:
		lBlockSize = SSLGetCipherBlockSize(pSession->secParams.readBulkCipherAlgorithm);
		if ((lCipherSize < lBlockSize) || (lCipherSize % lBlockSize != 0)) {
			return SSL_ERROR;
		}

		/* decrypt the data */
		lReturnVal = SSLDecrypt(pSession->secParams.readBulkCipherAlgorithm,pCipherText,
			lCipherSize, &pSession->secParams.readKey,&pSession->secParams.readIV, 
			pData, lDataSize);
		if(lReturnVal == SSL_FAILURE) {
			return SSL_ERROR;
		}

		/* read the padlength from decrypted data */
		padLength = pData[lCipherSize - 1] + 1;
		lDataLength = lCipherSize - lHashSize - padLength;
		if (padLength > (lCipherSize - lHashSize)) {
			return SSL_ERROR;
		}

		/* in case of TLS v1 record, check the pad data */
		if (pSession->sessionCache.version == TLS_VERSION1) {
			lReturnFlag = SSL_FALSE;
			for (padCount = 2;padCount < padLength;padCount++) {
				if (pData[lCipherSize-padCount] != pData[lCipherSize - 1]) {
					lReturnFlag = SSL_TRUE;
					break;
				}
			}
			if(lReturnFlag) {
				return SSL_ERROR;
			}
		}
		break;

	case CIPHER_STREAM:

		/* decrypt the data */
		lReturnVal = SSLDecrypt(pSession->secParams.readBulkCipherAlgorithm,pCipherText, 
			lCipherSize, &pSession->secParams.readKey,&pSession->secParams.readIV, 
			pData,lDataSize);
		if(lReturnVal == SSL_FAILURE) {
			return SSL_ERROR;
		}
		lDataLength = lCipherSize - lHashSize;
		break;
	}

	/* check output bufer length is enough */
	if (lDataSize < lDataLength) {
		return SSL_ERROR;
	}

	/* in case MAC is present validate MAC */
	if(lHashSize != 0) {
		OutputMac.pData = Mac;
		OutputMac.ulSize = lHashSize;

		/* generate the MAC for the current record */
		if(!SSLGenerateMac(pSession->secParams.readMacAlgorithm,
			&pSession->secParams.readMacSecret, &pSession->secParams.readSequenceNumber, 
			cType,pSession->sessionCache.version, pData, lDataLength,&OutputMac)) {
			return SSL_ERROR;
		}

		/* check the MAC is correct */
		if(memcmp(OutputMac.pData,&pData[lDataLength],lHashSize) != 0) {
			return SSL_ERROR;
		}
	}
	
	return lDataLength;
}

/*****************************************************************************************
* Function Name	:	SSLCheckRecvRecordType
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					ContentType cType - (IN) The requested record payload type
					ContentType recvCType - (IN) The received record payload type
					UINT8 *pData - (IN) The plain text data generated
					INT32 lDataSize- (IN) The data size
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_CONNECTION_CLOSED(0) on SSL connction closed
							SSL_ERROR(-1) in case of error
* Description	:	This checks the record payload type and puts the data into the buffer
******************************************************************************************/
static INT32 SSLCheckRecvRecordType(SSL *pSession, ContentType cType, 
									ContentType recvCType, UINT8 *pData, 
									INT32 lDataSize) 
{
	INT32 lReturnVal;

	if((recvCType == cType) && (cType == APPLICATION_DATA || recvCType == HANDSHAKE)) {
		/* if application data or handshake data and it is requested just put into the 
		appropriate buffer */
		lReturnVal = SSLPutRecordBuffer(pSession, recvCType, pData, lDataSize);
	} else {
		switch (recvCType) {
		case ALERT:
			if(lDataSize != ALERT_SIZE) {
				return SSL_ERROR;
			}
			pSession->connectState.lastAlert= pData[ALERT_CODE_POS];
			
			/* if close notify is received and the alert is not fatal close the session */
			if(pData[ALERT_CODE_POS] == CLOSE_NOTIFY_ALERT && 
				pData[ALERT_LEVEL_POS] != AL_FATAL) {
				/* If we have been expecting for an alert do */
				lReturnVal = SSL_CONNECTION_CLOSED;
			} else {
						
				/* if the alert is FATAL invalidate the session */
				lReturnVal = SSL_SUCCESS;
				if (pData[ALERT_LEVEL_POS] == AL_FATAL) {
					pSession->connectState.valid = INVALID_SESSION;
					//pSession->connectState.resumable= RESUME_FALSE;		//YKS & Debu... 13/01/2004
					pSession->connectState.resumable= RESUME_TRUE;			//YKS & Debu... 13/01/2004
					lReturnVal = SSL_ERROR;
				}
			}
			break;

		case CHANGE_CIPHER_SPEC:
			/* this packet is now above */
			lReturnVal = SSL_ERROR;

		case APPLICATION_DATA:
			/* even if data is unexpected put it into the buffer */
			SSLPutRecordBuffer(pSession, recvCType, pData, lDataSize);
			lReturnVal = SSL_ERROR;
			break;

		case HANDSHAKE:
			lReturnVal = SSL_ERROR;
			break;

		default:
			lReturnVal = SSL_ERROR;
			break;
		}
	}
	/* return appropriate return values */
	return lReturnVal;
}

/*****************************************************************************************
* Function Name	:	SSLGetRecordBufferSize
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					ContentType cType - (IN) The record payload type
* Return Value	:	INT32 - size of data on Success
							SSL_ERROR(-1) in case of error
* Description	:	This functions returns the size of data buffered
******************************************************************************************/
static INT32 SSLGetRecordBufferSize(SSL *pSession, ContentType cType)
{
	switch(cType) {
		/* gets the buffer size based on the content type */
	case APPLICATION_DATA:
		return pSession->connectState.applicationData.ulSize;
	
	case HANDSHAKE:
		return pSession->connectState.handshakeData.ulSize;
	
	default:
		return SSL_ERROR;
	}
}

/*****************************************************************************************
* Function Name	:	SSLGetRecordBuffer
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					ContentType cType - (IN) The record payload type
					UINT8 *pData - (OUT) The pointer where data is copied
					INT32 lDataSize - (IN) The data size requested.
* Return Value	:	INT32 - size of data on Success
* Description	:	This functions copies the data from record buffer to user pointer
******************************************************************************************/
static INT32 SSLGetRecordBuffer(SSL *pSession, ContentType cType, UINT8 *pData, INT32 lDataSize)
{
	
	switch(cType) {
	case APPLICATION_DATA:

		/* sets the minimum of requested and the buffer available */
		if (lDataSize > (INT32)pSession->connectState.applicationData.ulSize) {
			lDataSize = pSession->connectState.applicationData.ulSize;
		}

		/* gets the data that is buffered for application from record application buffer */
		SSLRetrieveBuffer(&pSession->connectState.applicationData,pData,lDataSize);

		break;
		
	case HANDSHAKE:
		/* sets the minimum of requested and the buffer available */
		if (lDataSize > (INT32)pSession->connectState.handshakeData.ulSize) {
			lDataSize = pSession->connectState.handshakeData.ulSize;
		}

		/* gets the data that is buffered for handshake from record handshake buffer */
		SSLRetrieveBuffer(&pSession->connectState.handshakeData,pData,lDataSize);
		break;
	default:
		break;
	}
	return lDataSize;
}

/*****************************************************************************************
* Function Name	:	SSLCheckRecordBuffers
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					ContentType cType - (IN) The record payload type
					UINT8 *pData - (IN) The pointer where data is copied
					INT32 lDataSize - (IN) The data size requested.
* Return Value	:	INT32 - size of data on Success
* Description	:	This functions copies the data from record buffer to user pointer
******************************************************************************************/
static INT32 SSLCheckRecordBuffers(SSL *pSession, ContentType cType, UINT8 *pData, 
							INT32 lDataSize) 
{
	INT32 lReturnVal;

	lReturnVal = SSL_FAILURE;
	if(cType == APPLICATION_DATA || cType == HANDSHAKE) {
		
		/* if content types are application or handhsake checks the size of 
		available buffer, if data is available return that data */

		 lReturnVal = SSLGetRecordBufferSize(pSession, cType);
		 if(lReturnVal > 0) {
			 /* gets the data */
			 lReturnVal = SSLGetRecordBuffer(pSession, cType, pData, lDataSize);
		 }
	}
	/* returns the data size, if no data available returns 0 */
	return lReturnVal;
}

/*****************************************************************************************
* Function Name	:	SSLPutRecordBuffer
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					ContentType cType - (IN) The record payload type
					UINT8 *pData - (IN) The data which is to be inserted
					INT32 lDataLength - (IN) The data size.
* Return Value	:	INT32 - SSL_SUCCESS(1) on Success
							SSL_ERROR(-1) on error
* Description	:	This functions inserts the data into record layer buffers
******************************************************************************************/
static INT32 SSLPutRecordBuffer(SSL *pSession, ContentType cType, UINT8 *pData, INT32 lDataLength)
{
	switch(cType) {
	case APPLICATION_DATA:
		/* puts the data received in record application buffer */
		if (SSLInsertBuffer(&pSession->connectState.applicationData, pData, 
			lDataLength) != SSL_SUCCESS) {
			return SSL_ERROR;
		}
		break;

	case HANDSHAKE:
		/* puts the data received in record handshake buffer */
		if ( SSLInsertBuffer(&pSession->connectState.handshakeData, pData, 
			lDataLength) != SSL_SUCCESS) {
			return SSL_ERROR;
		}
		break;

	default:
		return SSL_ERROR;
	}

	return SSL_SUCCESS;

}

/*****************************************************************************************
* Function Name	:	SSLRecordRecv
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					ContentType cType - (IN) The record payload type
					HandshakeType hType - (IN) The handshake type
					void *pData - (OUT) The pointer where received data is copied
					INT32 lDataLength - (IN) The data size.
* Return Value	:	INT32 - Data size received on Success
							SSL_CONNECTION_CLOSED(0) on TCP Connection closed
							SSL_ERROR(-1) on error
							SSL_WOULD_BLOCK(-2) on lower layer will block for this 
							operation and has to be called again.
* Description	:	This functions recives the data over an SSL session
******************************************************************************************/
INT32 SSLRecordRecv (SSL * pSession, ContentType cType, HandshakeType hType, void *pData, 
					 INT32 lSize)
{
	INT32	lReturnVal;
	UINT8	*pRecord;
	INT32	lRecordSize;
	UINT8	*pPlainText;
	INT32	lPlainTextLen;
	INT32	lDecryptLen =-1;
	ContentType	recvCType;
	INT32	lEmptyPackets=0;
	INT32	lReturnFlag;
	INT32	lRecordHdrSize;

	if (lSize == 0 || pData == NULL) {
		/* buffer is not passed for receiving the data, return */
		return SSL_ERROR;
	}

	/* check whether session is valid or not */
	if(pSession->connectState.valid != VALID_SESSION) {
		return SSL_ERROR;
	}

	/* check the record application and handshake buffers, if data is available return 
	it without reading a new record */
	lReturnVal = SSLCheckRecordBuffers(pSession, cType, pData, lSize);
	if(lReturnVal != 0) {
		return lReturnVal;
	}

	lReturnFlag = SSL_FALSE;

	/* reads until a proper record is read. some applications may use empty records,  
	To avoid just sending only empty records there is configurable 
	maximum empty records */
	while(lEmptyPackets < SSL_CFG_MAX_EMPTY_RECORDS) {

		/* reads a record */
		if((lReturnVal = SSLReadRecord(pSession, cType, &recvCType, hType, &pRecord, 
			&lRecordSize, &lRecordHdrSize)) <= 0) {
			lReturnFlag = SSL_TRUE;
			break;
		}

		/* if record size is equal to record header size it is an empty record */
		if(lRecordSize <= lRecordHdrSize) {
			lEmptyPackets++;
			continue;
		}
				
		/* allocate memory for plain text after decryption */
		lPlainTextLen = (lRecordSize - lRecordHdrSize) + MAX_RECORD_OVERHEAD;
		pPlainText = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lPlainTextLen);

		if(pPlainText == NULL) {
			OS_ADPT_Deallocate_Memory(pRecord);
			lReturnFlag = SSL_TRUE;
			lReturnVal = SSL_ERROR;
			break;
		}

		/* generate plaintext from cipher text, MAC validation is also done */
		lDecryptLen = SSLGeneratePlainText(pSession, &pRecord[lRecordHdrSize], 
			(lRecordSize - lRecordHdrSize), pPlainText, lPlainTextLen, recvCType);

		OS_ADPT_Deallocate_Memory(pRecord);
		
		/* decryption failed */
		if(lDecryptLen < 0) {
			OS_ADPT_Deallocate_Memory(pPlainText);
			pSession->connectState.valid = INVALID_SESSION;
			//pSession->connectState.resumable = RESUME_FALSE;		//YKS & Debu... 13/01/2004
			pSession->connectState.resumable= RESUME_TRUE;			//YKS & Debu... 13/01/2004
			lReturnFlag = SSL_TRUE;
			lReturnVal = SSL_ERROR;
			break;
		}

		/* empty record, continue reading */
		if(lDecryptLen == 0) {
			OS_ADPT_Deallocate_Memory(pPlainText);
			if(SSLIncrementUint64(&pSession->secParams.readSequenceNumber) != SSL_SUCCESS) {
				pSession->connectState.valid = INVALID_SESSION;
				//pSession->connectState.resumable= RESUME_FALSE;		//YKS & Debu... 13/01/2004
				pSession->connectState.resumable= RESUME_TRUE;			//YKS & Debu... 13/01/2004
				lReturnFlag = SSL_TRUE;
				lReturnVal = SSL_ERROR;
				break;
			}
			lEmptyPackets++;
		} else {
			/* succesfully read a record */
			break;
		}
	}

	if(lDecryptLen == -1)
	    return SSL_ERROR;

	if(lReturnFlag) {
		return lReturnVal;
	}

	/* if plain text length is 0, then it is continuous empty record */
	if(lDecryptLen == 0) {
		OS_ADPT_Deallocate_Memory(pPlainText);
		return SSL_ERROR;
	}

	/* if change cipher spec is received and it is expected return it */
	if (cType == CHANGE_CIPHER_SPEC && recvCType == CHANGE_CIPHER_SPEC) {
		if (lDecryptLen != lSize) {
			OS_ADPT_Deallocate_Memory(pPlainText);
			return SSL_ERROR;
		}
		memcpy(pData, pPlainText, lSize);
		OS_ADPT_Deallocate_Memory(pPlainText);
		return lDecryptLen;
	}

	/* increment read sequence number */
	if(SSLIncrementUint64(&pSession->secParams.readSequenceNumber) != SSL_SUCCESS) {
		pSession->connectState.valid = INVALID_SESSION;
		//pSession->connectState.resumable= RESUME_FALSE;		//YKS & Debu... 13/01/2004
		pSession->connectState.resumable= RESUME_TRUE;			//YKS & Debu... 13/01/2004
		OS_ADPT_Deallocate_Memory(pPlainText);
		return SSL_ERROR;
	}

	/* check the received and expected type are same */
	lReturnVal = SSLCheckRecvRecordType(pSession, cType, recvCType,pPlainText,
		lDecryptLen);
	if(lReturnVal <= 0) {
		OS_ADPT_Deallocate_Memory(pPlainText);
		return lReturnVal;
	}

	/* in case of application or handshake data return the buffer */
	if ((cType == APPLICATION_DATA || cType == HANDSHAKE) && (recvCType== cType)) {
		lReturnVal = SSLGetRecordBufferSize(pSession,cType);
		if(lReturnVal > 0) {
			lReturnVal = SSLGetRecordBuffer(pSession, cType, pData, lSize);
		}
	} else {
		lReturnVal = SSL_ERROR;
	}

	OS_ADPT_Deallocate_Memory(pPlainText);
	return lReturnVal;
}


/*****************************************************************************************
* Function Name	:	SSLSend
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					void *pData - (IN) The data to be sent
					INT32 lSize - (IN) The data size.
* Return Value	:	INT32 - Data size sent on Success
							SSL_ERROR(-1) on error
							SSL_WOULD_BLOCK(-2) on lower layer will block for this 
							operation and has to be called again.
* Description	:	This functions sends aplication data over an SSL session
******************************************************************************************/
INT32 SSLSend (SSL * pSession, void *pData, INT32 lSize)
{
	if(pSession->connectState.hsState != APPLICATION_STATE)
		return SSL_ERROR;
	/* sends application data over record layer */
	return SSLRecordSend (pSession, APPLICATION_DATA, pData, lSize);
}

/*****************************************************************************************
* Function Name	:	SSLRecv
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
					void *pData - (OUT) The pointer where data is copied
					INT32 lSize - (IN) The data size.
* Return Value	:	INT32 - Data size received on Success
							SSL_CONNECTION_CLOSED(0) on connection close
							SSL_ERROR(-1) on error
							SSL_WOULD_BLOCK(-2) on lower layer will block for this 
							operation and has to be called again.
* Description	:	This functions recives aplication data over an SSL session
******************************************************************************************/
INT32 SSLRecv (SSL * pSession, void *pData, INT32 lSize)
{
	if(pSession->connectState.hsState != APPLICATION_STATE)
		return SSL_ERROR;

	/* receives application data over record layer */
	return SSLRecordRecv (pSession, APPLICATION_DATA, -1, pData, lSize);
}


/*****************************************************************************************
* Function Name	:	SSLGetPendingDataSize
* Parameters	:	SSL *pSession	- (IN) The SSL session pointer
* Return Value	:	INT32 - Data size of application buffer
* Description	:	This functions returns size of the application data buffered.
******************************************************************************************/
INT32 SSLGetPendingDataSize(SSL *pSession)
{
	/* returns application data size buffered in record layer */
	return pSession->connectState.applicationData.ulSize;
}

