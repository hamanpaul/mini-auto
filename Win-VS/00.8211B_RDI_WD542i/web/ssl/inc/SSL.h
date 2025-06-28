/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Description			:	This file contains include information for SSL Applications

* Author Name			:	M.K.Saravanan

******************************************************************************************/

#ifndef SSL_H
#define SSL_H

#include <SSLCfg.h>
#include <CryptCmn.h>
#include <MD5.h>
//#include <../Crypto/common/inc/md5.h>

#include <SHA1.h>
#include <DES.h>
#include <rand.h>
#include <RSA.h>
#include <Certlib.h>
#include <RC4.h>
#include <DH.h>
#include <DSA.h>

//#include <DLSAgent/h/UsmMD5.h>

#define SSL_FAILURE				0
#define SSL_SUCCESS				1
#define SSL_ERROR				-1
#define SSL_WOULD_BLOCK			-2
#define SSL_CONNECTION_CLOSED	0

#define	MAX_RANDOM_SIZE			32
#define	MASTER_SECRET_SIZE		48

//BEGIN - YKS & Debu... 16/01/2004
#define	SSL_MIN_TIMEOUT			(1 * 60)		//1 minute
#define	SSL_MAX_TIMEOUT			(24 * 60 * 60)	//24 hours
#define	SSL_CONVERT_MILLISEC	1000
#define	SSL_TIMER_TIMEOUT		(10 * 60)	//YKS & Debu... 13/01/2004... 600 minutes - unit seconds
//END - YKS & Debu... 16/01/2004

typedef enum {
	SSL_SERVER=1, SSL_CLIENT 
} ConnectionEnd;

typedef enum {
	NULL_CIPHER=1, RC4_128_CIPHER, RC4_40_CIPHER, TRIPLEDES_CIPHER 
} BulkCipherAlgorithm;

typedef enum { 
	NULL_MAC=1, MD5_MAC, SHA1_MAC 
} MACAlgorithm;

typedef enum { 
	SSL_KX_RSA=1, SSL_KX_RSA_EXPORT, SSL_KX_DHE_DSS
} KXAlgorithm;

typedef enum { 
	SSL_VERSION3 = 1, TLS_VERSION1, SSL_VERSION_UNKNOWN=0xff 
} SSLVersion;

typedef enum { 
	RESUME_TRUE=1, RESUME_FALSE 
} ResumableSession;

typedef enum { 
	VALID_SESSION=1, INVALID_SESSION 
} ValidSession;

typedef enum {
	INIT_STATE=1, SERVERHELLO_STATE, CERTIFICATE_STATE, SERVERKEYEX_STATE, 
	REQUEST_CERT_STATE, HELLODONE_STATE, CLIENT_CERT_STATE, CLIENTKEYEX_STATE, 
	CERT_VERIFY_STATE, SEND_FINISHED_STATE, SEND_CHANGE_CIPHER_STATE, RECV_FINISHED_STATE, 
	RECV_CHANGE_CIPHER_STATE, APPLICATION_STATE, CLOSE_STATE, CLEAR_STATE
} ServerHandshakeState;

typedef enum { 
	RSA_SIGN=1, DSS_SIGN, RSA_FIXED_DH, DSS_FXED_DH 
} CertificateType;

typedef enum { 
	NOT_HANDSHAKE=-1, HELLO_REQUEST=0, CLIENT_HELLO=1, SERVER_HELLO=2,CERTIFICATE=11, SERVERKEYEX=12, 
	CERTIFICATE_REQUEST=13, HELLO_DONE=14, CERTIFICATE_VERIFY=15, CLIENTKEYEX=16, FINISHED=20 
} HandshakeType;

typedef enum {
	NULL_COMPRESSION = 0
} CompressionMethod;

typedef enum {
	EXPORTABLE = 1, NOT_EXPORTABLE
} IsExportable;


typedef UINT8 CipherSuite[2];

typedef struct {
	UINT8 u64Byte[8];
} SSLU64;

typedef struct {
	UINT8 	*pData;
	UINT32 	ulSize;
} VarData;


typedef struct {
	CipherSuite 			cipherSuite;	/* cipher suite */
	CertificateType			certType;		/* certificate type needed for this cipher suite */
	KXAlgorithm				kxAlgorithm;	/* key exchange algorithm used */
	BulkCipherAlgorithm 	cipherAlgorithm;	/* cipher used for decryption */
	MACAlgorithm 			macAlgorithm;	/* MAC algorithm used for incoming messages */
	SSLVersion				lowestVersion;	/* lowest version from which cipher suite is available */
	INT8					cipherEnabled;	/* flag to indicate cipher is enabled or disabled */
	IsExportable			export;			/* whether has export status or not */
} CipherSuiteMappings;


typedef struct SSL_MainDataStructure	SSL;	//Added... YKS & Debu... 13/01/2004

typedef struct {
	UINT8 					masterSecret[MASTER_SECRET_SIZE];	/* master Secret */
	CipherSuiteMappings		*pCipherMapping;					/* cipher suite used */
	SSLVersion				version;							/* SSL version */
	SSLVersion				clientVersion;						/* Latest SSL version used by Client */
}SSLSessionCache;

typedef VarData SSLBuffer;

typedef struct {
	ConnectionEnd 			entity;		/* client or server*/
	VarData					sessionId;		/* Session Identifier*/
	BulkCipherAlgorithm		readBulkCipherAlgorithm;/* cipher used for decryption */
	MACAlgorithm 			readMacAlgorithm;	/* MAC algorithm used for incoming messages */
	VarData					readMacSecret;	/* MAC Key used for incoming messages */
	VarData					readKey;		/* cipher key used for Decryption */
	VarData					readIV;		/* Initialization vector used for decryption */
	SSLU64					readSequenceNumber;/* Sequence no. used in read operations */
	BulkCipherAlgorithm		writeBulkCipherAlgorithm;/* cipher used for encryption */
	MACAlgorithm 			writeMacAlgorithm;/* MAC algorithm used for outgoing messages*/
	VarData					writeMacSecret;	/* MAC Key used in outgoing messages */
	VarData					writeKey;		/* cipher key used for encryption */
	VarData					writeIV;		/* Initialization vector used for encryption */
	SSLU64					writeSequenceNumber;/* sequence no. used in write operations */
	UINT8 					clientRandom[MAX_RANDOM_SIZE];	/* random selected by client */
	UINT8 					serverRandom[MAX_RANDOM_SIZE]; /* random selected by server */
} SSLSecurityParameters;

typedef void *(SSL_RETR_FUNC) (UINT8 *pSessionID, UINT32 ulIDLength);
typedef INT32 (SSL_STORE_FUNC) (UINT8 *pSessionID, UINT32 ulIDLength, UINT8 *pData, UINT32 ulDataLength);
typedef void (SSL_REMOVE_FUNC) (UINT8 *pSessionID, UINT32 ulIDLength);

typedef struct {
	SSLBuffer				applicationData;	/* application data buffer */
	SSLBuffer				handshakeData;		/* handshake data */
	SSLBuffer				recordSendBuffer;
	SSLBuffer				recordRecvBuffer;
	INT32					lRecordSendUserSize;
	SSLBuffer				handshakeSendBuffer;
	SSLBuffer				handshakeRecvBuffer;
	MD5_CTX					hShakeMd5Hash;
	SHA_CTX					hShakeSha1Hash;
	INT32					lhandshakeSendPrevSize;
	INT32					v2Hello;
	ResumableSession		resumed;		/* resumed or not */
	ResumableSession		resumable;		/* resumable or not */
	ServerHandshakeState	hsState;		/* handshake state */
	ValidSession			valid;			/* valid session */
	UINT8					lastAlert;		/*last alert received*/
	INT32					lSocketDescr;	/* socket descriptor */
	VarData					keyBlock;		/* keys generated */
	SSL_STORE_FUNC 			*pStoreFunc;		/*callback function for storing the session*/
	SSL_RETR_FUNC 			*pRetrFunc;		/*callback function for retrieving session */
	SSL_REMOVE_FUNC			*pRemFunc;		/* callback function for removing session */
#if SSL_CFG_REQUEST_CERTIFICATE
	INT8					certRequested;	/* Inorder to store whether cert is requested within a session 
											as gRequestCertificate may get chnaged within  session */
	struct Certificate		*pClientCert;
#endif
}SSLConnection;


typedef struct {
	union {
		void *pTemp;
#if SSL_CFG_DH_ENABLED
		struct {
			/* DH parameters */
			BIGNUM	*prime;		/* prime used in DH key exchange */
			BIGNUM 	*generator;	/* generator used in DH keyex */
			BIGNUM	*x;			/* server's secret value */
			BIGNUM	*X;			/* server's public value */
		}DH;
#endif /* SSL_CFG_DH_ENABLED */
	}KeyExAlgorithm;
}SSLKeyExchange;


//Modified... YKS & Debu... 13/01/2004
struct SSL_MainDataStructure{
	SSLSessionCache			sessionCache;	/* cache for this session */
	SSLSecurityParameters 	secParams;		/* SSL security parameters */
	SSLConnection			connectState;	/* connection state holding SSL data */
	SSLKeyExchange			keyExParams;	/* Key exchange parameters */
};

#define RSA_WITH_RC4_128_MD5			0x0004
#define RSA_WITH_3DES_EDE_CBC_SHA		0x000A
#define DHE_DSS_WITH_3DES_EDE_CBC_SHA	0x0013
#define RSA_EXPORT_WITH_RC4_40_MD5		0x0003

/* Function Prototypes */
INT32 SSLSend (SSL * pSession, void *pData, INT32 lSize);
INT32 SSLRecv (SSL * pSession, void *pData, INT32 lSize);
INT32 SSLGetPendingDataSize(SSL *pSession);
SSL *SSLCreateSession(void);
INT32 SSLCloseSession(SSL *pSession);
void SSLLibraryInit(void);
INT32 SSLSetTransportPtr(SSL *pSession, INT32 lSocketDescr);
void SSLSetSessionResumeFunc ( SSL *pSession, SSL_RETR_FUNC *pRetrFunc, 
							  SSL_STORE_FUNC *pStoreFunc, SSL_REMOVE_FUNC *pRemFunc);
INT32 SSLSetCipherSuitePriority(UINT16 cipherSuite);
INT32 SSLServerHandshake(SSL *pSession);


#endif /* SSL_H */
