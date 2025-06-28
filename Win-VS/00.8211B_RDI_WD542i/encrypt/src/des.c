/*--------------------------------------------------------------------------------
DES.C: DES Algorithm Program from the Book Appliced Cryptography, Bruce Schneier
--------------------------------------------------------------------------------*/

#include "general.h"
#include "board.h"
#include "des.h"
#include "encrptyapi.h"

#if 0

#define EN0   0      /* MODE == encrypt */
#define DE1   1      /* MODE == decrypt */

extern void deskey(unsigned char *, short);
/*                  hexkey[8]     MODE
 * Sets the internal key register according to the hexadecimal
 * key contained in the 8 bytes of hexkey, according to the DES,
 * for encryption or decryption according to MODE.
 */

extern void usekey(unsigned long *);
/*                cookedkey[32]
 * Loads the internal key register with the data in cookedkey.
 */

extern void cpkey(unsigned long *);
/*               cookedkey[32]
 * Copies the contents of the internal key register into the storage
 * located at &cookedkey[0].
 */

extern void des(unsigned char *, unsigned char *);
/*                from[8]         to[8]
 * Encrypts/Decrypts (according to the key currently loaded in the
 * internal key register) one block of eight bytes at address 'from'
 * into the block at address 'to'.  They can be the same.
 */

static void scrunch(unsigned char *, unsigned long *);
static void unscrun(unsigned long *, unsigned char *);
static void desfunc(unsigned long *, unsigned long *);
static void cookey(unsigned long *);

static unsigned long KnL[32] = { 0L };
static unsigned long KnR[32] = { 0L };
static unsigned long Kn3[32] = { 0L };
static unsigned char Df_Key[24] = 
{
    0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
    0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10,
    0x89,0xab,0xcd,0xef,0x01,0x23,0x45,0x67 
};

static unsigned short bytebit[8]    = 
{
    0200, 0100, 040, 020, 010, 04, 02, 01 
};

static unsigned long bigbyte[24] = 
{
    0x800000L,    0x400000L,     0x200000L,    0x100000L,
    0x80000L,     0x40000L,      0x20000L,     0x10000L,
    0x8000L,      0x4000L,       0x2000L,      0x1000L,
    0x800L,              0x400L,               0x200L,              0x100L,
    0x80L,               0x40L,                0x20L,               0x10L,
    0x8L,         0x4L,          0x2L,         0x1L
};

/* Use the key schedule specified in the Standard (ANSI X3.92-1981). */

static unsigned char pc1[56] = 
{
    56, 48, 40, 32, 24, 16,  8,   0, 57, 49, 41, 33, 25, 17,
    9,  1, 58, 50, 42, 34, 26,  18, 10,  2, 59, 51, 43, 35,
    62, 54, 46, 38, 30, 22, 14,   6, 61, 53, 45, 37, 29, 21,
    13,  5, 60, 52, 44, 36, 28,  20, 12,  4, 27, 19, 11,  3
};

static unsigned char totrot[16] = 
{
    1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28 
};

static unsigned char pc2[48] = 
{
    13, 16, 10, 23,  0,  4,       2, 27, 14,  5, 20,  9,
    22, 18, 11,  3, 25,  7,      15,  6, 26, 19, 12,  1,
    40, 51, 30, 36, 46, 54,      29, 39, 50, 44, 32, 47,
    43, 48, 38, 55, 33, 52,      45, 41, 49, 35, 28, 31 
};

void deskey(unsigned char *key, short edf)  
{
    /* Thanks to James Gillogly & Phil Karn! */
    register int i, j, l, m, n;
    unsigned char pc1m[56], pcr[56];
    unsigned long kn[32];

    for ( j = 0; j < 56; j++ ) 
    {
        l = pc1[j];
        m = l & 07;
        pc1m[j] = (key[l >> 3] & bytebit[m]) ? 1 : 0;
    }
    for( i = 0; i < 16; i++ ) 
    {
        if( edf == DE1 ) 
            m = (15 - i) << 1;
        else             
            m = i << 1;
        n = m + 1;
        kn[m] = kn[n] = 0L;
        for( j = 0; j < 28; j++ ) 
        {
            l = j + totrot[i];
            if( l < 28 ) 
                pcr[j] = pc1m[l];
            else 
                pcr[j] = pc1m[l - 28];
        }
        for( j = 28; j < 56; j++ ) 
        {
            l = j + totrot[i];
            if( l < 56 ) 
                pcr[j] = pc1m[l];
            else         
                pcr[j] = pc1m[l - 28];
        }
        for( j = 0; j < 24; j++ ) 
        {
            if( pcr[pc2[j]] )    
                kn[m] |= bigbyte[j];
            if( pcr[pc2[j+24]] ) 
                kn[n] |= bigbyte[j];
        }
    }
    cookey(kn);
}

static void cookey(unsigned long *raw1)  
{
    register unsigned long *cook, *raw0;
    unsigned long dough[32];
    register int i;

    cook = dough;
    for( i = 0; i < 16; i++, raw1++ ) 
    {
        raw0 = raw1++;
        *cook   = (*raw0 & 0x00fc0000L) << 6;
        *cook  |= (*raw0 & 0x00000fc0L) << 10;
        *cook  |= (*raw1 & 0x00fc0000L) >> 10;
        *cook++|= (*raw1 & 0x00000fc0L) >> 6;
        *cook   = (*raw0 & 0x0003f000L) << 12;
        *cook  |= (*raw0 & 0x0000003fL) << 16;
        *cook  |= (*raw1 & 0x0003f000L) >> 4;
        *cook++       |= (*raw1 & 0x0000003fL);
    }
    usekey(dough);
}

void cpkey(unsigned long *into)  
{
    register unsigned long *from, *endp;

    from = KnL, endp = &KnL[32];
    while( from < endp ) 
        *into++ = *from++;
}

void usekey(unsigned long *from)  
{
    register unsigned long *to, *endp;

    to = KnL, endp = &KnL[32];
    while( to < endp ) 
        *to++ = *from++;
}

#if 0
void des(unsigned char *inblock, unsigned char *outblock)  
{
    unsigned long work[2];

    scrunch(inblock, work);
    desfunc(work, KnL);
    unscrun(work, outblock);
}
#endif

static void scrunch(unsigned char *outof, unsigned long *into)  
{
    *into   = (*outof++ & 0xffL) << 24;
    *into  |= (*outof++ & 0xffL) << 16;
    *into  |= (*outof++ & 0xffL) << 8;
    *into++ |= (*outof++ & 0xffL);
    *into   = (*outof++ & 0xffL) << 24;
    *into  |= (*outof++ & 0xffL) << 16;
    *into  |= (*outof++ & 0xffL) << 8;
    *into  |= (*outof   & 0xffL);
}

static void unscrun(unsigned long *outof, unsigned char *into)  
{
    *into++ = (*outof >> 24) & 0xffL;
    *into++ = (*outof >> 16) & 0xffL;
    *into++ = (*outof >>  8) & 0xffL;
    *into++ =  *outof++      & 0xffL;
    *into++ = (*outof >> 24) & 0xffL;
    *into++ = (*outof >> 16) & 0xffL;
    *into++ = (*outof >>  8) & 0xffL;
    *into   =  *outof     & 0xffL;
}

static unsigned long SP1[64] = 
{
    0x01010400L, 0x00000000L, 0x00010000L, 0x01010404L,
    0x01010004L, 0x00010404L, 0x00000004L, 0x00010000L,
    0x00000400L, 0x01010400L, 0x01010404L, 0x00000400L,
    0x01000404L, 0x01010004L, 0x01000000L, 0x00000004L,
    0x00000404L, 0x01000400L, 0x01000400L, 0x00010400L,
    0x00010400L, 0x01010000L, 0x01010000L, 0x01000404L,
    0x00010004L, 0x01000004L, 0x01000004L, 0x00010004L,
    0x00000000L, 0x00000404L, 0x00010404L, 0x01000000L,
    0x00010000L, 0x01010404L, 0x00000004L, 0x01010000L,
    0x01010400L, 0x01000000L, 0x01000000L, 0x00000400L,
    0x01010004L, 0x00010000L, 0x00010400L, 0x01000004L,
    0x00000400L, 0x00000004L, 0x01000404L, 0x00010404L,
    0x01010404L, 0x00010004L, 0x01010000L, 0x01000404L,
    0x01000004L, 0x00000404L, 0x00010404L, 0x01010400L,
    0x00000404L, 0x01000400L, 0x01000400L, 0x00000000L,
    0x00010004L, 0x00010400L, 0x00000000L, 0x01010004L 
};

static unsigned long SP2[64] = 
{
    0x80108020L, 0x80008000L, 0x00008000L, 0x00108020L,
    0x00100000L, 0x00000020L, 0x80100020L, 0x80008020L,
    0x80000020L, 0x80108020L, 0x80108000L, 0x80000000L,
    0x80008000L, 0x00100000L, 0x00000020L, 0x80100020L,
    0x00108000L, 0x00100020L, 0x80008020L, 0x00000000L,
    0x80000000L, 0x00008000L, 0x00108020L, 0x80100000L,
    0x00100020L, 0x80000020L, 0x00000000L, 0x00108000L,
    0x00008020L, 0x80108000L, 0x80100000L, 0x00008020L,
    0x00000000L, 0x00108020L, 0x80100020L, 0x00100000L,
    0x80008020L, 0x80100000L, 0x80108000L, 0x00008000L,
    0x80100000L, 0x80008000L, 0x00000020L, 0x80108020L,
    0x00108020L, 0x00000020L, 0x00008000L, 0x80000000L,
    0x00008020L, 0x80108000L, 0x00100000L, 0x80000020L,
    0x00100020L, 0x80008020L, 0x80000020L, 0x00100020L,
    0x00108000L, 0x00000000L, 0x80008000L, 0x00008020L,
    0x80000000L, 0x80100020L, 0x80108020L, 0x00108000L 
};

static unsigned long SP3[64] = 
{
    0x00000208L, 0x08020200L, 0x00000000L, 0x08020008L,
    0x08000200L, 0x00000000L, 0x00020208L, 0x08000200L,
    0x00020008L, 0x08000008L, 0x08000008L, 0x00020000L,
    0x08020208L, 0x00020008L, 0x08020000L, 0x00000208L,
    0x08000000L, 0x00000008L, 0x08020200L, 0x00000200L,
    0x00020200L, 0x08020000L, 0x08020008L, 0x00020208L,
    0x08000208L, 0x00020200L, 0x00020000L, 0x08000208L,
    0x00000008L, 0x08020208L, 0x00000200L, 0x08000000L,
    0x08020200L, 0x08000000L, 0x00020008L, 0x00000208L,
    0x00020000L, 0x08020200L, 0x08000200L, 0x00000000L,
    0x00000200L, 0x00020008L, 0x08020208L, 0x08000200L,
    0x08000008L, 0x00000200L, 0x00000000L, 0x08020008L,
    0x08000208L, 0x00020000L, 0x08000000L, 0x08020208L,
    0x00000008L, 0x00020208L, 0x00020200L, 0x08000008L,
    0x08020000L, 0x08000208L, 0x00000208L, 0x08020000L,
    0x00020208L, 0x00000008L, 0x08020008L, 0x00020200L 
};

static unsigned long SP4[64] = 
{
    0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
    0x00802080L, 0x00800081L, 0x00800001L, 0x00002001L,
    0x00000000L, 0x00802000L, 0x00802000L, 0x00802081L,
    0x00000081L, 0x00000000L, 0x00800080L, 0x00800001L,
    0x00000001L, 0x00002000L, 0x00800000L, 0x00802001L,
    0x00000080L, 0x00800000L, 0x00002001L, 0x00002080L,
    0x00800081L, 0x00000001L, 0x00002080L, 0x00800080L,
    0x00002000L, 0x00802080L, 0x00802081L, 0x00000081L,
    0x00800080L, 0x00800001L, 0x00802000L, 0x00802081L,
    0x00000081L, 0x00000000L, 0x00000000L, 0x00802000L,
    0x00002080L, 0x00800080L, 0x00800081L, 0x00000001L,
    0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
    0x00802081L, 0x00000081L, 0x00000001L, 0x00002000L,
    0x00800001L, 0x00002001L, 0x00802080L, 0x00800081L,
    0x00002001L, 0x00002080L, 0x00800000L, 0x00802001L,
    0x00000080L, 0x00800000L, 0x00002000L, 0x00802080L 
};

static unsigned long SP5[64] = 
{
    0x00000100L, 0x02080100L, 0x02080000L, 0x42000100L,
    0x00080000L, 0x00000100L, 0x40000000L, 0x02080000L,
    0x40080100L, 0x00080000L, 0x02000100L, 0x40080100L,
    0x42000100L, 0x42080000L, 0x00080100L, 0x40000000L,
    0x02000000L, 0x40080000L, 0x40080000L, 0x00000000L,
    0x40000100L, 0x42080100L, 0x42080100L, 0x02000100L,
    0x42080000L, 0x40000100L, 0x00000000L, 0x42000000L,
    0x02080100L, 0x02000000L, 0x42000000L, 0x00080100L,
    0x00080000L, 0x42000100L, 0x00000100L, 0x02000000L,
    0x40000000L, 0x02080000L, 0x42000100L, 0x40080100L,
    0x02000100L, 0x40000000L, 0x42080000L, 0x02080100L,
    0x40080100L, 0x00000100L, 0x02000000L, 0x42080000L,
    0x42080100L, 0x00080100L, 0x42000000L, 0x42080100L,
    0x02080000L, 0x00000000L, 0x40080000L, 0x42000000L,
    0x00080100L, 0x02000100L, 0x40000100L, 0x00080000L,
    0x00000000L, 0x40080000L, 0x02080100L, 0x40000100L 
};

static unsigned long SP6[64] = 
{
    0x20000010L, 0x20400000L, 0x00004000L, 0x20404010L,
    0x20400000L, 0x00000010L, 0x20404010L, 0x00400000L,
    0x20004000L, 0x00404010L, 0x00400000L, 0x20000010L,
    0x00400010L, 0x20004000L, 0x20000000L, 0x00004010L,
    0x00000000L, 0x00400010L, 0x20004010L, 0x00004000L,
    0x00404000L, 0x20004010L, 0x00000010L, 0x20400010L,
    0x20400010L, 0x00000000L, 0x00404010L, 0x20404000L,
    0x00004010L, 0x00404000L, 0x20404000L, 0x20000000L,
    0x20004000L, 0x00000010L, 0x20400010L, 0x00404000L,
    0x20404010L, 0x00400000L, 0x00004010L, 0x20000010L,
    0x00400000L, 0x20004000L, 0x20000000L, 0x00004010L,
    0x20000010L, 0x20404010L, 0x00404000L, 0x20400000L,
    0x00404010L, 0x20404000L, 0x00000000L, 0x20400010L,
    0x00000010L, 0x00004000L, 0x20400000L, 0x00404010L,
    0x00004000L, 0x00400010L, 0x20004010L, 0x00000000L,
    0x20404000L, 0x20000000L, 0x00400010L, 0x20004010L 
};

static unsigned long SP7[64] = 
{
    0x00200000L, 0x04200002L, 0x04000802L, 0x00000000L,
    0x00000800L, 0x04000802L, 0x00200802L, 0x04200800L,
    0x04200802L, 0x00200000L, 0x00000000L, 0x04000002L,
    0x00000002L, 0x04000000L, 0x04200002L, 0x00000802L,
    0x04000800L, 0x00200802L, 0x00200002L, 0x04000800L,
    0x04000002L, 0x04200000L, 0x04200800L, 0x00200002L,
    0x04200000L, 0x00000800L, 0x00000802L, 0x04200802L,
    0x00200800L, 0x00000002L, 0x04000000L, 0x00200800L,
    0x04000000L, 0x00200800L, 0x00200000L, 0x04000802L,
    0x04000802L, 0x04200002L, 0x04200002L, 0x00000002L,
    0x00200002L, 0x04000000L, 0x04000800L, 0x00200000L,
    0x04200800L, 0x00000802L, 0x00200802L, 0x04200800L,
    0x00000802L, 0x04000002L, 0x04200802L, 0x04200000L,
    0x00200800L, 0x00000000L, 0x00000002L, 0x04200802L,
    0x00000000L, 0x00200802L, 0x04200000L, 0x00000800L,
    0x04000002L, 0x04000800L, 0x00000800L, 0x00200002L 
};

static unsigned long SP8[64] = 
{
    0x10001040L, 0x00001000L, 0x00040000L, 0x10041040L,
    0x10000000L, 0x10001040L, 0x00000040L, 0x10000000L,
    0x00040040L, 0x10040000L, 0x10041040L, 0x00041000L,
    0x10041000L, 0x00041040L, 0x00001000L, 0x00000040L,
    0x10040000L, 0x10000040L, 0x10001000L, 0x00001040L,
    0x00041000L, 0x00040040L, 0x10040040L, 0x10041000L,
    0x00001040L, 0x00000000L, 0x00000000L, 0x10040040L,
    0x10000040L, 0x10001000L, 0x00041040L, 0x00040000L,
    0x00041040L, 0x00040000L, 0x10041000L, 0x00001000L,
    0x00000040L, 0x10040040L, 0x00001000L, 0x00041040L,
    0x10001000L, 0x00000040L, 0x10000040L, 0x10040000L,
    0x10040040L, 0x10000000L, 0x00040000L, 0x10001040L,
    0x00000000L, 0x10041040L, 0x00040040L, 0x10000040L,
    0x10040000L, 0x10001000L, 0x10001040L, 0x00000000L,
    0x10041040L, 0x00041000L, 0x00041000L, 0x00001040L,
    0x00001040L, 0x00040040L, 0x10000000L, 0x10041000L 
};

static void desfunc(unsigned long *block, unsigned long *keys)  
{
    register unsigned long fval, work, right, leftt;
    register int round;

    leftt = block[0];
    right = block[1];
    work = ((leftt >> 4) ^ right) & 0x0f0f0f0fL;
    right ^= work;
    leftt ^= (work << 4);
    work = ((leftt >> 16) ^ right) & 0x0000ffffL;
    right ^= work;
    leftt ^= (work << 16);
    work = ((right >> 2) ^ leftt) & 0x33333333L;
    leftt ^= work;
    right ^= (work << 2);
    work = ((right >> 8) ^ leftt) & 0x00ff00ffL;
    leftt ^= work;
    right ^= (work << 8);
    right = ((right << 1) | ((right >> 31) & 1L)) & 0xffffffffL;
    work = (leftt ^ right) & 0xaaaaaaaaL;
    leftt ^= work;
    right ^= work;
    leftt = ((leftt << 1) | ((leftt >> 31) & 1L)) & 0xffffffffL;

    for( round = 0; round < 8; round++ ) 
    {
        work  = (right << 28) | (right >> 4);
        work ^= *keys++;
        fval  = SP7[ work             & 0x3fL];
        fval |= SP5[(work >>  8) & 0x3fL];
        fval |= SP3[(work >> 16) & 0x3fL];
        fval |= SP1[(work >> 24) & 0x3fL];
        work  = right ^ *keys++;
        fval |= SP8[ work             & 0x3fL];
        fval |= SP6[(work >>  8) & 0x3fL];
        fval |= SP4[(work >> 16) & 0x3fL];
        fval |= SP2[(work >> 24) & 0x3fL];
        leftt ^= fval;
        work  = (leftt << 28) | (leftt >> 4);
        work ^= *keys++;
        fval  = SP7[ work             & 0x3fL];
        fval |= SP5[(work >>  8) & 0x3fL];
        fval |= SP3[(work >> 16) & 0x3fL];
        fval |= SP1[(work >> 24) & 0x3fL];
        work  = leftt ^ *keys++;
        fval |= SP8[ work             & 0x3fL];
        fval |= SP6[(work >>  8) & 0x3fL];
        fval |= SP4[(work >> 16) & 0x3fL];
        fval |= SP2[(work >> 24) & 0x3fL];
        right ^= fval;
    }

    right = (right << 31) | (right >> 1);
    work = (leftt ^ right) & 0xaaaaaaaaL;
    leftt ^= work;
    right ^= work;
    leftt = (leftt << 31) | (leftt >> 1);
    work = ((leftt >> 8) ^ right) & 0x00ff00ffL;
    right ^= work;
    leftt ^= (work << 8);
    work = ((leftt >> 2) ^ right) & 0x33333333L;
    right ^= work;
    leftt ^= (work << 2);
    work = ((right >> 16) ^ leftt) & 0x0000ffffL;
    leftt ^= work;
    right ^= (work << 16);
    work = ((right >> 4) ^ leftt) & 0x0f0f0f0fL;
    leftt ^= work;
    right ^= (work << 4);
    *block++ = right;
    *block = leftt;
}

/* Validation sets:
 *
 * Single-length key, single-length plaintext -
 * Key    : 0123 4567 89ab cdef
 * Plain  : 0123 4567 89ab cde7
 * Cipher : c957 4425 6a5e d31d
 *
 **********************************************************************/

void des_key(des_ctx *dc, unsigned char *key)
{
    deskey(key,EN0);
    cpkey(dc->ek);
    deskey(key,DE1);
    cpkey(dc->dk);
}

/* Encrypt several blocks in ECB mode.  Caller is responsible for
   short blocks. */
void des_enc(des_ctx *dc, unsigned char *Data, int blocks)
{
    unsigned long work[2];
    int i;
    unsigned char *cp;

    cp = Data;
    for(i=0;i<blocks;i++)
    {
        scrunch(cp,work);
        desfunc(work,dc->ek);
        unscrun(work,cp);
        cp+=8;
    }
}

void des_dec(des_ctx *dc, unsigned char *Data, int blocks)
{
    unsigned long work[2];
    int i;
    unsigned char *cp;

    cp = Data;
    for(i=0;i<blocks;i++)
    {
        scrunch(cp,work);
        desfunc(work,dc->dk);
        unscrun(work,cp);
        cp+=8;
    }
}
/*
void main(void){
  des_ctx dc;
  int i;
  unsigned long Data[10];
  char *cp,key[8] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};
  char x[8] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xe7};



  cp = x;

  des_key(&dc,key);
  des_enc(&dc,cp,1);
  printf("Enc(0..7,0..7) = ");
  for(i=0;i<8;i++) printf("%02x ", ((unsigned int) cp[i])&0x00ff);
  printf("\n");

  des_dec(&dc,cp,1);

  printf("Dec(above,0..7) = ");
  for(i=0;i<8;i++) printf("%02x ",((unsigned int)cp[i])&0x00ff);
  printf("\n");

  cp = (char *) Data;
  for(i=0;i<10;i++) Data[i]=i;

  des_enc(&dc,cp,5); 
  for(i=0;i<10;i+=2) printf("Block %01d = %08lx %08lx.\n",
                             i/2,Data[i],Data[i+1]);

  des_dec(&dc,cp,1);
  des_dec(&dc,cp+8,4);
  for(i=0;i<10;i+=2) printf("Block %01d = %08lx %08lx.\n",
                             i/2,Data[i],Data[i+1]);
  while (1);

}
*/
#endif

OS_EVENT*   EncryptReadySemEvt;
OS_EVENT*   EncryptCpleSemEvt;


/*

Routine Description:

    Initialize encrypt/decrypt.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 encryptInit(void)
{
    int i;  

    /* Create the semaphore */
    EncryptReadySemEvt  = OSSemCreate(1);
    EncryptCpleSemEvt   = OSSemCreate(0);

    return 1;   
}

/*

Routine Description:

    The FIQ handler of encrypt/decrypt.

Arguments:

    None.

Return Value:

    None.

*/
void encryptIntHandler(void)
{
    u32 intStat     = DES_CTRL;
    u32 temp;

    if(intStat & DES_FINISH)    // encode/decode is finished
    {
        OSSemPost(EncryptCpleSemEvt);
    } 
    if(intStat & SHA_READY)     // SHA checksum is ready
    {
    }
    if(intStat & AUTH1_READY)   // 1'st Auth checksum is ready
    {
    }

    DES_CTRL    = intStat;      // clear interrupt status bits
}

/*

Routine Description:

    Encrypting a data packet using the Triple DES algorithm in CBC mode.

Arguments:

    KEY:        The key to be used for encryption.
    EnData:     The input data to be encrypted.
    len:        The input data length.
    IVData:     The initialization vector to be used, will be updated after this function call.
    OData:      The buffer which will be filled by the encrypted data.
    Sha_K1:     SHA extended key K1 for TLSV1 format (big-endian order).
    Sha_K2:     SHA extended key K2 for TLSV1 format (big-endian order).
    Sha_PMAC:   SHA PMAC data header for TLSV1 format (big-endian order).
    Head:       1: Indicate the triggered data is the end of transfer data, 0: otherwise.
    Tail:       1: Indicate the triggered data is the beginning of the transfer data, 0: otherwise.

Return Value:

    0 - Failure.
    1 - Success.

*/
int TripleDesCBCEncryptOnePacket(u8 *KEY, u8 *EnData, u32 len, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u8 Head, u8 Tail)
{
    u8  err;
    int i;

    if(len == 0)
    {
        DEBUG_ENCRYPT("TripleDesCBCEncryptOnePacket(0x%08x, 0x%08x, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, %d, %d)\n", KEY, EnData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail);
        DEBUG_ENCRYPT("Error: The source data size(%d) can't equal zero!!!\n", len);
        return 0;
    }
    if(!Tail)
    {
        if(len & 0x0007)
        {
            DEBUG_ENCRYPT("TripleDesCBCEncryptOnePacket(0x%08x, 0x%08x, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, %d, %d)\n", KEY, EnData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail);
            DEBUG_ENCRYPT("Error: The source data size(%d) must be multiple of 8 when DATA_TAIL=0 in DES encode mode!!!\n", len);
            return 0;
        }
    }

    OSSemPend(EncryptReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if(Head)
    {
        DES_CTRL        = DES_INIT;
        memcpy((void*)&DES_IV,         IVData,   8);
        memcpy((void*)&DES_KEY1,       KEY,      24);
        memcpy((void*)&SHA_K1,         Sha_K1,   64);
        memcpy((void*)&SHA_PMAC_HEAD,  Sha_PMAC, 13);
        memcpy((void*)&SHA_K2,         Sha_K2,   64);
    }
    DES_SOURCE_ADDR = (u32)EnData;
    DES_OUTPUT_ADDR = (u32)OData;
    DES_CTRL        = TRIG_ENCODE | (Tail ? DATA_TAIL : 0) | (Head ? DATA_HEAD : 0) | CODE_DES | ((len - 1) << 16);

    OSSemPend(EncryptCpleSemEvt, ENCRYPT_TIMEOUT, &err);
    
    if (err != OS_NO_ERR)
    {
        // reset encrypt hardware
        DES_CTRL        = DES_INIT;
        
        DEBUG_MP4("Encrypt Error: EncryptCpleSemEvt is %d.\n", err);
        OSSemPost(EncryptReadySemEvt);
        return 0;
    }

    OSSemPost(EncryptReadySemEvt);
    return 1;
}

/*

Routine Description:

    Encrypting data using the Triple DES algorithm in CBC mode.

Arguments:

    KEY:        The key to be used for encryption.
    EnData:     The input data to be encrypted.
    TotalLen:   The input data length.
    IVData:     The initialization vector to be used, will be updated after this function call.
    OData:      The buffer which will be filled by the encrypted data.
    Sha_K1:     SHA extended key K1 for TLSV1 format (big-endian order).
    Sha_K2:     SHA extended key K2 for TLSV1 format (big-endian order).
    Sha_PMAC:   SHA PMAC data header for TLSV1 format (big-endian order).
    PacketLen:  Packet length, The packet length must be multiple of 8 when DATA_TAIL=0 in DES encode mode, and multiple of 16 for AES.

Return Value:

    0           - Failure.
    Otherwise   - Output data length.

*/
int TripleDesCBCEncrypt(u8 *KEY, u8 *EnData, u32 TotalLen, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u32 PacketLen)
{
    int i;
    u8  Head, Tail;
    u32 len;

    Head    = 1;
    Tail    = 0;
    for(i = 0; i < TotalLen; i += PacketLen)
    {
        if((TotalLen - i) <= PacketLen)
        {
            len     = TotalLen - i;
            Tail    = 1;
        }
        else
        {
            len     = PacketLen;
        }
        if(!TripleDesCBCEncryptOnePacket(KEY, EnData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail))
        {
            return 0;
        }
        EnData += len;
        OData  += len;
        Head    = 0;
    }
    return (TotalLen + 28) & ~7;
}


/*

Routine Description:

    Decrypting a data packet using the Triple DES algorithm in CBC mode.

Arguments:

    KEY:        The key to be used for decryption.
    DeData:     The input data to be decrypted.
    len:        The input data length.
    IVData:     The initialization vector to be used, will be updated after this function call.
    OData:      The buffer which will be filled by the encrypted data.
    Sha_K1:     SHA extended key K1 for TLSV1 format (big-endian order).
    Sha_K2:     SHA extended key K2 for TLSV1 format (big-endian order).
    Sha_PMAC:   SHA PMAC data header for TLSV1 format (big-endian order).
    Head:       1: Indicate the triggered data is the end of transfer data, 0: otherwise.
    Tail:       1: Indicate the triggered data is the beginning of the transfer data, 0: otherwise.

Return Value:

    0 - Failure.
    1 - Success.

*/
int TripleDesCBCDecryptOnePacket(u8 *KEY, u8 *DeData, u32 len, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u8 Head, u8 Tail)
{
    u8  err;
    int i;

    if(len == 0)
    {
        DEBUG_ENCRYPT("TripleDesCBCDecryptOnePacket(0x%08x, 0x%08x, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, %d, %d)\n", KEY, DeData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail);
        DEBUG_ENCRYPT("Error: The source data size(%d) can't equal zero!!!\n", len);
        return 0;
    }
    if(len & 0x0007)
    {
        DEBUG_ENCRYPT("TripleDesCBCDecryptOnePacket(0x%08x, 0x%08x, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, %d, %d)\n", KEY, DeData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail);
        DEBUG_ENCRYPT("Error: The source data size(%d) must be multiple of 8 in DES decode mode!!!\n", len);
        return 0;
    }

    OSSemPend(EncryptReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if(Head)
    {
        DES_CTRL        = DES_INIT;
        memcpy((void*)&DES_IV,         IVData,   8);
        memcpy((void*)&DES_KEY1,       KEY,      24);
        memcpy((void*)&SHA_K1,         Sha_K1,   64);
        memcpy((void*)&SHA_PMAC_HEAD,  Sha_PMAC, 13);
        memcpy((void*)&SHA_K2,         Sha_K2,   64);
    }
    DES_SOURCE_ADDR = (u32)DeData;
    DES_OUTPUT_ADDR = (u32)OData;
    DES_CTRL        = TRIG_DECODE | (Tail ? DATA_TAIL : 0) | (Head ? DATA_HEAD : 0) | CODE_DES | ((len - 1) << 16);

    OSSemPend(EncryptCpleSemEvt, ENCRYPT_TIMEOUT, &err);
    
    if (err != OS_NO_ERR)
    {
        // reset encrypt hardware
        DES_CTRL        = DES_INIT;
        
        DEBUG_MP4("Encrypt Error: EncryptCpleSemEvt is %d.\n", err);
        OSSemPost(EncryptReadySemEvt);
        return 0;
    }

    OSSemPost(EncryptReadySemEvt);
    return 1;
}

/*

Routine Description:

    Decrypting data using the Triple DES algorithm in CBC mode.

Arguments:

    KEY:        The key to be used for decryption.
    DeData:     The input data to be decrypted.
    TotalLen:   The input data length.
    IVData:     The initialization vector to be used, will be updated after this function call.
    OData:      The buffer which will be filled by the encrypted data.
    Sha_K1:     SHA extended key K1 for TLSV1 format (big-endian order).
    Sha_K2:     SHA extended key K2 for TLSV1 format (big-endian order).
    Sha_PMAC:   SHA PMAC data header for TLSV1 format (big-endian order).
    PacketLen:  Packet length, The packet length must be multiple of 8 when DATA_TAIL=0 in DES encode mode, and multiple of 16 for AES.

Return Value:

    0           - Failure.
    Otherwise   - Output original data length(not include SHA and padding).

*/
int TripleDesCBCDecrypt(u8 *KEY, u8 *DeData, u32 TotalLen, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u32 PacketLen)
{
    int i;
    u8  Head, Tail;
    u32 len;

    Head    = 1;
    Tail    = 0;
    for(i = 0; i < TotalLen; i += PacketLen)
    {
        if((TotalLen - i) <= PacketLen)
        {
            len     = TotalLen - i;
            Tail    = 1;
        }
        else
        {
            len     = PacketLen;
        }
        if(!TripleDesCBCDecryptOnePacket(KEY, DeData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail))
        {
            return 0;
        }
        DeData += len;
        OData  += len;
        Head    = 0;
    }
    len     = TotalLen - (u32)*(OData - 1) - 21;
    return len;
}

/*

Routine Description:

    Encrypting a data packet using the AES algorithm in CBC mode.

Arguments:

    KEY:        The key to be used for encryption.
    EnData:     The input data to be encrypted.
    len:        The input data length.
    IVData:     The initialization vector to be used, will be updated after this function call.
    OData:      The buffer which will be filled by the encrypted data.
    Sha_K1:     SHA extended key K1 for TLSV1 format (big-endian order).
    Sha_K2:     SHA extended key K2 for TLSV1 format (big-endian order).
    Sha_PMAC:   SHA PMAC data header for TLSV1 format (big-endian order).
    Head:       1: Indicate the triggered data is the end of transfer data, 0: otherwise.
    Tail:       1: Indicate the triggered data is the beginning of the transfer data, 0: otherwise.

Return Value:

    0 - Failure.
    1 - Success.

*/
int TripleAesCBCEncryptOnePacket(u8 *KEY, u8 *EnData, u32 len, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u8 Head, u8 Tail)
{
    u8  err;
    int i;

    if(len == 0)
    {
        DEBUG_ENCRYPT("TripleAesCBCEncryptOnePacket(0x%08x, 0x%08x, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, %d, %d)\n", KEY, EnData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail);
        DEBUG_ENCRYPT("Error: The source data size(%d) can't equal zero!!!\n", len);
        return 0;
    }
    if(!Tail)
    {
        if(len & 0x000f)
        {
            DEBUG_ENCRYPT("TripleAesCBCEncryptOnePacket(0x%08x, 0x%08x, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, %d, %d)\n", KEY, EnData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail);
            DEBUG_ENCRYPT("Error: The source data size(%d) must be multiple of 16 when DATA_TAIL=0 in AES encode mode!!!\n", len);
            return 0;
        }
    }

    OSSemPend(EncryptReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if(Head)
    {
        DES_CTRL        = DES_INIT;
        memcpy((void*)&DES_IV,         IVData,   16);
        memcpy((void*)&DES_KEY2,       KEY,      16);
        memcpy((void*)&SHA_K1,         Sha_K1,   64);
        memcpy((void*)&SHA_PMAC_HEAD,  Sha_PMAC, 13);
        memcpy((void*)&SHA_K2,         Sha_K2,   64);
    }
    DES_SOURCE_ADDR = (u32)EnData;
    DES_OUTPUT_ADDR = (u32)OData;
    DES_CTRL        = TRIG_ENCODE | (Tail ? DATA_TAIL : 0) | (Head ? DATA_HEAD : 0) | CODE_AES | ((len - 1) << 16);

    OSSemPend(EncryptCpleSemEvt, ENCRYPT_TIMEOUT, &err);
    
    if (err != OS_NO_ERR)
    {
        // reset encrypt hardware
        DES_CTRL        = DES_INIT;
        
        DEBUG_MP4("Encrypt Error: EncryptCpleSemEvt is %d.\n", err);
        OSSemPost(EncryptReadySemEvt);
        return 0;
    }

    OSSemPost(EncryptReadySemEvt);
    return 1;
}

/*

Routine Description:

    Encrypting data using the AES algorithm in CBC mode.

Arguments:

    KEY:        The key to be used for encryption.
    EnData:     The input data to be encrypted.
    TotalLen:   The input data length.
    IVData:     The initialization vector to be used, will be updated after this function call.
    OData:      The buffer which will be filled by the encrypted data.
    Sha_K1:     SHA extended key K1 for TLSV1 format (big-endian order).
    Sha_K2:     SHA extended key K2 for TLSV1 format (big-endian order).
    Sha_PMAC:   SHA PMAC data header for TLSV1 format (big-endian order).
    PacketLen:  Packet length, The packet length must be multiple of 8 when DATA_TAIL=0 in DES encode mode, and multiple of 16 for AES.

Return Value:

    0           - Failure.
    Otherwise   - Output data length.

*/
int TripleAesCBCEncrypt(u8 *KEY, u8 *EnData, u32 TotalLen, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u32 PacketLen)
{
    int i;
    u8  Head, Tail;
    u32 len;

    Head    = 1;
    Tail    = 0;
    for(i = 0; i < TotalLen; i += PacketLen)
    {
        if((TotalLen - i) <= PacketLen)
        {
            len     = TotalLen - i;
            Tail    = 1;
        }
        else
        {
            len     = PacketLen;
        }
        if(!TripleAesCBCEncryptOnePacket(KEY, EnData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail))
        {
            return 0;
        }
        EnData += len;
        OData  += len;
        Head    = 0;
    }
    return (TotalLen + 36) & ~15;
}

/*

Routine Description:

    Decrypting a data packet using the AES algorithm in CBC mode.

Arguments:

    KEY:        The key to be used for decryption.
    DeData:     The input data to be decrypted.
    len:        The input data length.
    IVData:     The initialization vector to be used, will be updated after this function call.
    OData:      The buffer which will be filled by the encrypted data.
    Sha_K1:     SHA extended key K1 for TLSV1 format (big-endian order).
    Sha_K2:     SHA extended key K2 for TLSV1 format (big-endian order).
    Sha_PMAC:   SHA PMAC data header for TLSV1 format (big-endian order).
    Head:       1: Indicate the triggered data is the end of transfer data, 0: otherwise.
    Tail:       1: Indicate the triggered data is the beginning of the transfer data, 0: otherwise.

Return Value:

    0 - Failure.
    1 - Success.

*/
int TripleAesCBCDecryptOnePacket(u8 *KEY, u8 *DeData, u32 len, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u8 Head, u8 Tail)
{
    u8  err;
    int i;

    if(len == 0)
    {
        DEBUG_ENCRYPT("TripleAesCBCDecryptOnePacket(0x%08x, 0x%08x, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, %d, %d)\n", KEY, DeData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail);
        DEBUG_ENCRYPT("Error: The source data size(%d) can't equal zero!!!\n", len);
        return 0;
    }
    if(len & 0x000f)
    {
        DEBUG_ENCRYPT("TripleAesCBCDecryptOnePacket(0x%08x, 0x%08x, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, %d, %d)\n", KEY, DeData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail);
        DEBUG_ENCRYPT("Error: The source data size(%d) must be multiple of 16 in AES decode mode!!!\n", len);
        return 0;
    }

    OSSemPend(EncryptReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if(Head)
    {
        DES_CTRL        = DES_INIT;
        memcpy((void*)&DES_IV,         IVData,   16);
        memcpy((void*)&DES_KEY2,       KEY,      16);
        memcpy((void*)&SHA_K1,         Sha_K1,   64);
        memcpy((void*)&SHA_PMAC_HEAD,  Sha_PMAC, 13);
        memcpy((void*)&SHA_K2,         Sha_K2,   64);
    }
    DES_SOURCE_ADDR = (u32)DeData;
    DES_OUTPUT_ADDR = (u32)OData;
    DES_CTRL        = TRIG_DECODE | (Tail ? DATA_TAIL : 0) | (Head ? DATA_HEAD : 0) | CODE_AES | ((len - 1) << 16);

    OSSemPend(EncryptCpleSemEvt, ENCRYPT_TIMEOUT, &err);
    
    if (err != OS_NO_ERR)
    {
        // reset encrypt hardware
        DES_CTRL        = DES_INIT;
        
        DEBUG_MP4("Encrypt Error: EncryptCpleSemEvt is %d.\n", err);
        OSSemPost(EncryptReadySemEvt);
        return 0;
    }

    OSSemPost(EncryptReadySemEvt);
    return 1;
}

/*

Routine Description:

    Decrypting data using the AES algorithm in CBC mode.

Arguments:

    KEY:        The key to be used for decryption.
    DeData:     The input data to be decrypted.
    TotalLen:   The input data length.
    IVData:     The initialization vector to be used, will be updated after this function call.
    OData:      The buffer which will be filled by the encrypted data.
    Sha_K1:     SHA extended key K1 for TLSV1 format (big-endian order).
    Sha_K2:     SHA extended key K2 for TLSV1 format (big-endian order).
    Sha_PMAC:   SHA PMAC data header for TLSV1 format (big-endian order).
    PacketLen:  Packet length, The packet length must be multiple of 8 when DATA_TAIL=0 in DES encode mode, and multiple of 16 for AES.

Return Value:

    0           - Failure.
    Otherwise   - Output original data length(not include SHA and padding).

*/
int TripleAesCBCDecrypt(u8 *KEY, u8 *DeData, u32 TotalLen, u8 *IVData, u8 *OData, u8 *Sha_K1, u8 *Sha_K2, u8 *Sha_PMAC, u32 PacketLen)
{
    int i;
    u8  Head, Tail;
    u32 len;

    Head    = 1;
    Tail    = 0;
    for(i = 0; i < TotalLen; i += PacketLen)
    {
        if((TotalLen - i) <= PacketLen)
        {
            len     = TotalLen - i;
            Tail    = 1;
        }
        else
        {
            len     = PacketLen;
        }
        if(!TripleAesCBCDecryptOnePacket(KEY, DeData, len, IVData, OData, Sha_K1, Sha_K2, Sha_PMAC, Head, Tail))
        {
            return 0;
        }
        DeData += len;
        OData  += len;
        Head    = 0;
    }
    len     = TotalLen - (u32)*(OData - 1) - 21;
    return len;
}


