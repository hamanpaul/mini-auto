
#include <SSLCommon.h>


#define SESSION_RESUME		1



#if SESSION_RESUME

typedef struct HTPSSLCache {
	UINT8 *pSessionID;
	UINT32 ulIDLength;
	UINT8 *pData;
	struct HTPSSLCache *pNext;
}HTTPSSLCache;


INT32	HTTPStoreSession (UINT8 *pSessionID, UINT32 ulIDLength, UINT8 *pData, UINT32 ulDataLength);
void *	HTTPRetrieveSession(UINT8 *pSessionID, UINT32 ulIDLength);
void	HTTPRemoveSession (UINT8 *pSessionID, UINT32 ulIDLength);


#endif // SESSION_RESUME