/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    encrypt.c

Abstract:

    The routines of Encrypt/Decrypt function

Environment:

        ARM RealView Developer Suite

Revision History:

    2011/04/20  Elsa Lee Create

*/
#include "general.h"
#include "board.h"

#include <string.h>
#include "task.h"
#include "sysapi.h"
#include "siuapi.h"
#include "uiapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "siuapi.h"
#include "ipuapi.h"
#include "isuapi.h"
#include "iduapi.h"
#include "jpegapi.h"
#include "mp4api.h"
#include "asfapi.h"
#include "movapi.h"
#include "timerapi.h"
#include "usbapi.h"
#include "aviapi.h" /* Peter 0704 */
#include "mpeg4api.h"
#include "iisapi.h"
#include "ispapi.h" /*CY 1023*/
#include "gpioapi.h"
#include "adcapi.h"
#include "uartapi.h"
#include "ClockSwitchApi.h"
#include "awbapi.h"
#include "../ui/inc/ui.h"
#include "smcapi.h"
#include "spiapi.h"
#include "gpsapi.h"
#include "sdcapi.h"
#include "osapi.h"
#include "usbapi.h"
#include "uikey.h"
#include "logfileapi.h"
#include "i2capi.h"
#if (TUTK_SUPPORT)
#include "p2pserver_api.h"
#endif

#include "ciuapi.h"
#include "rfiuapi.h"

#include "des.h"
#include "encrptyapi.h"


#if AESDES_TEST

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */


 /*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
u8  DES_KEY[24]     = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
u8  DES_IVData[8]   = {24,25,26,27,28,29,30,31};
u8  AES_KEY[16]     = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
u8  AES_IVData[16]  = {24,25,26,27,28,29,30,31,16,17,18,19,20,21,22,23};
u8  K1[64]          = {0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
                       0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
                       0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
                       0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
                       0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
                       0x48,0x49,0x3a,0x4b,0x4c,0x4d,0x4e,0x4f,
                       0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
                       0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f};
u8  K2[64]          = {0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
                       0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
                       0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
                       0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
                       0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
                       0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
                       0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
                       0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f};
u8	PMAC_Header[13] = {0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0x17,0x03,0x01,0x00,0x00};

/*
 *********************************************************************************************************
 * Extern Variable
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Function Prototype
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Extern Function 
 *********************************************************************************************************
 */


/*
  *********************************************************************************************************
  * Function body
  *********************************************************************************************************
  */


/*

Routine Description:

    Test encrypt function.

Arguments:

    None.

Return Value:

    None.

*/
#if AESDES_TEST
void TestEncrypt(void)
{
    DCF_LIST_FILEENT*   dcfCurrFile;
    FS_FILE*            pFile;
    u32                 size, TotalLen, EnSize, DeSize;
    u8                  *EnData, *OData, *DeData, SHA[36];
    u8  tmp;

    dcfCurrFile     = dcfGetPlaybackFileListHead();
    EnData          = PKBuf;
    OData           = EnData + 1048576;
    DeData          = OData + 1048576;
    while(1)
    {
        DEBUG_ENCRYPT("\n");
        if ((pFile = dcfOpen((signed char*)dcfCurrFile->pDirEnt->d_name, "r")) == NULL)
        {
            DEBUG_ENCRYPT("Error:dcfOpen(%s) fail!!!\n", (signed char*)dcfCurrFile->pDirEnt->d_name);
            if(dcfCurrFile == dcfGetPlaybackFileListTail())
                break;
            else
                dcfCurrFile = dcfCurrFile->next;
            continue;
        }
        TotalLen    = pFile->size;
        if(TotalLen > (65536 - 28))
        {
            DEBUG_ENCRYPT("TotalLen(%d) too big, continue to next one....\n", TotalLen);
            if (dcfClose(pFile, &tmp) == NULL)
                return;
            if(dcfCurrFile == dcfGetPlaybackFileListTail())
                break;
            else
                dcfCurrFile = dcfCurrFile->next;
            continue;
        }
        if (dcfRead(pFile, EnData, TotalLen, &size) == NULL)
        {
            DEBUG_ENCRYPT("Error: dcfRead(0x%08x, 0x%08x, %d, %d) fail!!!\n", pFile, EnData, TotalLen, size);
            dcfClose(pFile, &tmp);
            if(dcfCurrFile == dcfGetPlaybackFileListTail())
                break;
            else
                dcfCurrFile = dcfCurrFile->next;
            continue;
        }
        dcfClose(pFile, &tmp);

        PMAC_Header[11] = (TotalLen & 0x0000ff00) >> 8;
        PMAC_Header[12] = TotalLen & 0x000000ff;

     #if(CHIP_OPTION == CHIP_A1018B)
        DEBUG_ENCRYPT("TripleDesCBCEncrypt(%d)\n", TotalLen);
        EnSize  = TripleDesCBCEncrypt(DES_KEY, EnData, TotalLen, DES_IVData, OData, K1, K2, PMAC_Header, 16384);

        memcpy((u8*)SHA, (u8*)&SHA_CS, 20);
        memset(&SHA[20], (u8)(EnSize - TotalLen - 21), EnSize - TotalLen - 20);

        DEBUG_ENCRYPT("TripleDesCBCDecrypt(%d)\n", EnSize);
        DeSize  = TripleDesCBCDecrypt(DES_KEY, OData, EnSize, DES_IVData, DeData, K1, K2, PMAC_Header, 16384);

        if(DeSize != TotalLen)
            DEBUG_ENCRYPT("Error: DeSize(%d) != TotalLen(%d)\n", DeSize, TotalLen);

        if(memcmp(EnData, DeData, TotalLen))
            DEBUG_ENCRYPT("Error: Value don't match between encrypt and decrypt data!!!!(0x%08x,0x%08x,0x%08x))\n", EnData, DeData, TotalLen);
        else
            DEBUG_ENCRYPT("%s DES encrypt and decrypt success\n", (signed char*)dcfCurrFile->pDirEnt->d_name);

        if(memcmp(DeData + TotalLen, (u8*)SHA, EnSize - TotalLen))
            DEBUG_ENCRYPT("Error: SHA Value don't match between encrypt and decrypt data!!!!)\n");
      #endif


        DEBUG_ENCRYPT("TripleAesCBCEncrypt(%d)\n", TotalLen);
        EnSize  = TripleAesCBCEncrypt(AES_KEY, EnData, TotalLen, AES_IVData, OData, K1, K2, PMAC_Header, 16384);

        memcpy((u8*)SHA, (u8*)&SHA_CS, 20);
        memset(&SHA[20], (u8)(EnSize - TotalLen - 21), EnSize - TotalLen - 20);

        DEBUG_ENCRYPT("TripleAesCBCDecrypt(%d)\n", EnSize);
        DeSize  = TripleAesCBCDecrypt(AES_KEY, OData, EnSize, AES_IVData, DeData, K1, K2, PMAC_Header, 16384);

        if(DeSize != TotalLen)
            DEBUG_ENCRYPT("Error: DeSize(%d) != TotalLen(%d)\n", DeSize, TotalLen);

        if(memcmp(EnData, DeData, TotalLen))
            DEBUG_ENCRYPT("Error: Value don't match between encrypt and decrypt data!!!!(0x%08x,0x%08x,0x%08x))\n", EnData, DeData, TotalLen);
        else
            DEBUG_ENCRYPT("%s AES encrypt and decrypt success\n", (signed char*)dcfCurrFile->pDirEnt->d_name);

        if(memcmp(DeData + TotalLen, (u8*)SHA, EnSize - TotalLen))
            DEBUG_ENCRYPT("Error: SHA Value don't match between encrypt and decrypt data!!!!)\n");



        if(dcfCurrFile == dcfGetPlaybackFileListTail())
            //DEBUG_ENCRYPT("dcfCurrFile == dcfListFileEntTail\n");
            break;
        else
            dcfCurrFile = dcfCurrFile->next;
    }
    DEBUG_ENCRYPT("TestEncrypt() finish!!!\n");
}


void TestEncrypt_2(void)
{
    u32  size, TotalLen, EnSize, DeSize;
    u8  *EnData, *OData, *DeData, SHA[36];
    int TestRun;
    u32 *pp;
    int i;
    //--------------------------//
    TestRun=0;
    TotalLen=20*1024;
    
    EnData          = PKBuf;
    OData           = EnData + 1048576;
    DeData          = OData + 1048576;

    pp=(u32 *)EnData;
    for(i=0;i<1024*64/4;i++)
    {
       *pp = i; 
       pp ++;
    }

    SYS_CTL0_EXT |= SYS_CTL0_EXT_DES_CKEN;    
    while(TotalLen<48*1024)
    {
        DEBUG_ENCRYPT("-----Test Encrypt Run-%d -----\n",TestRun);
        pp=(u32 *)EnData;
        for(i=0;i<TotalLen/4;i++)
        {
           *pp = *pp + 0x10101101; 
           pp ++;
        }
        //-------------------------//
        PMAC_Header[11] = (TotalLen & 0x0000ff00) >> 8;
        PMAC_Header[12] = TotalLen & 0x000000ff;
    #if(CHIP_OPTION == CHIP_A1018B)
        DEBUG_ENCRYPT("TripleDesCBCEncrypt(%d)\n", TotalLen);
        EnSize  = TripleDesCBCEncrypt(DES_KEY, EnData, TotalLen, DES_IVData, OData, K1, K2, PMAC_Header, 16384);

        memcpy((u8*)SHA, (u8*)&SHA_CS, 20);
        memset(&SHA[20], (u8)(EnSize - TotalLen - 21), EnSize - TotalLen - 20);

        DEBUG_ENCRYPT("TripleDesCBCDecrypt(%d)\n", EnSize);
        DeSize  = TripleDesCBCDecrypt(DES_KEY, OData, EnSize, DES_IVData, DeData, K1, K2, PMAC_Header, 16384);

        if(DeSize != TotalLen)
            DEBUG_ENCRYPT("Error: DeSize(%d) != TotalLen(%d)\n", DeSize, TotalLen);

        if(memcmp(EnData, DeData, TotalLen))
            DEBUG_ENCRYPT("Error: Value don't match between encrypt and decrypt data!!!!(0x%08x,0x%08x,0x%08x))\n", EnData, DeData, TotalLen);
        else
            DEBUG_ENCRYPT("DES encrypt and decrypt success\n");

        if(memcmp(DeData + TotalLen, (u8*)SHA, EnSize - TotalLen))
            DEBUG_ENCRYPT("Error: SHA Value don't match between encrypt and decrypt data!!!!)\n");
    #endif


        DEBUG_ENCRYPT("TripleAesCBCEncrypt(%d)\n", TotalLen);
        EnSize  = TripleAesCBCEncrypt(AES_KEY, EnData, TotalLen, AES_IVData, OData, K1, K2, PMAC_Header, 16384);

        memcpy((u8*)SHA, (u8*)&SHA_CS, 20);
        memset(&SHA[20], (u8)(EnSize - TotalLen - 21), EnSize - TotalLen - 20);

        DEBUG_ENCRYPT("TripleAesCBCDecrypt(%d)\n", EnSize);
        DeSize  = TripleAesCBCDecrypt(AES_KEY, OData, EnSize, AES_IVData, DeData, K1, K2, PMAC_Header, 16384);

        if(DeSize != TotalLen)
            DEBUG_ENCRYPT("Error: DeSize(%d) != TotalLen(%d)\n", DeSize, TotalLen);

        if(memcmp(EnData, DeData, TotalLen))
            DEBUG_ENCRYPT("Error: Value don't match between encrypt and decrypt data!!!!(0x%08x,0x%08x,0x%08x))\n", EnData, DeData, TotalLen);
        else
            DEBUG_ENCRYPT("AES encrypt and decrypt success\n");

        if(memcmp(DeData + TotalLen, (u8*)SHA, EnSize - TotalLen))
            DEBUG_ENCRYPT("Error: SHA Value don't match between encrypt and decrypt data!!!!)\n");

        TotalLen +=4;
        TestRun ++;
    }

    SYS_CTL0_EXT &= (~SYS_CTL0_EXT_DES_CKEN);

    DEBUG_ENCRYPT("TestEncrypt() finish!!!\n");
}
#endif
#endif

#if LWIP2_SUPPORT
/*
 ***********************************************************************
 **  Message-digest routines:                                         **
 **  To form the message digest for a message M                       **
 **    (1) Initialize a context buffer mdContext using MD5Init        **
 **    (2) Call MD5Update on mdContext and M                          **
 **    (3) Call MD5Final on mdContext                                 **
 **  The message digest is now in mdContext->digest[0...15]           **
 ***********************************************************************
 */

/* forward declaration */
static void Transform (u32 *buf, u32 *in);

static unsigned char PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G, H and I are basic MD5 functions */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (u32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (u32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (u32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (u32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

#ifdef __STDC__
#define UL(x)	x##UL
#else
#ifdef WIN32
#define UL(x)	x##UL
#else
#define UL(x)	x
#endif
#endif

/* The routine MD5Init initializes the message-digest context
   mdContext. All fields are set to zero.
 */
void MD5Init (MD5_CTX *mdContext)
{
  mdContext->i[0] = mdContext->i[1] = (u32)0;

  /* Load magic initialization constants.
   */
  mdContext->buf[0] = (u32)0x67452301UL;
  mdContext->buf[1] = (u32)0xefcdab89UL;
  mdContext->buf[2] = (u32)0x98badcfeUL;
  mdContext->buf[3] = (u32)0x10325476UL;
}

/* The routine MD5Update updates the message-digest context to
   account for the presence of each of the characters inBuf[0..inLen-1]
   in the message whose digest is being computed.
 */
void MD5Update(MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen)
{
  u32 in[16];
  int mdi;
  unsigned int i, ii;

#if 0
  ppp_trace(LOG_INFO, "MD5Update: %u:%.*H\n", inLen, MIN(inLen, 20) * 2, inBuf);
  ppp_trace(LOG_INFO, "MD5Update: %u:%s\n", inLen, inBuf);
#endif
  
  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContext->i[0] + ((u32)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((u32)inLen << 3);
  mdContext->i[1] += ((u32)inLen >> 29);

  while (inLen--) {
    /* add new character to buffer, increment mdi */
    mdContext->in[mdi++] = *inBuf++;

    /* transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((u32)mdContext->in[ii+3]) << 24) |
                (((u32)mdContext->in[ii+2]) << 16) |
				(((u32)mdContext->in[ii+1]) << 8) |
                ((u32)mdContext->in[ii]);
      Transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}

/* The routine MD5Final terminates the message-digest computation and
   ends with the desired message digest in mdContext->digest[0...15].
 */
void MD5Final (unsigned char hash[], MD5_CTX *mdContext)
{
  u32 in[16];
  int mdi;
  unsigned int i, ii;
  unsigned int padLen;

  /* save number of bits */
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* pad out to 56 mod 64 */
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  MD5Update (mdContext, PADDING, padLen);

  /* append length in bits and transform */
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((u32)mdContext->in[ii+3]) << 24) |
            (((u32)mdContext->in[ii+2]) << 16) |
            (((u32)mdContext->in[ii+1]) << 8) |
            ((u32)mdContext->in[ii]);
  Transform (mdContext->buf, in);

  /* store buffer in digest */
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
    mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
	mdContext->digest[ii+1] =
      (unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
    mdContext->digest[ii+2] =
      (unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
    mdContext->digest[ii+3] =
      (unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
  }
  memcpy(hash, mdContext->digest, 16);
}

/* Basic MD5 step. Transforms buf based on in.
 */
static void Transform (u32 *buf, u32 *in)
{
  u32 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, in[ 0], S11, UL(3614090360)); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, UL(3905402710)); /* 2 */
  FF ( c, d, a, b, in[ 2], S13, UL( 606105819)); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, UL(3250441966)); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, UL(4118548399)); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, UL(1200080426)); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, UL(2821735955)); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, UL(4249261313)); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, UL(1770035416)); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, UL(2336552879)); /* 10 */
  FF ( c, d, a, b, in[10], S13, UL(4294925233)); /* 11 */
  FF ( b, c, d, a, in[11], S14, UL(2304563134)); /* 12 */
  FF ( a, b, c, d, in[12], S11, UL(1804603682)); /* 13 */
  FF ( d, a, b, c, in[13], S12, UL(4254626195)); /* 14 */
  FF ( c, d, a, b, in[14], S13, UL(2792965006)); /* 15 */
  FF ( b, c, d, a, in[15], S14, UL(1236535329)); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, UL(4129170786)); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, UL(3225465664)); /* 18 */
  GG ( c, d, a, b, in[11], S23, UL( 643717713)); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, UL(3921069994)); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, UL(3593408605)); /* 21 */
  GG ( d, a, b, c, in[10], S22, UL(  38016083)); /* 22 */
  GG ( c, d, a, b, in[15], S23, UL(3634488961)); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, UL(3889429448)); /* 24 */
  GG ( a, b, c, d, in[ 9], S21, UL( 568446438)); /* 25 */
  GG ( d, a, b, c, in[14], S22, UL(3275163606)); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, UL(4107603335)); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, UL(1163531501)); /* 28 */
  GG ( a, b, c, d, in[13], S21, UL(2850285829)); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, UL(4243563512)); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, UL(1735328473)); /* 31 */
  GG ( b, c, d, a, in[12], S24, UL(2368359562)); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, UL(4294588738)); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, UL(2272392833)); /* 34 */
  HH ( c, d, a, b, in[11], S33, UL(1839030562)); /* 35 */
  HH ( b, c, d, a, in[14], S34, UL(4259657740)); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, UL(2763975236)); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, UL(1272893353)); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, UL(4139469664)); /* 39 */
  HH ( b, c, d, a, in[10], S34, UL(3200236656)); /* 40 */
  HH ( a, b, c, d, in[13], S31, UL( 681279174)); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, UL(3936430074)); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, UL(3572445317)); /* 43 */
  HH ( b, c, d, a, in[ 6], S34, UL(  76029189)); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, UL(3654602809)); /* 45 */
  HH ( d, a, b, c, in[12], S32, UL(3873151461)); /* 46 */
  HH ( c, d, a, b, in[15], S33, UL( 530742520)); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, UL(3299628645)); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, UL(4096336452)); /* 49 */
  II ( d, a, b, c, in[ 7], S42, UL(1126891415)); /* 50 */
  II ( c, d, a, b, in[14], S43, UL(2878612391)); /* 51 */
  II ( b, c, d, a, in[ 5], S44, UL(4237533241)); /* 52 */
  II ( a, b, c, d, in[12], S41, UL(1700485571)); /* 53 */
  II ( d, a, b, c, in[ 3], S42, UL(2399980690)); /* 54 */
  II ( c, d, a, b, in[10], S43, UL(4293915773)); /* 55 */
  II ( b, c, d, a, in[ 1], S44, UL(2240044497)); /* 56 */
  II ( a, b, c, d, in[ 8], S41, UL(1873313359)); /* 57 */
  II ( d, a, b, c, in[15], S42, UL(4264355552)); /* 58 */
  II ( c, d, a, b, in[ 6], S43, UL(2734768916)); /* 59 */
  II ( b, c, d, a, in[13], S44, UL(1309151649)); /* 60 */
  II ( a, b, c, d, in[ 4], S41, UL(4149444226)); /* 61 */
  II ( d, a, b, c, in[11], S42, UL(3174756917)); /* 62 */
  II ( c, d, a, b, in[ 2], S43, UL( 718787259)); /* 63 */
  II ( b, c, d, a, in[ 9], S44, UL(3951481745)); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}
#endif

