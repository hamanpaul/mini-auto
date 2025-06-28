/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	VideoCodec_common.h

Abstract:

   	The declarations of VideoCodec_common.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/06	Lsk	Create	

*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include "VideoCodecAPI.h"


/*
*********************************************************************************************************
* Constant
*********************************************************************************************************
*/
/*----------------- type definition --------------*/



/*
**************************************************************************
 * Entry definition
**************************************************************************
*/

/*
*********************************************************************************************************
* Variable
*********************************************************************************************************
*/
extern u32 VideoPictureIndex;  /*BJ 0530 S*/   /*CY 0907*/
extern u32 VideoSmallPictureIndex;  /*BJ 0530 S*/   /*CY 0907*/

#define I_FRAME           1
#define P_FRAME           0


#endif

