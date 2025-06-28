/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Interface Spec		:	None.

* Description			:	This file contains functions implementing SSL interface to 
							crypto algorithms

* Author Name			:	M.K.Saravanan

* Revision				:	1.0

* Known Bugs			:	None

******************************************************************************************/

#define EXTERN_SSL_MEMORY
#include <SSLCommon.h>


/*****************************************************************************************
* Function Name :	SSLGetMessageDigestSize
* Parameters	:	MACAlgorithm macAlgorithm	- (IN) The mac algorithm
* Return Value	:	INT32 - Returns the message digest size of MAC algorithm
* Description	:	This function returns the message digest size of MAC algorithm
******************************************************************************************/

INT32 SSLGetMessageDigestSize(MACAlgorithm macAlgorithm)
{
	INT32 lDigestSize;
	switch(macAlgorithm) {
	case MD5_MAC:
		lDigestSize = SSL_MD5_DIGEST_LEN;
		break;
	case SHA1_MAC:
		lDigestSize = SSL_SHA1_DIGEST_LEN;
		break;
	case NULL_MAC:
	default:
		lDigestSize = 0;
		break;
	}
	/* returns the message digest size of the MAC algorithm */
	return lDigestSize;
}

/*****************************************************************************************
* Function Name :	SSLGetCipherKeySize
* Parameters	:	BulkCipherAlgorithm cipherAlgorithm	- (IN) The cipher algorithm
* Return Value	:	INT32 - Returns the key size of cipher algorithm
* Description	:	This function returns the key size of cipher algorithm
******************************************************************************************/

INT32 SSLGetCipherKeySize(BulkCipherAlgorithm cipherAlgorithm)
{
	INT32 lKeySize;
	switch(cipherAlgorithm) {

#if SSL_CFG_3DES_EDE_CBC_ENABLED
	case TRIPLEDES_CIPHER:
		lKeySize = SSL_3DES_KEY_SIZE;
		break;
#endif /* SSL_CFG_3DES_EDE_CBC_ENABLED */

#if SSL_CFG_RC4_128_ENABLED
	case RC4_128_CIPHER:
		lKeySize = SSL_RC4_128_KEY_SIZE;
		break;
#endif /* SSL_CFG_RC4_128_ENABLED */

#if SSL_CFG_RC4_40_ENABLED
	case RC4_40_CIPHER:
		lKeySize = SSL_RC4_40_FINAL_KEY_SIZE;
		break;
#endif /* SSL_CFG_RC4_40_ENABLED */

	case NULL_CIPHER:
	default:
		lKeySize = 0;
		break;
	}

	/* returns the key size of the cipher algorithm */
	return lKeySize;
}

/*****************************************************************************************
* Function Name :	SSLGetExportCipherKeySize
* Parameters	:	BulkCipherAlgorithm cipherAlgorithm	- (IN) The cipher algorithm
* Return Value	:	INT32 - Returns the export key size of export cipher algorithms
* Description	:	This function returns the export key size of cipher algorithm
******************************************************************************************/
INT32 SSLGetExportCipherKeySize(BulkCipherAlgorithm cipherAlgorithm)
{
	INT32 lKeySize;
	switch(cipherAlgorithm) {

#if SSL_CFG_RC4_40_ENABLED
	case RC4_40_CIPHER:
		lKeySize = SSL_RC4_40_EXPORT_KEY_SIZE;
		break;
#endif /* SSL_CFG_RC4_40_ENABLED */

	case NULL_CIPHER:
	default:
		lKeySize = 0;
		break;
	}

	/* returns the export key size of the cipher algorithm */
	return lKeySize;
}



/*****************************************************************************************
* Function Name :	SSLGetCipherIVSize
* Parameters	:	BulkCipherAlgorithm cipherAlgorithm	- (IN) The cipher algorithm
* Return Value	:	INT32 - Returns the initialisation vector size of cipher algorithm
* Description	:	This function returns the IV size of cipher algorithm
******************************************************************************************/

INT32 SSLGetCipherIVSize(BulkCipherAlgorithm cipherAlgorithm)
{
	INT32 lIVSize;
	switch(cipherAlgorithm) {

#if SSL_CFG_3DES_EDE_CBC_ENABLED
	case TRIPLEDES_CIPHER:
		lIVSize = SSL_3DES_IV_SIZE;
		break;
#endif /* SSL_CFG_3DES_EDE_CBC_ENABLED */

#if SSL_CFG_RC4_128_ENABLED
	case RC4_128_CIPHER:
#endif /* SSL_CFG_RC4_128_ENABLED */

#if SSL_CFG_RC4_40_ENABLED
	case RC4_40_CIPHER:
#endif /* SSL_CFG_RC4_40_ENABLED */

	case NULL_CIPHER:
	default:
		lIVSize = 0;
		break;
	}
	/* returns the initialisation vector size of the cipher algorithm */
	return lIVSize;
}

/*****************************************************************************************
* Function Name	:	SSLGetPaddingLength
* Parameters	:	BulkCipherAlgorithm cipherAlgorithm	- (IN) The cipher algorithm
					INT32 lDataSize	-(IN) The data size for which padding is calculated
* Return Value	:	UINT8 - Returns the padding size required
* Description	:	This function returns the padding size required for input data size 
					and cipher algoithm
******************************************************************************************/

UINT8 SSLGetPaddingLength(BulkCipherAlgorithm cipherAlgorithm, INT32 lDataSize)
{
	UINT8 padLength;
	INT32 lBlockSize;
	switch(cipherAlgorithm) {

#if SSL_CFG_3DES_EDE_CBC_ENABLED
	case TRIPLEDES_CIPHER:
		lBlockSize = SSL_3DES_BLOCK_SIZE;
		break;
#endif /* SSL_CFG_3DES_EDE_CBC_ENABLED */

#if SSL_CFG_RC4_128_ENABLED
	case RC4_128_CIPHER:
#endif /* SSL_CFG_RC4_128_ENABLED */

#if SSL_CFG_RC4_40_ENABLED
	case RC4_40_CIPHER:
#endif /* SSL_CFG_RC4_40_ENABLED */

	case NULL_CIPHER:
	default:
		lBlockSize = SSL_STREAM_BLOCK_SIZE;
		break;
	}
	
	/* returns the padding length of the cipher algorithm */
	if(lBlockSize == SSL_STREAM_BLOCK_SIZE) {
		/* in case of stream ciphers no padding is required */
		return SSL_STREAM_BLOCK_SIZE;
	} else {
		/* calculate the padding length */
		padLength = (UINT8) (lBlockSize - (lDataSize % lBlockSize));
		return padLength;
	}
}

/*****************************************************************************************
* Function Name	:	SSLGetCipherType
* Parameters	:	BulkCipherAlgorithm cipherAlgorithm	- (IN) The cipher algorithm
* Return Value	:	CipherType - Returns whether block or stream cipher
* Description	:	This function returns the cipher algoithm is block or stream
******************************************************************************************/

CipherType SSLGetCipherType(BulkCipherAlgorithm cipherAlgorithm)
{
	CipherType	cipherType;
	switch(cipherAlgorithm) {

#if SSL_CFG_3DES_EDE_CBC_ENABLED
	case TRIPLEDES_CIPHER:
		cipherType = CIPHER_BLOCK;
		break;
#endif /* SSL_CFG_3DES_EDE_CBC_ENABLED */

#if SSL_CFG_RC4_128_ENABLED
	case RC4_128_CIPHER:
#endif /* SSL_CFG_RC4_128_ENABLED */

#if SSL_CFG_RC4_40_ENABLED
	case RC4_40_CIPHER:
#endif /* SSL_CFG_RC4_40_ENABLED */

	case NULL_CIPHER:
	default:
		cipherType = CIPHER_STREAM;
		break;
	}
	/* returns whether stream or block cipher */
	return cipherType;
}

/*****************************************************************************************
* Function Name	:	SSLGetCipherBlockSize
* Parameters	:	BulkCipherAlgorithm cipherAlgorithm	- (IN) The cipher algorithm
* Return Value	:	INT32 - Returns block size of the cipher algorithm
* Description	:	This function returns the cipher algoithm is block or stream
******************************************************************************************/

INT32 SSLGetCipherBlockSize(BulkCipherAlgorithm cipherAlgorithm)
{
	INT32 lBlockSize;
	switch(cipherAlgorithm) {

#if SSL_CFG_3DES_EDE_CBC_ENABLED
	case TRIPLEDES_CIPHER:
		lBlockSize = SSL_3DES_BLOCK_SIZE;
		break;
#endif /* SSL_CFG_3DES_EDE_CBC_ENABLED */

#if SSL_CFG_RC4_128_ENABLED
	case RC4_128_CIPHER:
#endif /* SSL_CFG_RC4_128_ENABLED */

#if SSL_CFG_RC4_40_ENABLED
	case RC4_40_CIPHER:
#endif /* SSL_CFG_RC4_40_ENABLED */

	case NULL_CIPHER:
	default:
		lBlockSize = SSL_STREAM_BLOCK_SIZE;
		break;
	}
	/* returns the block size of the cipher algorithm */
	return lBlockSize;
}

/*****************************************************************************************
* Function Name	:	SSLGetStreamStateSize
* Parameters	:	BulkCipherAlgorithm cipherAlgorithm	- (IN) The cipher algorithm
* Return Value	:	INT32 - Returns size of stream state which is needed by cipher
* Description	:	This function returns size of stream state
******************************************************************************************/

INT32 SSLGetStreamStateSize(BulkCipherAlgorithm cipherAlgorithm)
{
	INT32 lStreamStateSize;
	switch(cipherAlgorithm) {

#if SSL_CFG_RC4_128_ENABLED
	case RC4_128_CIPHER:
		lStreamStateSize = SSL_RC4_STREAM_STATE_SIZE;
		break;
#endif /* SSL_CFG_RC4_128_ENABLED */

#if SSL_CFG_RC4_40_ENABLED
	case RC4_40_CIPHER:
		lStreamStateSize = SSL_RC4_STREAM_STATE_SIZE;
		break;
#endif /* SSL_CFG_RC4_40_ENABLED */


#if SSL_CFG_3DES_EDE_CBC_ENABLED
	case TRIPLEDES_CIPHER:
#endif /* SSL_CFG_3DES_EDE_CBC_ENABLED */

	case NULL_CIPHER:
	default:
		lStreamStateSize = 0;
		break;
	}
	/* returns the size of stream state required for the algorithm */
	return lStreamStateSize;
}

/*****************************************************************************************
* Function Name	:	SSLInitStreamCipherState
* Parameters	:	SSL *pSession	- (IN) The pointer to SSL session
* Return Value	:	None.
* Description	:	This function initialies the state of the stream ciphers
******************************************************************************************/
void SSLInitStreamCipherState(SSL *pSession)
{
	switch(pSession->sessionCache.pCipherMapping->cipherAlgorithm) {
		
#if (SSL_CFG_RC4_128_ENABLED || SSL_CFG_RC4_40_ENABLED)
	case RC4_128_CIPHER:
	case RC4_40_CIPHER:
		/* intialise the state in case of RC4 stream cipher */
		RC4Init((RC4Context *)pSession->secParams.writeIV.pData,
			pSession->secParams.writeKey.pData, 
			pSession->secParams.writeKey.ulSize);
		RC4Init((RC4Context *)pSession->secParams.readIV.pData, 
			pSession->secParams.readKey.pData, 
			pSession->secParams.readKey.ulSize);

		break;
#endif /* (SSL_CFG_RC4_128_ENABLED || SSL_CFG_RC4_40_ENABLED) */

	/* no initialisation in case of block ciphers */
#if SSL_CFG_3DES_EDE_CBC_ENABLED
	case TRIPLEDES_CIPHER:
#endif /* SSL_CFG_3DES_EDE_CBC_ENABLED */

	case NULL_CIPHER:
	default:
		break;
	}
}

/*****************************************************************************************
* Function Name	:	SSLEncrypt
* Parameters	:	BulkCipherAlgorithm cipherAlgorithm	- (IN) The cipher algorithm
					UINT8 *pData - (IN) The data to be encrypted
					INT32 lDataSize - (IN) The length of data to be encrypted
					VarData *pWriteKey - (IN) The key for encryption
					VarData *pWriteIV - (IN/OUT) The initializtion vector for encryption
					UINT8 *pOutputCipher - (OUT) The cipher text
					INT32 ulCipherLen - (IN) The cipher text length
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function encrypts the data and retruns the cipher text.
******************************************************************************************/
INT32 SSLEncrypt(BulkCipherAlgorithm cipherAlgorithm, UINT8 *pData, INT32 lDataSize, 
				  VarData *pWriteKey, VarData *pWriteIV, UINT8 *pOutputCipher, 
				  INT32 ulCipherLen)
{
	INT32 lRetStatus;

	/* encrypts data based on the cipher algorithm */
	switch(cipherAlgorithm) {

#if SSL_CFG_3DES_EDE_CBC_ENABLED
	case TRIPLEDES_CIPHER:
		lRetStatus = TripleDesCBCEncryptData(pWriteKey->pData,
												pData, 
												(UINT32 *)&lDataSize,
												pWriteIV->pData,
												pOutputCipher);

		break;
#endif /* SSL_CFG_3DES_EDE_CBC_ENABLED */

#if (SSL_CFG_RC4_128_ENABLED || SSL_CFG_RC4_40_ENABLED)
	case RC4_128_CIPHER:
	case RC4_40_CIPHER:
		lRetStatus = RC4EncryptStream((RC4Context *)pWriteIV->pData, pOutputCipher, pData, 
			lDataSize);
		break;
#endif /* (SSL_CFG_RC4_128_ENABLED || SSL_CFG_RC4_40_ENABLED) */

	case NULL_CIPHER:
		if(ulCipherLen >= lDataSize) {
			memcpy(pOutputCipher, pData, lDataSize);
			lRetStatus = SSL_SUCCESS;
		} else {
			lRetStatus = SSL_FAILURE;
		}
		break;
	default:
		lRetStatus = SSL_ERROR;
		break;
	}

	if(lRetStatus < 0) {
		return SSL_FAILURE;
	} else {
		return SSL_SUCCESS;
	}
}

/*****************************************************************************************
* Function Name	:	SSLDecrypt
* Parameters	:	BulkCipherAlgorithm cipherAlgorithm	- (IN) The cipher algorithm
					UINT8 *pCipher - (IN) The data to be decrypted
					INT32 lCipherSize - (IN) The length of data to be decrypted
					VarData *pReadKey - (IN) The key for decryption
					VarData *pReadIV - (IN/OUT) The initializtion vector for decryption
					UINT8 *pData - (OUT) The plain text
					INT32 ulDataLen - (IN) The data length
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function decrypts the data and retruns the plain text.
******************************************************************************************/

INT32 SSLDecrypt (BulkCipherAlgorithm cipherAlgorithm, UINT8 *pCipher, INT32 lCipherSize, 
				  VarData *pReadKey, VarData *pReadIV, UINT8 *pData, INT32 ulDataLen)
{
	INT32 lRetStatus;

	/* decrypts data based on the cipher algorithm */
	switch(cipherAlgorithm) {

#if SSL_CFG_3DES_EDE_CBC_ENABLED
	case TRIPLEDES_CIPHER:
		lRetStatus = TripleDesCBCDecryptData(pReadKey->pData,
												pCipher, 
												lCipherSize,
												pReadIV->pData,
												pData);

		break;
#endif /* SSL_CFG_3DES_EDE_CBC_ENABLED */

#if (SSL_CFG_RC4_128_ENABLED || SSL_CFG_RC4_40_ENABLED)
	case RC4_128_CIPHER:
	case RC4_40_CIPHER:
		lRetStatus = RC4EncryptStream((RC4Context *)pReadIV->pData, pData, pCipher, 
			lCipherSize);
		break;
#endif /* (SSL_CFG_RC4_128_ENABLED || SSL_CFG_RC4_40_ENABLED) */


	case NULL_CIPHER:
		if(ulDataLen >= lCipherSize) {
			memcpy(pData, pCipher, lCipherSize);
			lRetStatus = SSL_SUCCESS;
		} else {
			lRetStatus = SSL_FAILURE;
		}
		break;

	default:
		lRetStatus = SSL_ERROR;
		break;
	}

	if(lRetStatus < 0) {
		return SSL_FAILURE;
	} else {
		return SSL_SUCCESS;
	}
}

/*****************************************************************************************
* Function Name	:	SSL3MacSign
* Parameters	:	MACAlgorithm macAlgorithm - (IN) The mac algorithm used
					VarData  *pMacSecret - (IN) The secret to be used in MAC
					SSLU64		*pSeqNum - (IN) The sequence number used in MAC
					ContentType	cType - (IN) The record type used in MAC
					UINT8	*pData - (IN) The data to be used in MAC
					UINT32	ulDataLen - (IN) The data length
					UINT8	*pAuthParams - (OUT) The MAC generated
					INT32	lAuthParamsLen - (IN) The MAC length
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates MAC for SSL version 3 messages
******************************************************************************************/
static INT32  SSL3MacSign(MACAlgorithm macAlgorithm, VarData  *pMacSecret, SSLU64	*pSeqNum,
				   ContentType	cType, UINT8	*pData, UINT32	ulDataLen,
				   UINT8	*pAuthParams, INT32	lAuthParamsLen)
{
	UINT8	*pMacDataFirst;
	UINT32	ulMacDataSizeFirst;
	UINT8	*pMacDataSecond;
	UINT32	ulMacDataSizeSecond;
	INT32	lPadSize;
	INT32	lDataPtr;

	if(lAuthParamsLen == 0) {
		return SSL_SUCCESS;
	}

	/* 
	hash(MAC_write_secret + pad_2 +
		hash(MAC_write_secret + pad_1 + seq_num + type + length + fragment));

	where "+" denotes concatenation.
	pad_1             The character 0x36 repeated 48 times for MD5
		              or 40 times for SHA.
	pad_2             The character 0x5c repeated 48 times for MD5
                      or 40 times for SHA. */

	switch(macAlgorithm) {
	case MD5_MAC:
		lPadSize = SSL3_MAC_MD5_PAD_SIZE;
		break;

	case SHA1_MAC:
		lPadSize = SSL3_MAC_SHA1_PAD_SIZE;
		break;
	}

	ulMacDataSizeFirst =  lPadSize + pMacSecret->ulSize + SSL_MAC_DATA_FIXED_SIZE + ulDataLen;
	ulMacDataSizeSecond = lAuthParamsLen + lPadSize + pMacSecret->ulSize;
	

	pMacDataFirst = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,ulMacDataSizeFirst);

	if(pMacDataFirst == NULL) {
		return SSL_FAILURE;
	}
	pMacDataSecond = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,ulMacDataSizeSecond);

	if(pMacDataSecond == NULL) {
		OS_ADPT_Deallocate_Memory(pMacDataFirst);
		return SSL_FAILURE;
	}

	/* concatenating all the data for the first hash */

	memcpy(pMacDataFirst,pMacSecret->pData, pMacSecret->ulSize);
	lDataPtr = pMacSecret->ulSize;

	memset(&pMacDataFirst[lDataPtr],0x36, lPadSize);
	lDataPtr += lPadSize;

	SSLWriteUint64(pSeqNum,&pMacDataFirst[lDataPtr]);
	lDataPtr += sizeof(SSLU64);

	pMacDataFirst[lDataPtr++] = cType;
	SSLWriteUint16((UINT16)ulDataLen,&pMacDataFirst[lDataPtr]);
	lDataPtr += sizeof(UINT16);
	if(ulDataLen) {
		memcpy(&pMacDataFirst[lDataPtr],pData,ulDataLen);
	}

	/* performing first hash */
	switch(macAlgorithm) {
	case MD5_MAC:
		MD5Sign(pMacDataFirst, ulMacDataSizeFirst, pAuthParams);
		break;

	case SHA1_MAC:
		SHA1Sign(pMacDataFirst, ulMacDataSizeFirst, pAuthParams);
		break;
	}

	/* concatenating data for the second hash */
	memcpy(pMacDataSecond,pMacSecret->pData, pMacSecret->ulSize);
	lDataPtr = pMacSecret->ulSize;

	memset(&pMacDataSecond[lDataPtr],0x5c, lPadSize);
	lDataPtr += lPadSize;

	memcpy(&pMacDataSecond[lDataPtr],pAuthParams,lAuthParamsLen);

	/* performing second hash */
	switch(macAlgorithm) {
		/* final oputput is copied in pAuthParams by MD5 and SHA1 functions */
	case MD5_MAC:
		MD5Sign(pMacDataSecond, ulMacDataSizeSecond, pAuthParams);
		break;

	case SHA1_MAC:
		SHA1Sign(pMacDataSecond, ulMacDataSizeSecond, pAuthParams);
		break;
	}
	
	OS_ADPT_Deallocate_Memory(pMacDataFirst);
	OS_ADPT_Deallocate_Memory(pMacDataSecond);

	return SSL_SUCCESS;
}


/*****************************************************************************************
* Function Name	:	SSLHmacSign
* Parameters	:	MACAlgorithm macAlgorithm - (IN) The mac algorithm used
					UINT8	*pData - (IN) The data to be used in MAC
					UINT32	ulDataLen - (IN) The data length
					VarData  *pMacSecret - (IN) The secret to be used in MAC
					UINT8	*pAuthParams - (OUT) The MAC generated
					INT32	lAuthParamsLen - (IN) The MAC length
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function implemets HMAC for TLS Version 1 messages
******************************************************************************************/

static INT32  SSLHmacSign(MACAlgorithm macAlgorithm, UINT8    *pData, UINT32	ulDataLen,
				   VarData	*pMacSecret, UINT8	*pAuthParams, INT32	lAuthParamsLen)
{
	UINT8	 au8K1[HMAC_EXTENDED_KEY_LEN];
	UINT8	 au8K2[HMAC_EXTENDED_KEY_LEN];
	UINT8    au8ExtendedKey[HMAC_EXTENDED_KEY_LEN] = {0};
	UINT8	*pDataFirstRound;
	UINT8	*pDataSecRound;
	INT32	lCount;
	
	if(lAuthParamsLen == 0) {
		return SSL_SUCCESS;
	}

	/*
	We define two fixed and different strings au8K1 and au8K2 as follows
		au8K1 = the byte 0x36 repeated B times
		au8K2 = the byte 0x5C repeated B times.
	where B is 64 for both MD5 and SHA1
	
	To compute HMAC over the data `text' we perform
		hash(K XOR au8K2, hash(K XOR au8K1, text))

	where K is the key of length less than or equeal to 64 */


	if(pMacSecret->ulSize > HMAC_EXTENDED_KEY_LEN) {
		return SSL_FAILURE;
	}

	/*Preparing extended key*/
	memset(au8ExtendedKey,0,HMAC_EXTENDED_KEY_LEN);
	if(pMacSecret->ulSize > 0) {
		memcpy(au8ExtendedKey, pMacSecret->pData, pMacSecret->ulSize);
	}

	/*Preparing the two different extended keys to be used in HMAC*/
	for(lCount = 0; lCount < HMAC_EXTENDED_KEY_LEN; lCount++)
	{
		au8K1[lCount] = au8ExtendedKey[lCount] ^ (0x36);
		au8K2[lCount] = au8ExtendedKey[lCount] ^ (0x5C);
	}

	pDataFirstRound = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,ulDataLen + HMAC_EXTENDED_KEY_LEN);
	if(pDataFirstRound == NULL) {
		return SSL_FAILURE;
	}
	pDataSecRound = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lAuthParamsLen + HMAC_EXTENDED_KEY_LEN);
	
	if(pDataSecRound == NULL) {
		OS_ADPT_Deallocate_Memory(pDataFirstRound);
		return SSL_FAILURE;
	}

	/*Preparing data as inout for first round of HASHing*/
	memcpy(pDataFirstRound, au8K1, HMAC_EXTENDED_KEY_LEN);
	memcpy(&pDataFirstRound[HMAC_EXTENDED_KEY_LEN],pData,ulDataLen);

	/*first round of HASHing*/
	switch(macAlgorithm) {
	case MD5_MAC:
		MD5Sign(pDataFirstRound, ulDataLen + HMAC_EXTENDED_KEY_LEN, pAuthParams);
		break;

	case SHA1_MAC:
		SHA1Sign(pDataFirstRound, ulDataLen + HMAC_EXTENDED_KEY_LEN, pAuthParams);
		break;
	}


	/*Preparing data for the second round*/
	memcpy(pDataSecRound, au8K2, HMAC_EXTENDED_KEY_LEN);
	memcpy(pDataSecRound + HMAC_EXTENDED_KEY_LEN, pAuthParams, lAuthParamsLen);

	/*second round of HASHing*/
	switch(macAlgorithm) {
	case MD5_MAC:
		MD5Sign(pDataSecRound, lAuthParamsLen + HMAC_EXTENDED_KEY_LEN, pAuthParams);
		break;
	case SHA1_MAC:
		SHA1Sign(pDataSecRound, lAuthParamsLen + HMAC_EXTENDED_KEY_LEN, pAuthParams);
		break;
	}

	OS_ADPT_Deallocate_Memory(pDataFirstRound);
	OS_ADPT_Deallocate_Memory(pDataSecRound);

	return SSL_SUCCESS;
}



/*****************************************************************************************
* Function Name	:	SSLGenerateMac
* Parameters	:	MACAlgorithm macAlgorithm - (IN) The mac algorithm used
					VarData  *pMacSecret - (IN) The secret to be used in MAC
					SSLU64		*pSeqNum - (IN) The sequence number used in MAC
					ContentType	cType - (IN) The record type used in MAC
					UINT8	*pData - (IN) The data to be used in MAC
					UINT32	ulDataSize - (IN) The data length
					VarData	*pOutputMac - (OUT) The MAC generated
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates MAC for SSL messages
******************************************************************************************/
INT32 SSLGenerateMac (MACAlgorithm macAlgorithm,VarData *pMacSecret, SSLU64 *pSeqNum, 
					  ContentType cType, SSLVersion version, UINT8 *pData,
					  INT32 lDataSize, VarData *pOutputMac)
{
	UINT8	*pMacData;
	UINT32	ulMacDataSize;
	UINT32	ulDataPtr;
	INT32	lReturnVal;

	/* generate the MAC for the SSL messages based on input params */
	if(version == SSL_VERSION3) {
		/* generates a SSL v3 MAC */
		lReturnVal = SSL3MacSign(macAlgorithm, pMacSecret, pSeqNum, cType, pData, lDataSize, pOutputMac->pData,
			pOutputMac->ulSize);
	} else {
		/* allocate memory for a TLS v1 MAC data*/
		ulMacDataSize = TLS_MAC_DATA_FIXED_SIZE + lDataSize;
		pMacData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, ulMacDataSize);
		if(pMacData == NULL) {
			return SSL_FAILURE;
		}

		/* concatenate all the MAC data */
		SSLWriteUint64(pSeqNum,pMacData);
		ulDataPtr = sizeof(SSLU64);
		pMacData[ulDataPtr++] = cType;
		pMacData[ulDataPtr++] = TLS1_VER_MAJOR;
		pMacData[ulDataPtr++] = TLS1_VER_MINOR;
		SSLWriteUint16((UINT16)lDataSize,&pMacData[ulDataPtr]);
		ulDataPtr += sizeof(UINT16);
		if(lDataSize) {
			memcpy(&pMacData[ulDataPtr],pData,lDataSize);
		}

		/* generartes HMAC of TLS v1 MAC data */
		lReturnVal = SSLHmacSign(macAlgorithm, pMacData,ulMacDataSize, pMacSecret,pOutputMac->pData,
			pOutputMac->ulSize);
		OS_ADPT_Deallocate_Memory(pMacData);
	}

	return lReturnVal;
}

/*****************************************************************************************
* Function Name	:	SSLTlsv1Finished
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					ConnectionEnd isServer - (IN) The Server or Client finished message
					UINT8 *pData - (OUT) The generated TLS v1 finished message
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates TLS v1 finished message for this session
******************************************************************************************/

INT32 SSLTlsv1Finished(SSL *pSession, ConnectionEnd isServer, UINT8 *pData)
{
	UINT8 data[SSL_MD5_DIGEST_LEN+SSL_SHA1_DIGEST_LEN];
	INT8 *pLabel;
	MD5_CTX md5;
	SHA_CTX sha;

	/* implements 
	PRF(master_secret, finished_label, MD5(handshake_messages) +
           SHA-1(handshake_messages)) [0..11];
	*/

	/* copying the hash of handshake messages to the local hash buffers */
	memcpy(&md5, &pSession->connectState.hShakeMd5Hash, sizeof(MD5_CTX));
	memcpy(&sha, &pSession->connectState.hShakeSha1Hash, sizeof(SHA_CTX));
	
	/* finalising the SHA1 and MD5 hashes of handshake messages */
	MD5_Final(data, &md5);
	SHA1_Final(&data[SSL_MD5_DIGEST_LEN], &sha);

	/* getting the label for PRF based on server or client finished message */
	if(isServer == SSL_SERVER) {
		pLabel = SERVER_FINSIHED_LABEL;
	} else {
		pLabel = CLIENT_FINSIHED_LABEL;
	}

	/* gets 12 byte output using the master secret, label and handshake hashes */
	return SSLGeneratePRF(pSession->sessionCache.masterSecret,MASTER_SECRET_SIZE,pLabel,
		TLS_FINISHED_LABEL_LEN, data, SSL_MD5_DIGEST_LEN+SSL_SHA1_DIGEST_LEN, pData,
		TLS_FINISHED_SIZE);
}

/*****************************************************************************************
* Function Name	:	SSLv3Finished
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					ConnectionEnd isServer - (IN) The Server or Client finished message
					UINT8 *pData - (OUT) The generated SSL v3 finished message
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function generates SSL v3 finished message for this session
******************************************************************************************/
INT32 SSLv3Finished(SSL *pSession, ConnectionEnd isServer, UINT8 *pData)
{
	MD5_CTX md5;
	SHA_CTX sha;
	UINT8 *pSender;
	UINT8 pad1[SSL3_MAC_MD5_PAD_SIZE];
	UINT8 pad2[SSL3_MAC_MD5_PAD_SIZE];
	UINT8 data[SSL_MD5_DIGEST_LEN+SSL_SHA1_DIGEST_LEN];

	/* implements 
		md5_hash[16] + sha_hash[20]
		
		md5_hash       MD5(master_secret + pad2 +
							MD5(handshake_messages + Sender +
								master_secret + pad1));
		sha_hash       SHA(master_secret + pad2 + 
			                SHA(handshake_messages + Sender +
								master_secret + pad1));

		pad_1			The character 0x36 repeated 48 times for MD5 or 40 times for SHA.
		pad_2           The character 0x5c repeated 48 times for MD5 or 40 times for SHA.

		Sender			"CLNT" for client finished and "SRVR" for server finished
	*/

	/* copying the hash of handshake messages to the local hash buffers */
	memcpy(&md5, &pSession->connectState.hShakeMd5Hash, sizeof(MD5_CTX));
	memcpy(&sha, &pSession->connectState.hShakeSha1Hash, sizeof(SHA_CTX));

	/* getting the Sender based on server or client finished message */
	if(isServer == SSL_SERVER) {
		pSender = (UINT8 *)SSL3_SERVER_FINISHED_LABEL;
	} else {
		pSender = (UINT8 *)SSL3_CLIENT_FINISHED_LABEL;
	}

	/* setting pad1 and pad2 */
	memset(pad1,0x36,SSL3_MAC_MD5_PAD_SIZE);
	memset(pad2,0x5c,SSL3_MAC_MD5_PAD_SIZE);

	/* takes first MD5 hash */
	MD5_Update(&md5,pSender,SSL3_FINISHED_LABEL_LEN);
	MD5_Update(&md5,pSession->sessionCache.masterSecret,MASTER_SECRET_SIZE);
	MD5_Update(&md5, pad1, SSL3_MAC_MD5_PAD_SIZE);
	MD5_Final(data, &md5);

	/* takes first SHA1 hash */
	SHA1_Update(&sha,pSender,SSL3_FINISHED_LABEL_LEN);
	SHA1_Update(&sha,pSession->sessionCache.masterSecret,MASTER_SECRET_SIZE);
	SHA1_Update(&sha,pad1,SSL3_MAC_SHA1_PAD_SIZE);
	SHA1_Final(&data[SSL_MD5_DIGEST_LEN], &sha);

	/* takes second MD5 hash */
	MD5_Init(&md5);
	MD5_Update(&md5, pSession->sessionCache.masterSecret,MASTER_SECRET_SIZE);
	MD5_Update(&md5, pad2, SSL3_MAC_MD5_PAD_SIZE);
	MD5_Update(&md5, data, SSL_MD5_DIGEST_LEN);
	MD5_Final(pData, &md5);

	/* takes second SHA1 hash */
	SHA1_Init(&sha);
	SHA1_Update(&sha, pSession->sessionCache.masterSecret,MASTER_SECRET_SIZE);
	SHA1_Update(&sha, pad2, SSL3_MAC_SHA1_PAD_SIZE);
	SHA1_Update(&sha, &data[SSL_MD5_DIGEST_LEN], SSL_SHA1_DIGEST_LEN);
	SHA1_Final(&pData[SSL_MD5_DIGEST_LEN], &sha);
	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLHShakeHashUpdate
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
					HandshakeType hType - (IN) The handshake message type
					UINT8 *pHData - (IN) The handhsake message
					INT32 lHShakeLength - (IN) The handshake message length
* Return Value	:	None.
* Description	:	This function updates the hash of the handshake messages.
******************************************************************************************/
void SSLHShakeHashUpdate(SSL *pSession, HandshakeType hType, UINT8 *pHData, 
							INT32 lHShakeLength)
{
	/* hello request is not included in handshake hashes */
	if(hType != HELLO_REQUEST && lHShakeLength != 0) {
		
		/* updates the MD5 handshake hash buffer */
		MD5_Update(&pSession->connectState.hShakeMd5Hash, pHData, lHShakeLength);

		/* updates the SHA1 handshake hash buffer */
		SHA1_Update(&pSession->connectState.hShakeSha1Hash, pHData, lHShakeLength);
	}
}


/*****************************************************************************************
* Function Name	:	SSLHShakeHashInit
* Parameters	:	SSL *pSession - (IN) The SSL session pointer
* Return Value	:	None.
* Description	:	This function initializes the hash of the handshake messages.
******************************************************************************************/
void SSLHShakeHashInit(SSL *pSession)
{
	/* initialize MD5 handshake hash buffer */
	MD5_Init(&pSession->connectState.hShakeMd5Hash);

	/* initialize SHA1 handshake hash buffer */
	SHA1_Init(&pSession->connectState.hShakeSha1Hash);
}

/*****************************************************************************************
* Function Name	:	SSLPHash
* Parameters	:	MACAlgorithm macAlgorithm - (IN) The mac algorithm used
					UINT8 *pSecret - (IN) The secret to be used
					INT32 lSecretSize - (IN) The size of the secret
					UINT8 *pSeed - (IN) The seed to be used for this operation
					INT32 lSeedSize - (IN) The seed size
					UINT8 *pResult - (OUT) The data generated
					INT32 lReqSize - (IN) The required data length
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function expands the input secret and sedd into required amount 
					of data
******************************************************************************************/
static INT32 SSLPHash(MACAlgorithm macAlgorithm, UINT8 *pSecret, INT32 lSecretSize, 
			   UINT8 *pSeed, INT32 lSeedSize, UINT8 *pResult, INT32 lReqSize)
{
	INT32 lCount;
	INT32 lNoOfTimes, lBlockSize, lPrevASize;
	UINT8 prevA[MAX_SEED_SIZE];
	UINT8 *pTemp;
	VarData hmacSecret;

	if (lSeedSize > MAX_SEED_SIZE || lReqSize <= 0) {
		return SSL_FAILURE;
	}

	/* implements 
	 P_hash(secret, seed) =	HMAC_hash(secret, A(1) + seed) +
                            HMAC_hash(secret, A(2) + seed) +
                            HMAC_hash(secret, A(3) + seed) + ...

	where  A() is defined as:
       A(0) = seed
       A(i) = HMAC_hash(secret, A(i-1))
	*/

	/* gets the hash block size to determine no of iterations of HMAC hashing */
	lBlockSize = SSLGetMessageDigestSize(macAlgorithm);
	if(lBlockSize == 0) {
		return SSL_FAILURE;
	}

	/* calculate the no. of iterations for HMAC hashing */
	lNoOfTimes = lReqSize/lBlockSize;
	lCount = lReqSize%lBlockSize;
	if(lCount != 0) {
		lNoOfTimes++;
	}

	/* copying A(0) */
	memcpy(prevA, pSeed, lSeedSize);
	lPrevASize = lSeedSize;

	/* secret for HMAC hashing */
	hmacSecret.pData = pSecret;
	hmacSecret.ulSize = lSecretSize;

	/* Allocate memory for storing the input for HMAC */
	pTemp = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lBlockSize+lSeedSize);
	
	if(pTemp == NULL) {
		return SSL_FAILURE;
	}
	/* copying the seed for HMAC iterations */
	memcpy(pTemp+lBlockSize,pSeed,lSeedSize);

	
	for (lCount = 0; lCount < lNoOfTimes; lCount++) {
		/* perform HMAC hash to get A(i)*/
		SSLHmacSign(macAlgorithm,prevA,lPrevASize,&hmacSecret,prevA,lBlockSize);
		lPrevASize = lBlockSize;

		/* copying A(i) */
		memcpy(pTemp,prevA,lPrevASize);

		if ( (lCount+1) * lBlockSize <= lReqSize) {
			/* perform actual HMAC hash */
			SSLHmacSign(macAlgorithm,pTemp,lBlockSize+lSeedSize,&hmacSecret,
				&pResult[lCount * lBlockSize], lBlockSize);
		} else {
			/* last HMAC hash, the output buffer is small. Take the HMAC hash 
			output in pTemp and copy only the remaining required size to pResult */ 
			SSLHmacSign(macAlgorithm,pTemp,lBlockSize+lSeedSize,&hmacSecret,
				pTemp, lBlockSize);
			memcpy(&pResult[lCount * lBlockSize], pTemp, lReqSize - lCount * lBlockSize);
		}
	}
	OS_ADPT_Deallocate_Memory(pTemp);
	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLXor
* Parameters	:	UINT8 *pInput1 - (IN/OUT) The first data stream
					UINT8 *pInput2 - (IN) The second data stream
					INT32 lLength - (IN) The size of the data stream
* Return Value	:	None.
* Description	:	This function performs XOR operation between two data streams
******************************************************************************************/
static void SSLXor(UINT8 *pInput1, UINT8 *pInput2, INT32 lLength)
{
	INT32 lCount;

	for (lCount = 0; lCount < lLength; lCount++) {
		/* Xors the pInput1 data stream with pInput2 and stores the result in pInput1 */
		pInput1[lCount] ^= pInput2[lCount];
	}
}

/*****************************************************************************************
* Function Name	:	SSLGeneratePRF
* Parameters	:	UINT8 *pSecret - (IN) The secret to be used
					INT32 lSecretSize - (IN) The secret size
					UINT8 *pLabel - (IN) The label to be used
					INT32 lLabelSize - (IN) The label size
					UINT8 *pSeed - (IN) The seed size
					INT32 lSeedSize - (IN) The seed size
					UINT8 *pResult - (OUT) The random data generated
					INT32 lReqSize - (IN) The data size required
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function expands the secret, seed and label to the required
					amount of data.
******************************************************************************************/
INT32 SSLGeneratePRF(UINT8 *pSecret, INT32 lSecretSize, INT8 *pLabel, INT32 lLabelSize, 
		   UINT8 *pSeed, INT32 lSeedSize, UINT8 *pResult, INT32 lReqSize)
{
	INT32 lSecertHalf;
	INT32 lLabelSeedSize;
	UINT8 *pSecret1, *pSecret2;
	UINT8 *pSeedLabel;
	UINT8 *pSha1Bytes;

	/* implements 
	PRF(secret, label, seed) = P_MD5(S1, label + seed) XOR
                               P_SHA-1(S2, label + seed);

	where 
	P_MD5 and P_SHA-1 are calculated using SSLPHash function
	S1, S2 are generated as follows
  
		L_S = length in bytes of secret;
		L_S1 = L_S2 = ceil(L_S / 2);
	
		The secret is partitioned into two halves (with the possibility of 
		one shared byte) , S1 taking the first L_S1 bytes 
		and S2 the last L_S2 bytes.
	*/

	/* size of the label+seed buffer */
	lLabelSeedSize = lSeedSize + lLabelSize;

	pSeedLabel = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lLabelSeedSize);
	if(pSeedLabel == NULL) {
		return SSL_FAILURE;
	}

	/* memory for SHA - PHash */
	pSha1Bytes = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lReqSize);
	if(pSha1Bytes == NULL) {
		OS_ADPT_Deallocate_Memory(pSeedLabel);
		return SSL_FAILURE;
	}

	/* make the label+seed buffer */
	memcpy(pSeedLabel, pLabel, lLabelSize);
	memcpy(&pSeedLabel[lLabelSize], pSeed, lSeedSize);

	/* split the secret into S1 and S2 */
	lSecertHalf = lSecretSize / 2;

	if(lSecretSize > 0) {
		pSecret1 = &pSecret[0];
		pSecret2 = &pSecret[lSecertHalf];
		
		if (lSecretSize % 2 != 0) {
			lSecertHalf++;
		}
	}

	/* take MD5 PHash */
	if(SSLPHash(MD5_MAC, pSecret1, lSecertHalf, pSeedLabel, lLabelSeedSize,
		pResult, lReqSize) != SSL_SUCCESS) {
		OS_ADPT_Deallocate_Memory(pSeedLabel);
		OS_ADPT_Deallocate_Memory(pSha1Bytes);
		return SSL_FAILURE;
	}


	/* take SHA1 PHash */
	if(SSLPHash(SHA1_MAC, pSecret2, lSecertHalf, pSeedLabel, lLabelSeedSize, 
		pSha1Bytes, lReqSize) != SSL_SUCCESS) {
		OS_ADPT_Deallocate_Memory(pSeedLabel);
		OS_ADPT_Deallocate_Memory(pSha1Bytes);
		return SSL_FAILURE;
	}

	/* Xor MD5 PHash and SHA1 PHash */
	SSLXor(pResult, pSha1Bytes, lReqSize);

	OS_ADPT_Deallocate_Memory(pSeedLabel);
	OS_ADPT_Deallocate_Memory(pSha1Bytes);

	return SSL_SUCCESS;

}

/*****************************************************************************************
* Function Name	:	SSLv3ShaHash
* Parameters	:	INT32 lIterationCount - (IN) The iteration count of this operation
					UINT8 *pSecret - (IN) The secret to be used
					INT32 lSecretSize - (IN) The secret size
					UINT8 *pRandom - (IN) The random data
					INT32 lRandomSize - (IN) The random data size
					UINT8 *pDigest - (OUT) The resulting SHA1 digest
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function performs SHA1 hash on the iput data along with alphabet
					streams based on iteration count.
******************************************************************************************/
static INT32 SSLv3ShaHash(INT32 lIterationCount, UINT8 *pSecret, INT32 lSecretSize, 
					UINT8 *pRandom, INT32 lRandomSize, UINT8 *pDigest)
{
	INT32 lCount;
	UINT8 alphabets[NO_OF_ALPHABETS];
	UINT8 *pData;
	
	/* implements 
	SHA(Alphabet Sequence + secret + ClientHello.random + ServerHello.random)) 
	*/

	for (lCount = 0; lCount < lIterationCount + 1; lCount++) {
		/* makes a series of A, BB, CCC, DDDD, .....
		for A lIterationCount = 0 */
		alphabets[lCount] = 65 + lIterationCount;	/* A==65 */
	}

	/* allocate memory for SHA input data */
	pData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, lSecretSize+lRandomSize+(lIterationCount+1));
	if(pData == NULL) {
		return SSL_FAILURE;
	}

	/* conacatenates laphabet sequence, secret and randoms */
	memcpy(pData ,alphabets, lIterationCount+1);
	memcpy(&pData[lIterationCount+1] ,pSecret, lSecretSize);
	memcpy(&pData[lSecretSize+lIterationCount+1] ,pRandom, lRandomSize);

	/* get the SHA1 digest */
	SHA1Sign(pData,lSecretSize+lRandomSize+(lIterationCount+1),pDigest);

	OS_ADPT_Deallocate_Memory(pData);
	
	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLv3Md5Hash
* Parameters	:	INT32 lIterationCount - (IN) The iteration count of this operation
					UINT8 *pSecret - (IN) The secret to be used
					INT32 lSecretSize - (IN) The secret size
					UINT8 *pRandom - (IN) The random data
					INT32 lRandomSize - (IN) The random data size
					UINT8 *pDigest - (OUT) The resulting SHA1 digest
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function performs MD5 hash on SHA1 digest of iput data 
					along with alphabet streams based on iteration count.
******************************************************************************************/
static INT32 SSLv3Md5Hash(INT32 lIterationCount, UINT8 *pSecret, INT32 lSecretSize, 
					UINT8 *pRandom, INT32 lRandomSize, UINT8 *pDigest)
{
	UINT8 *pMd5Input;

	/* implements 
	MD5(secret + SHA('A' + pre_master_secret +
           ClientHello.random + ServerHello.random)) 
	*/
	
	/* allocate memory for the input data of MD5 hash */
	pMd5Input = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, 
		SSL_SHA1_DIGEST_LEN + lSecretSize); 

	/* conactenate secret and SHA1 digest */
	memcpy(pMd5Input, pSecret, lSecretSize);

	/* append SHA1 digest with secret */
	if (SSLv3ShaHash(lIterationCount, pSecret, lSecretSize, pRandom, lRandomSize, 
		&pMd5Input[lSecretSize]) != SSL_SUCCESS) {
		OS_ADPT_Deallocate_Memory(pMd5Input);
		return SSL_FAILURE;
	}

	/* gets the MD5 digest */
	MD5Sign(pMd5Input,SSL_SHA1_DIGEST_LEN + lSecretSize,pDigest);
	OS_ADPT_Deallocate_Memory(pMd5Input);

	return SSL_SUCCESS;
}


/*****************************************************************************************
* Function Name	:	SSL3GenerateRandom
* Parameters	:	UINT8 *pSecret - (IN) The secret to be used
					INT32 lSecretSize - (IN) The secret size
					UINT8 *pRandom - (IN) The random data
					INT32 lRandomSize - (IN) The random data size
					UINT8 *pResult - (OUT) The data expanded
					INT32 lReqSize - (IN) The required data size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function expands the input secret and seed into required
					data size using MD5 and SHA1 hashes
******************************************************************************************/
INT32 SSL3GenerateRandom(UINT8 *pSecret, INT32 lSecretSize, UINT8 *pRandom,
					  INT32 lRandomSize, UINT8 *pResult, INT32 lReqSize)
{
	INT32 lCurSize;
	INT32 lIterationCount;
	UINT8 digest[SSL_MD5_DIGEST_LEN];
	INT32 lResult;

	/* implements
	MD5(secret + SHA(`A' + secret + random)) +
	MD5(secret + SHA(`BB' + secret + random)) +
	MD5(secret + SHA(`CCC' + secret + random)) + [...];
	*/

	lCurSize = 0;
	lIterationCount = 0;
	lResult = SSL_SUCCESS;
	/* continue the sequence until required number of bytes are produced */
	while (lCurSize < lReqSize) {
		/* takes the outer MD5 hash */
		lResult = SSLv3Md5Hash(lIterationCount, pSecret, lSecretSize, pRandom, lRandomSize,
			digest);
		if(lResult != SSL_SUCCESS) {
			break;
		}
		/* increase the size of current generated data */
		lCurSize += SSL_MD5_DIGEST_LEN;
		
		/* copy the MD5 digest to the result */
		memcpy(&pResult[lCurSize - SSL_MD5_DIGEST_LEN], digest,
			lCurSize > lReqSize ? (lReqSize % SSL_MD5_DIGEST_LEN) : SSL_MD5_DIGEST_LEN);
		lIterationCount++;

		/* check the iteration does not exceed the no of alphabets ... this
		algorithm can't be used to generate more than 
		NO_OF_ALPHABETS(26)*SSL_MD5_DIGEST_LEN(16) bytes of data */
		if(lIterationCount >= NO_OF_ALPHABETS) {
			lResult = SSL_FAILURE;
			break;
		}
	}
	return lResult;
}

