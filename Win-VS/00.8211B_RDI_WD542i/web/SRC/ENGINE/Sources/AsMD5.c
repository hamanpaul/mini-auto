/*
 *	File:		AsMD5.c
 *
 *	Contains:	RomPager version of RSA Data Security, Inc.
 *				MD5 message-digest algorithm routines.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2003 by Allegro Software Development Corporation
 *  All rights reserved.
 *
 *  This module contains confidential, unpublished, proprietary 
 *  source code of Allegro Software Development Corporation.
 *
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 *
 *  License is granted for specific uses only under separate 
 *  written license by Allegro Software Development Corporation.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	MD5.C - RSA Data Security, Inc., MD5 message-digest algorithm
 *
 *	Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 *	rights reserved.
 *
 *	License to copy and use this software is granted provided that it
 *	is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 *	Algorithm" in all material mentioning or referencing this software
 *	or this function.
 *
 *	License is also granted to make and use derivative works provided
 *	that such works are identified as "derived from the RSA Data
 *	Security, Inc. MD5 Message-Digest Algorithm" in all material
 *	mentioning or referencing the derived work.
 *
 *	RSA Data Security, Inc. makes no representations concerning either
 *	the merchantability of this software or the suitability of this
 *	software for any particular purpose. It is provided "as is"
 *	without express or implied warranty of any kind.
 *
 *	These notices must be retained in any copies of any part of this
 *	documentation and/or software.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 * * * * Release 4.20  * * *
 *		12/18/02	amp		enable MD5 code for RomCliSecure
 *		10/04/02	amp		expose RpMD5Init, RpMD5Update and RpMD5Final
 * * * * Release 4.12  * * *
 * * * * Release 4.00  * * *
 *		07/05/00	rhb		enable RpMD5 for WcDigestAuthentication
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 *		01/22/99	pjr		RpMD5.c -> AsMD5.c
 * * * * Release 2.2 * * * *
 *		11/09/98	bva		use macro abstraction for stdlib calls
 * * * * Release 2.1 * * * *
 *		04/16/98	bva		use memset, memcpy
 *		02/03/98	rhb		enable for PrUseApop
 * * * * Release 2.0 * * * *
 *		09/03/97	pjr		add RpMD5.
 * * * * Release 1.6 * * * *
 *		04/18/97	pjr		initial version
 * * * * Release 1.5 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"

#if RomPagerSecurityDigest || PrUseApop || WcDigestAuthentication \
		|| RomPagerSecure || RomWebClientSecure || RomCliSecure

/*
	Constants for MD5Transform routine.
*/

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


static void MD5Transform(Unsigned32Ptr theState, Unsigned8Ptr theBlock);
static void Encode(Unsigned8Ptr theOutput, Unsigned32Ptr theInput,
					Unsigned32 theLength);
static void Decode(Unsigned32Ptr theOutput, Unsigned8Ptr theInput,
					Unsigned32 theLength);


static Unsigned8 PADDING[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/*
	F, G, H and I are basic MD5 functions.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/*
	ROTATE_LEFT rotates x left n bits.
*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/*
	FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
	Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
	(a) += F ((b), (c), (d)) + (x) + (Unsigned32)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
	(a) += G ((b), (c), (d)) + (x) + (Unsigned32)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
	(a) += H ((b), (c), (d)) + (x) + (Unsigned32)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
	(a) += I ((b), (c), (d)) + (x) + (Unsigned32)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}


/*
	Call the routines necessary to perform an MD5 calculation.
*/

void RpMD5(char * theStringPtr, Unsigned8Ptr theDigestResult) {
	rpMD5Context		theContext;

	RpMD5Init(&theContext);
	RpMD5Update(&theContext, (Unsigned8Ptr)theStringPtr, 
			RP_STRLEN(theStringPtr));
	RpMD5Final(theDigestResult, &theContext);

	return;
}


/*
	MD5 initialization. Begins an MD5 operation, writing a new context.
*/

void RpMD5Init(rpMD5ContextPtr theContext) {

	theContext->fCount[0] = theContext->fCount[1] = 0;

	/*
		Load magic initialization constants.
	*/
	theContext->fState[0] = 0x67452301;
	theContext->fState[1] = 0xefcdab89;
	theContext->fState[2] = 0x98badcfe;
	theContext->fState[3] = 0x10325476;

	return;
}


/*
	MD5 block update operation. Continues an MD5 message-digest
	operation, processing another message block, and updating the
	context.
 */

void RpMD5Update(rpMD5ContextPtr theContext, Unsigned8Ptr theInput,
				Unsigned32 theInputLength) {
	Unsigned32		i, theIndex, thePartLength;

	/* Compute number of bytes mod 64 */
	theIndex = (unsigned int)((theContext->fCount[0] >> 3) & 0x3F);

	/* Update number of bits */
	if ((theContext->fCount[0] += (theInputLength << 3)) <
		(theInputLength << 3)) {
			theContext->fCount[1]++;
	}

	theContext->fCount[1] += (theInputLength >> 29);

	thePartLength = (64 - theIndex);

	/*
		Transform as many times as possible.
	*/
	if (theInputLength >= thePartLength) {
		RP_MEMCPY(&theContext->fBuffer[theIndex], theInput, thePartLength);
		MD5Transform(theContext->fState, theContext->fBuffer);

		for (i = thePartLength; i + 63 < theInputLength; i += 64) {
			MD5Transform(theContext->fState, &theInput[i]);
		}

		theIndex = 0;
	}
	else {
		i = 0;
	}

	/*
		Buffer remaining input
	*/
	RP_MEMCPY(&theContext->fBuffer[theIndex], &theInput[i],
				(theInputLength - i));

	return;
}


/*
	MD5 finalization. Ends an MD5 message-digest operation, writing the
	the message digest and zeroizing the context.
*/

void RpMD5Final(Unsigned8Ptr theDigest, rpMD5Context * theContext) {
	Unsigned8		theBits[8];
	Unsigned32		theIndex, thePadLength;

	/*
		Save number of bits
	*/
	Encode(theBits, theContext->fCount, 8);

	/*
		Pad out to 56 mod 64.
	*/
	theIndex = (unsigned int)((theContext->fCount[0] >> 3) & 0x3f);
	thePadLength = (theIndex < 56) ? (56 - theIndex) : (120 - theIndex);
	RpMD5Update(theContext, PADDING, thePadLength);

	/*
		Append length (before padding)
	*/
	RpMD5Update(theContext, theBits, 8);

	/*
		Store state in digest
	*/
	Encode(theDigest, theContext->fState, 16);

	/*
		Zeroize sensitive information.
	*/
	RP_MEMSET((Unsigned8Ptr)theContext, 0, sizeof(*theContext));

	return;
}


/*
	MD5 basic transformation. Transforms state based on block.
*/

static void MD5Transform(Unsigned32Ptr theState, Unsigned8Ptr theBlock) {
	Unsigned32	a = theState[0];
	Unsigned32	b = theState[1];
	Unsigned32	c = theState[2];
	Unsigned32	d = theState[3];
	Unsigned32	x[16];

	Decode(x, theBlock, 64);

	/*
		Round 1
	*/
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478);	/* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756);	/* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db);	/* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee);	/* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf);	/* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a);	/* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613);	/* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501);	/* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8);	/* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af);	/* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1);	/* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be);	/* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122);	/* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193);	/* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e);	/* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821);	/* 16 */

	/*
		Round 2
	*/
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562);	/* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340);	/* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51);	/* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa);	/* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d);	/* 21 */
	GG (d, a, b, c, x[10], S22, 0x2441453);		/* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681);	/* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8);	/* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6);	/* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6);	/* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87);	/* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed);	/* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905);	/* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8);	/* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9);	/* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a);	/* 32 */

	/*
		Round 3
	*/
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942);	/* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681);	/* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122);	/* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c);	/* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44);	/* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9);	/* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60);	/* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70);	/* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6);	/* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa);	/* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085);	/* 43 */
	HH (b, c, d, a, x[ 6], S34, 0x4881d05);		/* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039);	/* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5);	/* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8);	/* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665);	/* 48 */

	/*
		Round 4
	*/
	II (a, b, c, d, x[ 0], S41, 0xf4292244);	/* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97);	/* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7);	/* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039);	/* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3);	/* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92);	/* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d);	/* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1);	/* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f);	/* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0);	/* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314);	/* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1);	/* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82);	/* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235);	/* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb);	/* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391);	/* 64 */

	theState[0] += a;
	theState[1] += b;
	theState[2] += c;
	theState[3] += d;

	/*
		Zeroize sensitive information.
	*/
	RP_MEMSET((Unsigned8Ptr)x, 0, sizeof(x));

	return;
}


/*
	Encodes theInput (Unsigned32) into theOutput (unsigned char).
	Assumes theLength is a multiple of 4.
 */

static void Encode(Unsigned8Ptr theOutput, Unsigned32Ptr theInput,
					Unsigned32 theLength) {
	Unsigned16	i, j;

	for (i = 0, j = 0; j < theLength; i++, j += 4) {
		theOutput[j] = (Unsigned8)(theInput[i] & 0xff);
		theOutput[j+1] = (Unsigned8)((theInput[i] >> 8) & 0xff);
		theOutput[j+2] = (Unsigned8)((theInput[i] >> 16) & 0xff);
		theOutput[j+3] = (Unsigned8)((theInput[i] >> 24) & 0xff);
	}

	return;
}


/*
	Decodes theInput (unsigned char) into theOutput (Unsigned32).
	Assumes theLength is a multiple of 4.
*/

static void Decode(Unsigned32Ptr theOutput, Unsigned8Ptr theInput,
					Unsigned32 theLength) {
	Unsigned16	i, j;

	for (i = 0, j = 0; j < theLength; i++, j += 4) {
		theOutput[i] = ((Unsigned32)theInput[j]) |
						(((Unsigned32)theInput[j+1]) << 8) |
						(((Unsigned32)theInput[j+2]) << 16) |
						(((Unsigned32)theInput[j+3]) << 24);
	}

	return;
}

#endif	/* RomPagerSecurityDigest || PrUseApop || WcDigestAuthentication \
		|| RomPagerSecure || RomWebClientSecure || RomCliSecure */
