/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	encrptyapi.h

Abstract:

   	The declarations of log file.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/05/6   Elsa Lee    Create

*/

#ifndef __ENCRPTYAPI_H__
#define __ENCRPTYAPI_H__

extern  OS_EVENT*   EncryptReadySemEvt;
extern  OS_EVENT*   EncryptCpleSemEvt;


extern void Encrypt_DES_Encrypt(u8 *input, u8 *key, u8 *output, u32 length);
extern void Encrypt_DES_Decrypt(u8 *input, u8 *key, u8 *output, u32 length);
extern s32 encryptInit(void);
extern void encryptIntHandler(void);
extern int TripleDesCBCEncryptOnePacket(u8 *KEY, u8 *EnData, u32 len, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u8 Head, u8 Tail);
extern int TripleDesCBCEncrypt(u8 *KEY, u8 *EnData, u32 TotalLen, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u32 PacketLen);
extern int TripleDesCBCDecryptOnePacket(u8 *KEY, u8 *DeData, u32 len, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u8 Head, u8 Tail);
extern int TripleDesCBCDecrypt(u8 *KEY, u8 *DeData, u32 TotalLen, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u32 PacketLen);
extern int TripleAesCBCEncryptOnePacket(u8 *KEY, u8 *EnData, u32 len, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u8 Head, u8 Tail);
extern int TripleAesCBCEncrypt(u8 *KEY, u8 *EnData, u32 TotalLen, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u32 PacketLen);
extern int TripleAesCBCDecryptOnePacket(u8 *KEY, u8 *DeData, u32 len, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u8 Head, u8 Tail);
extern int TripleAesCBCDecrypt(u8 *KEY, u8 *DeData, u32 TotalLen, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u32 PacketLen);

#if AESDES_TEST
extern void TestEncrypt_2(void);
#endif
#if ICOMMWIFI_SUPPORT
typedef unsigned    int     u32_t;
#endif
#endif
#if LWIP2_SUPPORT
/* MD5_H */

#define MD5_H

//typedef unsigned    int     u32_t;
/* Data structure for MD5 (Message-Digest) computation */
typedef struct {
  u32 i[2];                   /* number of _bits_ handled mod 2^64 */
  u32 buf[4];                                    /* scratch buffer */
  unsigned char in[64];                              /* input buffer */
  unsigned char digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;

void MD5Init (MD5_CTX *mdContext);
void MD5Update (MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen);
void MD5Final (unsigned char hash[], MD5_CTX *mdContext);
#endif