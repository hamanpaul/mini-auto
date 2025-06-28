/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Description			:	This file contains structures and information needed only
							for SSL source files

* Author Name			:	M.K.Saravanan

******************************************************************************************/

#ifndef SSLINTERNAL_H
#define SSLINTERNAL_H

typedef enum {
	CHANGE_CIPHER_SPEC = 20, ALERT = 21, HANDSHAKE = 22, APPLICATION_DATA = 23
}ContentType;

typedef enum { 
	CIPHER_STREAM=1, CIPHER_BLOCK 
} CipherType;

typedef enum {
	TYPE_CHANGE_CIPHER_SPEC=1 
} ChangeCipherSpecType;

typedef enum {
	CLOSE_NOTIFY_ALERT=0, UNEXPECTED_MESSAGE_ALERT=10, 
	BAD_RECORD_MAC_ALERT=20, DECRYPTION_FAILED_ALERT=21, 
	RECORD_OVERFLOW_ALERT=22, DECOMPRESSION_FAILURE_ALERT=30,
	HANDSHAKE_FAILURE_ALERT=40, SSL3_NO_CERTIFICATE_ALERT=41,
	BAD_CERTIFICATE_ALERT=42, UNSUPPORTED_CERTIFICATE_ALERT=43,
	CERTIFICATE_REVOKED_ALERT=44, CERTIFICATE_EXPIRED_ALERT=45, 
	CERTIFICATE_UNKNOWN_ALERT=46, ILLEGAL_PARAMETER_ALERT=47, 
	UNKNOWN_CA_ALERT=48, ACCESS_DENIED_ALERT=49, DECODE_ERROR_ALERT=50,
	DECRYPT_ERROR_ALERT=51, EXPORT_RESTRICTION_ALERT=60, 
	PROTOCOL_VERSION_ALERT=70, INSUFFICIENT_SECURITY_ALERT=71, 
	INTERNAL_ERROR_ALERT=80, USER_CANCELED_ALERT=90,
	NO_RENEGOTIATION_ALERT=100
} AlertDescription;

typedef enum {
	AL_WARNING=1, AL_FATAL=2
} AlertLevel;


#define SSL_TRUE	1
#define SSL_FALSE	0

#define SSL_NO_PEEK			0
#define SSL_PEEK			1

#define SSL3_VER_MAJOR		3
#define SSL3_VER_MINOR		0
#define TLS1_VER_MAJOR		3
#define TLS1_VER_MINOR		1


/* record layer macros */
#define	RECORD_HEADER_SIZE	5
#define MAX_RECORD_SIZE		16384
#define MAX_CIPHERPAD_SIZE	2048
#define MAX_RECORD_OVERHEAD	MAX_CIPHERPAD_SIZE+RECORD_HEADER_SIZE
#define MAX_RECV_SIZE		MAX_CIPHERPAD_SIZE+MAX_RECORD_SIZE



/* Record header Positions */
#define CONTENT_TYPE_POS	0
#define VERSION_MAJOR_POS	1
#define	VERSION_MINOR_POS	2
#define RECORD_LENGTH_POS	3



/* change cipher spec macros */
#define CHANGE_CIPHER_SPEC_SIZE 1


/* alert protocol macros */
#define ALERT_LEVEL_POS				0
#define ALERT_CODE_POS				1
#define ALERT_SIZE					2

/* handshake layer macros */
#define HANDSHAKE_HEADER_SIZE		4
#define HSHAKE_TYPE_POS				0
#define HSHAKE_LENGTH_POS			1

#define PROTOCOL_VERSION_SIZE		2
#define SSL_RANDOM_SIZE				32
#define SSL_MAX_SESSION_ID_SIZE		32
#define SESSION_ID_LENGTH_SIZE		1
#define CIPHER_LIST_LENGTH_SIZE		2
#define COMP_LIST_LENGTH_SIZE		1
#define SSL_CIPHER_SIZE				2
#define SSL_COMP_SIZE				1
#define CERTIFICATE_MSG_LEN			3
#define TLS_FINISHED_SIZE			12
#define SSL3_FINISHED_SIZE			36
#define SERVER_FINSIHED_LABEL		"server finished"
#define CLIENT_FINSIHED_LABEL		"client finished"
#define TLS_FINISHED_LABEL_LEN		15
#define SSL3_CLIENT_FINISHED_LABEL	"CLNT"
#define SSL3_SERVER_FINISHED_LABEL	"SRVR"
#define SSL3_FINISHED_LABEL_LEN		4
#define TLS_CLIENT_KEYEX_LEN_SIZE	2
#define SSL_DH_PARAMS_LEN_SIZE		2
#define SSL_RSA_PARAMS_LEN_SIZE		2
#define SSL_SIGNATURE_LEN_SIZE		2
#define SSL_CLIENT_DH_LEN_SIZE		2
#define SSL_RSA_EXPORT_KEY_LEN		64
#define SSL_DN_LEN_SIZE				2

#define TLS_KEY_EXPANSION_LABEL		"key expansion"
#define KEY_EXPANSION_LABEL_SIZE	13

#define TLS_CLIENT_WRITE_KEY_LABEL		"client write key"
#define CLIENT_WRITE_KEY_LABEL_SIZE		16

#define TLS_SERVER_WRITE_KEY_LABEL		"server write key"
#define SERVER_WRITE_KEY_LABEL_SIZE		16

#define TLS_IV_BLOCK_LABEL				"IV block"
#define IV_BLOCK_LABEL_SIZE				8

#define TLS_MASTER_SECRET_LABEL		"master secret"
#define MASTER_SECRET_LABEL_SIZE	13

#define SSL2_LENGTH_SIZE			2
#define SSL2_RECORD_HEADER_SIZE		2
#define SSL2_CIPHERSPEC_SIZE		3
#define SSL2_MAX_SESSIONID_LEN		16
#define SSL2_MIN_CHALLENGE_LEN		16
#define SSL_MSG_TYPE_LEN			1


#define DECR_VALIDATE_LEN(length, decr) length-=decr; \
if (length<0) {return SSL_FAILURE;}

#define CHECK_RETURN_VALUE(lReturnValue) if (lReturnValue <= 0) { \
	return lReturnValue;}


/* function prototypes */
INT32 SSLWriteToTransport(INT32 lSocketDescr, UINT8 *pData, INT32 lSize);
INT32 SSLReadFromTransport(INT32 lSocketDescr, UINT8 *pData, INT32 lDataSize);
INT32 SSLRand(UINT8 *pRandomBytes, UINT32 ulNoOfBytes);

INT32 SSLGetBufferSize(SSLBuffer *pBuffer);
INT32 SSLInsertBuffer(SSLBuffer *pBuffer, UINT8 *pData, INT32 lDataSize);
INT32 SSLRetrieveBuffer(SSLBuffer *pBuffer, UINT8 *pData, INT32 lDataSize);
INT32 SSLPeekBuffer(SSLBuffer *pBuffer, UINT8 *pData, INT32 lDataSize);

UINT32 SSLSwap32(UINT32 ulNumber);
UINT16 SSLSwap16(UINT16 uNumber);
void SSLWriteUint16( UINT16 uNumber, UINT8 *pData);
UINT16 SSLReadUint16(UINT8 *pData);
void SSLWriteUint64( SSLU64 *pNumber, UINT8 *pData);
void SSLZeroUint64(SSLU64 *pNumber);
INT32 SSLIncrementUint64(SSLU64 *pU64Num);
UINT32 SSLReadUint24(UINT8 *pData);
void SSLWriteUint24(UINT32 ulNumber, UINT8 *pData);

INT32 SSLWriteRecordBuffered(SSL *pSession, UINT8 *pData, INT32 lDataSize);
INT32 SSLGenerateCipherText(SSL *pSession,ContentType cType,void *pData,INT32 lDataSize,
							UINT8 *pCiphertext,INT32 lMaxCipherLength);
INT32 SSLRecordSend (SSL * pSession, ContentType type, void *pData, INT32 lSize);
INT32 SSLReadRecordBuffered(SSL *pSession, UINT8 *pData, INT32 lDataSize, INT32 lPeek);
INT32 SSLCheckRecordHeader(SSL *pSession, UINT8 *pHeaders, ContentType cType, 
						   HandshakeType hType, ContentType *pRecvCType, 
						   SSLVersion *pVersion, UINT16 *pLength, INT32 *lRecordHdrSize);
INT32 SSLReadRecord(SSL *pSession, ContentType cType, ContentType *pRevType, 
					HandshakeType hType, void **pData, INT32 *pSize, INT32 *pRecordHdrSize);
INT32 SSLGeneratePlainText(SSL *pSession, UINT8 *pCipherText, INT32 lCipherSize,
					 UINT8 *pData, INT32 lDataSize, ContentType cType);
INT32 SSLCheckRecvRecordType(SSL *pSession, ContentType cType, ContentType recvCType, 
							UINT8 *pData, INT32 lDataSize);
INT32 SSLGetRecordBufferSize(SSL *pSession, ContentType cType);
INT32 SSLGetRecordBuffer(SSL *pSession, ContentType cType, UINT8 *pData, INT32 lDataSize);
INT32 SSLCheckRecordBuffers(SSL *pSession, ContentType cType, UINT8 *pData, 
							INT32 lDataSize);
INT32 SSLPutRecordBuffer(SSL *pSession, ContentType cType, UINT8 *pData, 
						 INT32 lDataLength);
INT32 SSLRecordRecv (SSL * pSession, ContentType cType, HandshakeType hType, void *pData, 
					 INT32 lSize);


INT32 SSLGetMessageDigestSize(MACAlgorithm macAlgorithm);
INT32 SSLGetCipherKeySize(BulkCipherAlgorithm cipherAlgorithm);
INT32 SSLGetExportCipherKeySize(BulkCipherAlgorithm cipherAlgorithm);
INT32 SSLGetCipherIVSize(BulkCipherAlgorithm cipherAlgorithm);
UINT8 SSLGetPaddingLength(BulkCipherAlgorithm cipherAlgorithm, INT32 lDataSize);
CipherType SSLGetCipherType(BulkCipherAlgorithm cipherAlgorithm);
INT32 SSLGetStreamStateSize(BulkCipherAlgorithm cipherAlgorithm);
void SSLInitStreamCipherState(SSL *pSession);
INT32 SSLGetCipherBlockSize(BulkCipherAlgorithm cipherAlgorithm);
INT32 SSLEncrypt(BulkCipherAlgorithm cipherAlgorithm, UINT8 *pData, INT32 lDataSize, 
				  VarData *pWriteKey, VarData *pWriteIV, UINT8 *pOutputCipher, 
				  INT32 ulCipherLen);
INT32 SSLDecrypt (BulkCipherAlgorithm cipherAlgorithm, UINT8 *pCipher, INT32 lCipherSize, 
				  VarData *pReadKey, VarData *pReadIV, UINT8 *pData, INT32 ulDataLen);
INT32  SSL3MacSign(MACAlgorithm macAlgorithm, VarData  *pMacSecret, SSLU64		*pSeqNum,
				   ContentType	cType, UINT8	*pData, UINT32	ulDataLen,
				   UINT8	*pAuthParams, INT32	lAuthParamsLen);
INT32  SSLHmacSign(MACAlgorithm macAlgorithm, UINT8    *pData, UINT32	ulDataLen,
				   VarData	*pMacSecret, UINT8	*pAuthParams, INT32	lAuthParamsLen);
INT32 SSLGenerateMac (MACAlgorithm macAlgorithm,VarData *pMacSecret, SSLU64 *pSeqNum, 
					  ContentType cType, SSLVersion version, UINT8 *pData,
					  INT32 lDataSize, VarData *pOutputMac);
INT32 SSLTlsv1Finished(SSL *pSession, ConnectionEnd isServer, UINT8 *pData);
INT32 SSLv3Finished(SSL *pSession, ConnectionEnd isServer, UINT8 *pData);
void SSLHShakeHashUpdate(SSL *pSession, HandshakeType hType, UINT8 *pHData, 
							INT32 lHShakeLength);
void SSLHShakeHashInit(SSL *pSession);
INT32 SSLPHash(MACAlgorithm macAlgorithm, UINT8 *pSecret, INT32 lSecretSize, 
			   UINT8 *pSeed, INT32 lSeedSize, UINT8 *pResult, INT32 lReqSize);
void SSLXor(UINT8 *pInput1, UINT8 *pInput2, INT32 lLength);
INT32 SSLGeneratePRF(UINT8 *pSecret, INT32 lSecretSize, INT8 *pLabel, INT32 lLabelSize, 
		   UINT8 *pSeed, INT32 lSeedSize, UINT8 *pResult, INT32 lReqSize);
INT32 SSLv3ShaHash(INT32 lIterationCount, UINT8 *pSecret, INT32 lSecretSize, 
					UINT8 *pRandom, INT32 lRandomSize, UINT8 *pDigest);
INT32 SSLv3Md5Hash(INT32 lIterationCount, UINT8 *pSecret, INT32 lSecretSize, 
					UINT8 *pRandom, INT32 lRandomSize, UINT8 *pDigest);
INT32 SSL3GenerateRandom(UINT8 *pSecret, INT32 lSecretSize, UINT8 *pRandom,
					  INT32 lRandomSize, UINT8 *pResult, INT32 lReqSize);


INT32 SSLProcessRSAClientKeyEx(SSL *pSession, UINT8 *pHData, INT32 lDataSize);
INT32 SSLProcessClientKeyEx(SSL *pSession, UINT8 *pHData, INT32 lDataSize);
INT32 SSLServerKeyExNeeded(SSL *pSession);
INT32 SSLServerCertNeeded(SSL *pSession);
INT32 SSLCertVerifyNeeded(SSL *pSession);
INT32 SSLGenerateServerKeyEx(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize);
INT32 SSLGenerateRSAExportServerKeyEx(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize);
INT32 SSLGenerateDHServerKeyEx(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize);
INT32 SSLGenerateSignature(SSL *pSession, UINT8 *pData, INT32 lDataSize, 
						   UINT8 **pSignature, INT32 *pSignatureLen);
INT32 SSLProcessDHEClientKeyEx(SSL *pSession, UINT8 *pHData, INT32 lDataSize);
INT32 SSLPremasterToMaster(SSL *pSession, UINT8 *pPremaster, INT32 lPremasterSize);




INT32 SSLRestoreSession(SSL *pSession, UINT8 *pSessionID,INT32 lSessionIDLen);
INT32 SSLRemoveSession(SSL *pSession);
INT32 SSLRegisterSession(SSL *pSession);
INT32 SSLIsServerCaching(SSL *pSession);
INT32 SSLSelectCipherSuite(SSL *pSession, UINT8 *pCipherPtr, INT32 cipherListSize);
INT32 SSLSelectV2CipherSuite(SSL *pSession, UINT8 *pCipherPtr, INT32 cipherListSize);
void  SSLHandshakeBuffersClear(SSL *pSession);
INT32 SSLSendChangeCipherSpec( SSL *pSession);
INT32 SSLRecvChangeCipherSpec( SSL *pSession);
INT32 SSLSendAlert(SSL *pSession, AlertLevel level, AlertDescription desc);
INT32 SSLGenerateSessionKeys(SSL *pSession);
INT32 SSLGenerateExportSessionKeys(SSL *pSession);
INT32 SSLGetNoOfCipherSuites(void);
void SSLReadConnectionStateInit(SSL *pSession);
void SSLWriteConnectionStateInit(SSL *pSession);
void SSLFreeDHParams(SSL *pSession);


INT32 SSLWriteHShakeBuffered(SSL *pSession, UINT8 *pHData, INT32 lDataSize);
INT32 SSLReadHShakeBuffered(SSL *pSession, HandshakeType hType, UINT8 *pData, 
							INT32 lDataSize, INT32 lPeek);
INT32 SSLGenerateFinished(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize);
INT32 SSLGenerateCertificate(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize);
INT32 SSLGenerateServerHello(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize);
INT32 SSLProcessFinished(SSL *pSession, UINT8 *pHData, INT32 lDataSize);
INT32 SSLProcessClientHello(SSL *pSession, UINT8 *pHData, INT32 lDataSize);
INT32 SSLProcessV2ClientHello(SSL *pSession, UINT8 *pHData, INT32 lDataSize);
INT32 SSLSendHandshake(SSL *pSession, HandshakeType hType);
INT32 SSLRecvHandshake(SSL *pSession, HandshakeType hType);
INT32 SSLHandshakeFinal(SSL *pSession);
INT32 SSLGenerateCertRequest(SSL *pSession, UINT8 **pHData, INT32 *pHDataSize);
INT32 SSLProcessCertificate(SSL *pSession, UINT8 *pHData, INT32 lDataSize);
INT32 SSLProcessCertVerify(SSL *pSession, UINT8 *pHData, INT32 lDataSize);
INT32 SSLVerifySignature(SSL *pSession, UINT8 *pSignature, INT32 lSignatureLen);
void SSLGetCertVerifyDigest(SSL *pSession, UINT8 *pDigest);

#endif /* SSLINTERNAL_H */


