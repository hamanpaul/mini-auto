/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Interface Spec		:	None.

* Description			:	This file contains buffer manipulation routines

* Author Name			:	M.K.Saravanan

* Revision				:	1.0

* Known Bugs			:	None

******************************************************************************************/
#define EXTERN_SSL_MEMORY
#include <SSLCommon.h>

/*****************************************************************************************
* Function Name	:	SSLGetBufferSize
* Parameters	:	SSLBuffer *pBuffer	- (IN) The buffer for which size is requested
* Return Value	:	INT32 - Returns buffer size.
* Description	:	This function returns the size of the buffer passed.
******************************************************************************************/
INT32 SSLGetBufferSize(SSLBuffer *pBuffer)
{
	return pBuffer->ulSize;
}

/*****************************************************************************************
* Function Name	:	SSLInsertBuffer
* Parameters	:	SSLBuffer *pBuffer	- (IN) The buffer pointer
					UINT8 *pData - (IN) The data to be inserted in the buffer
					INT32 lDataSize - (IN) The data length
					
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function inserts the data at the end of buffer
******************************************************************************************/
INT32 SSLInsertBuffer(SSLBuffer *pBuffer, UINT8 *pData, INT32 lDataSize)
{
	UINT8 *pTempData;
	INT32 lTempSize;
	INT32 lDataPtr;

	if(lDataSize == 0) {
		return SSL_SUCCESS;
	}

	lTempSize = lDataSize + pBuffer->ulSize; /* total size of the buffer */
	/* allocate memory for new buffer */
	pTempData = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory,lTempSize);
	if(pTempData == NULL) {
		return SSL_FAILURE;
	}

	lDataPtr = pBuffer->ulSize;
	if(pBuffer->ulSize != 0) {
		/* If data already exists in the buffer copy it first */
		memcpy(pTempData, pBuffer->pData,pBuffer->ulSize);
		OS_ADPT_Deallocate_Memory(pBuffer->pData);
		pBuffer->ulSize = 0;
	}

	/* copy the new data */
	memcpy(&pTempData[lDataPtr],pData, lDataSize);

	/* set the new pointer and siz ein the buffer */
	pBuffer->pData = pTempData;
	pBuffer->ulSize = lTempSize;
	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLRetrieveBuffer
* Parameters	:	SSLBuffer *pBuffer	- (IN) The buffer pointer
					UINT8 *pData - (OUT) The pointer to output data
					INT32 lDataSize - (IN) The data length
					
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function removes the data from the buffer and copies it to the
					pData pointer. If pData is NULL it is used to remove the specified
					no of bytes from the buffer.
******************************************************************************************/
INT32 SSLRetrieveBuffer(SSLBuffer *pBuffer, UINT8 *pData, INT32 lDataSize)
{
	INT32 lBufferSize;

	lBufferSize = SSLGetBufferSize(pBuffer); /* gets the buffer size */
	if(lDataSize > lBufferSize || lDataSize == 0) {
		/* if requested size is more than available return error */
		return SSL_FAILURE;
	}

	/* in case data needs to be copied to user pointer just copy it. If user passes
	NULL just remove the data from the buffer */

	if(pData != NULL) {
		memcpy(pData, pBuffer->pData, lDataSize);
	}

	pBuffer->ulSize -= lDataSize;

	/* deallocate total memory when only becomes empty. In case only part of data
	is retrieved just memmove it */
	if(pBuffer->ulSize != 0) {
		memmove(pBuffer->pData, &pBuffer->pData[lDataSize],pBuffer->ulSize);
	} else {
		OS_ADPT_Deallocate_Memory(pBuffer->pData);
	}
	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLPeekBuffer
* Parameters	:	SSLBuffer *pBuffer	- (IN) The buffer pointer
					UINT8 *pData - (OUT) The pointer to output data
					INT32 lDataSize - (IN) The data length
					
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This function copies the data to pData without removig it from the 
					buffer
******************************************************************************************/
INT32 SSLPeekBuffer(SSLBuffer *pBuffer, UINT8 *pData, INT32 lDataSize)
{
	INT32 lBufferSize;

	lBufferSize = SSLGetBufferSize(pBuffer);
	if(lDataSize > lBufferSize || lDataSize == 0) {
		/* if requested size is more than available return error */
		return SSL_FAILURE;
	}

	/* copies the data without actually removing it from the buffer */
	if(pData != NULL) {
		memcpy(pData, pBuffer->pData, lDataSize);
	}
	return SSL_SUCCESS;
}
