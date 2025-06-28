
//#include <web/src/Engine/Includes/AsEngine.h>
//#include <web/src/Engine/Includes/AsProtos.h>

#include "dlk_ssl.h"


extern SSL_EPM_SECTION_T		*ssl_info;

#if SESSION_RESUME


HTTPSSLCache *pCacheHead=NULL;

static void *pHTTPSMemPool = NULL;

void HTTPRemoveCache(void)
{
	HTTPSSLCache *pCacheTemp;
	HTTPSSLCache *pNextNode;

	pCacheTemp = pCacheHead;
	while(pCacheTemp) {
		pNextNode = pCacheTemp->pNext;
		OS_ADPT_Deallocate_Memory(pCacheTemp->pData);
		OS_ADPT_Deallocate_Memory(pCacheTemp->pSessionID);
		OS_ADPT_Deallocate_Memory(pCacheTemp);
		pCacheTemp = pNextNode;
	}
	pCacheHead = NULL;
}

INT32	HTTPStoreSession (UINT8 *pSessionID, UINT32 ulIDLength, UINT8 *pData, UINT32 ulDataLength)
{
	HTTPSSLCache *pCacheTemp;
	INT32 i32Count=0;
	pCacheTemp = pCacheHead;
	while(pCacheTemp) {
		if(pCacheTemp->ulIDLength == ulIDLength) {
			if(memcmp(pCacheTemp->pSessionID, pSessionID, ulIDLength) == 0) {
				return 0;
			}
		}
		i32Count++;
		pCacheTemp = pCacheTemp->pNext;
	}
	if(i32Count >= 30) {
		HTTPRemoveCache();
	}
	
    	if (pHTTPSMemPool == NULL)
        	pHTTPSMemPool = OS_ADPT_Create_Memory_Pool("HTTP", 200*1024, 16);
        	
	pCacheTemp = (HTTPSSLCache *)OS_ADPT_Allocate_Memory(pHTTPSMemPool, sizeof(HTTPSSLCache));
	pCacheTemp->pData = (UINT8 *)OS_ADPT_Allocate_Memory(pHTTPSMemPool, ulDataLength);
	pCacheTemp->pSessionID = (UINT8 *)OS_ADPT_Allocate_Memory(pHTTPSMemPool, ulIDLength);
	
	
	pCacheTemp->ulIDLength = ulIDLength;
	memcpy(pCacheTemp->pData,pData,ulDataLength);
	memcpy(pCacheTemp->pSessionID, pSessionID, ulIDLength);
	
	pCacheHead = pCacheTemp;
	
	return 1;
}

void *	HTTPRetrieveSession(UINT8 *pSessionID, UINT32 ulIDLength)
{
	HTTPSSLCache *pCacheTemp;
	pCacheTemp = pCacheHead;
	while(pCacheTemp) {
		if(pCacheTemp->ulIDLength == ulIDLength) {
			if(memcmp(pCacheTemp->pSessionID, pSessionID, ulIDLength) == 0) {
				return pCacheTemp->pData;
			}
		}
		pCacheTemp = pCacheTemp->pNext;
	}
	return NULL;
}


void	HTTPRemoveSession (UINT8 *pSessionID, UINT32 ulIDLength)
{
	HTTPSSLCache *pCacheTemp;
	HTTPSSLCache *pPrev=NULL;

	pCacheTemp = pCacheHead;
	while(pCacheTemp) {
		if(pCacheTemp->ulIDLength == ulIDLength) {
			if(memcmp(pCacheTemp->pSessionID, pSessionID, ulIDLength) == 0) {
				if(pPrev == NULL) {
					pCacheHead = pCacheTemp->pNext;
				} else {
					pPrev->pNext = pCacheTemp->pNext;
				}
				OS_ADPT_Deallocate_Memory(pCacheTemp->pData);
				OS_ADPT_Deallocate_Memory(pCacheTemp->pSessionID);
				OS_ADPT_Deallocate_Memory(pCacheTemp);
				break;
			}
		}
		pPrev = pCacheTemp;
		pCacheTemp = pCacheTemp->pNext;
	}
}

#endif //#SESSION_RESUME

/*----------------------------------------------------------------------------
 * UINT32 SSL_CacheTimeout_Get(void)
 * Purpose: This funtion will return the ssl cache timeout value
 * Parameters:			void 
 * Return:  			UINT32
 *----------------------------------------------------------------------------
 */
UINT32	SSL_CacheTimeout_Get(void)
{
	if(ssl_info->u32TimerTimeout)
		return ssl_info->u32TimerTimeout;
	else
		return NULL;
}

/*----------------------------------------------------------------------------
 * int	SSL_CacheTimeout_Set(UINT32 value)
 * Purpose: This funtion will return the ssl cache timeout value
 * Parameters:			void 
 * Return:  			UINT32
 *----------------------------------------------------------------------------
 */
 
int	SSL_CacheTimeout_Set(UINT32 value)
{
	if((value < SSL_MIN_TIMEOUT) || (value > SSL_MAX_TIMEOUT))
	{	
		return 0;
	}
	
	ssl_info->u32TimerTimeout = value;
	
	return TRUE;
}
	