/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Interface Spec		:	None.

* Description			:	This file contains the utility functions needed by SSL.

* Author Name			:	M.K.Saravanan

* Revision				:	1.0

* Known Bugs			:	None

******************************************************************************************/

#include <SSLCommon.h>

/*****************************************************************************************
* Function Name	:	SSLSwap32
* Parameters	:	UINT32 ulNumber	- (IN) The number to be swaped
* Return Value	:	UINT32 - Swaped value
* Description	:	This function swaps the bytes of a 4 byte number
******************************************************************************************/
UINT32 SSLSwap32(UINT32 ulNumber)
{
	UINT32 ulResult;
	INT32 lResCount,lNumCount;
	UINT8 *pResult,*pNumber;

	ulResult = 0;

	/* gets the pointer of result and input numbers */
	pResult = (UINT8 *)&ulResult;
	pNumber = (UINT8 *)&ulNumber;

	for(lResCount=0,lNumCount=3;lResCount<4;lResCount++,lNumCount--) {
		/* stores the value swaped in the result */
		pResult[lResCount] = pNumber[lNumCount];
	}
	/* returns the swaped number */
	return(ulResult);
}

/*****************************************************************************************
* Function Name	:	SSLSwap16
* Parameters	:	UINT16 uNumber	- (IN) The number to be swaped
* Return Value	:	UINT16 - Swaped value
* Description	:	This function swaps the bytes of a 2 byte number
******************************************************************************************/
UINT16 SSLSwap16(UINT16 uNumber)
{
	UINT16 uResult;
	INT32 lResCount,lNumCount;
	UINT8 *pResult,*pNumber;

	uResult = 0;

	/* gets the pointer of result and input numbers */
	pResult = (UINT8 *)&uResult;
	pNumber = (UINT8 *)&uNumber;

	for(lResCount=0,lNumCount=1;lResCount<2;lResCount++,lNumCount--)
	{
		/* stores the value swaped in the result */
		pResult[lResCount] = pNumber[lNumCount];
	}
	
	/* returns the swaped number */
	return(uResult);
}

/*****************************************************************************************
* Function Name	:	SSLWriteUint16
* Parameters	:	UINT16 uNumber	- (IN) The number to be swaped
					UINT8 *pData - (OUT) The pointer where uNumber is copied
* Return Value	:	None.
* Description	:	This function writes the 2 byte number to data stream.
******************************************************************************************/
void SSLWriteUint16( UINT16 uNumber, UINT8 *pData) 
{
	/* writes the 2 byte number to a byte stream, in case of little endian processor
	swaps before wrting it */
#if SSL_CFG_LITTLE_ENDIAN
	uNumber = SSLSwap16(uNumber);
#endif
	memcpy( pData, &uNumber, sizeof(UINT16));
	return;
}

/*****************************************************************************************
* Function Name	:	SSLReadUint16
* Parameters	:	UINT8 *pData - (IN) The pointer where 2 byte number is stored
* Return Value	:	UINT16 - 2 byte value read
* Description	:	This function reads the 2 byte number from data stream.
******************************************************************************************/
UINT16 SSLReadUint16(UINT8 *pData)
{
	UINT16 result;
	
	/* reads the 2 byte number from a byte stream, in case of little endian processor
	swaps before returnning it */
	memcpy( &result, pData, sizeof(UINT16));
#if SSL_CFG_LITTLE_ENDIAN
	result = SSLSwap16(result);
#endif
	return result;
}

/*****************************************************************************************
* Function Name	:	SSLWriteUint64
* Parameters	:	SSLU64	*pNumber - (IN) The 8 byte number
					UINT8 *pData - (OUT) The pointer where 8 byte number is stored
* Return Value	:	None.
* Description	:	This function writes the 8 byte number to data stream.
******************************************************************************************/
void SSLWriteUint64( SSLU64 *pNumber, UINT8 *pData)
{
	memcpy(pData, &pNumber->u64Byte[0], sizeof(SSLU64));
	return;
}

/*****************************************************************************************
* Function Name	:	SSLZeroUint64
* Parameters	:	SSLU64	*pNumber - (IN) The 8 byte number
* Return Value	:	None.
* Description	:	This function initialises the 8 byte number to zero.
******************************************************************************************/
void SSLZeroUint64(SSLU64 *pNumber)
{
	memset( &pNumber->u64Byte[0], 0, sizeof(SSLU64));
	return;
}

/*****************************************************************************************
* Function Name	:	SSLWriteUint64
* Parameters	:	SSLU64	*pNumber - (IN) The 8 byte number to be incremented
* Return Value	:	INT32 - SSL_SUCCESS(1) if incremented
							SSL_FAILURE(0) if max limit is reached.
* Description	:	This function increments the 8 byte number by one.
******************************************************************************************/

INT32 SSLIncrementUint64(SSLU64 *pU64Num) 
{
	INT32 lCount;
	INT32 lFlag;

	for (lCount = 7;lCount >= 0;lCount--) {
		lFlag = 0;
		if ( pU64Num->u64Byte[lCount] == 0xff) {
			/* this byte is over increment MSB and make this byte 0 */
			pU64Num->u64Byte[lCount] = 0;
			lFlag = 1;
		} else {
			/* Increment by one */
			pU64Num->u64Byte[lCount]++;
		}
		
		/* exit in case all the bits are over or if it is succesfully incremented */
		if (lFlag == 0) break;
	}
	
	if (lFlag != 0) return SSL_FAILURE; /* over 64 bits! */

	return SSL_SUCCESS;
}

/*****************************************************************************************
* Function Name	:	SSLReadUint24
* Parameters	:	UINT8 *pData - (IN) The pointer to 24 bit data.
* Return Value	:	UINT32 - The 3 byte number as 4 byte value
* Description	:	This function reads the 3 byte value from the data stream
******************************************************************************************/
UINT32 SSLReadUint24(UINT8 *pData)
{
	UINT32 ulResult;

	ulResult=0;

	/* reads the 3 byte number from a byte stream, in case of little endian processor
	swaps before returnning it */

	((UINT8 *)&ulResult)[1] = pData[0];
	((UINT8 *)&ulResult)[2] = pData[1];
	((UINT8 *)&ulResult)[3] = pData[2];

#if SSL_CFG_LITTLE_ENDIAN
	ulResult = SSLSwap32(ulResult);
#endif
	return ulResult;
}

/*****************************************************************************************
* Function Name	:	SSLWriteUint24
* Parameters	:	UINT32 ulNumber - (IN) The 3 byte number
					UINT8 *pData	- (OUT) The pointer where 3 byte number is copied
* Return Value	:	None.
* Description	:	This function writes the 3 byte value to the data stream
******************************************************************************************/
void SSLWriteUint24(UINT32 ulNumber, UINT8 *pData)
{
	/* writes the 3 byte number to a byte stream, in case of little endian processor
	swaps before wrting it */

#if SSL_CFG_LITTLE_ENDIAN
	ulNumber = SSLSwap32(ulNumber);
#endif

	pData[0] = ((UINT8 *)&ulNumber)[1];
	pData[1] = ((UINT8 *)&ulNumber)[2];
	pData[2] = ((UINT8 *)&ulNumber)[3];
	return;
}
