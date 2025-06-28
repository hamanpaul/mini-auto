/*--------------------------------------------------------------------------------
DES.C: DES Algorithm Program from the Book Appliced Cryptography, Bruce Schneier
--------------------------------------------------------------------------------*/

#include "general.h"
#include "board.h"
#include "des.h"
#include "encrptyapi.h"


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


#if (AES_DES_SUPPORT || AESDES_TEST)
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
#endif

