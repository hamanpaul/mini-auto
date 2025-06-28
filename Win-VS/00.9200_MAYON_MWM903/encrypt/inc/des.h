#ifndef __DES_H__
#define __DES_H__


/* DES/AES control register DES_CTRL parameter */
#define TRIG_ENCODE             0x00000001
#define TRIG_DECODE             0x00000002
#define DATA_TAIL               0x00000004
#define DATA_HEAD               0x00000008
#define DES_INIT                0x00000010
#define CODE_AES                0x00000020
#define CODE_DES                0x00000000
#define DES_FINISH              0x00000100
#define SHA_READY               0x00000200
#define AUTH1_READY             0x00000400
#define MASK_DES                0x00001000
#define MASK_SHA                0x00002000
#define MASK_AUTH1              0x00004000

#define ENCRYPT_TIMEOUT         20

typedef struct {
  unsigned long ek[32];
  unsigned long dk[32];
} des_ctx;

void des_key(des_ctx *dc, unsigned char *key);


/* Encrypt several blocks in ECB mode.  Caller is responsible for
   short blocks. */
void des_enc(des_ctx *dc, unsigned char *Data, int blocks);
void des_dec(des_ctx *dc, unsigned char *Data, int blocks);
#endif
