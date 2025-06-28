/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	BLE.h

Abstract:

   	The declarations of BT 4.0 interface.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2016/09/30    Amy Create  

*/

#ifndef __BLE_H__
#define __BLE_H__


enum
{
    BLE_CMD_BIT_PREAMBLE=0,
    BLE_CMD_BIT_PREAMBLE2,
    BLE_CMD_BIT_CMD1,
    BLE_CMD_BIT_CMD2,
    BLE_CMD_BIT_CMD3,
    BLE_CMD_BIT_CMD4,
    BLE_CMD_BIT_ANS
};
//TBD need or not?
enum
{
    BLE_CMD_VERSION,
    BLE_CMD_SET_ALARM,
    BLE_CMD_SCAN_DEV,
    BLE_CMD_TIME_SYNC,
    BLE_CMD_CONNECT_DEV,
    BLE_CMD_UNPAIR_DEV,
    BLE_CMD_ACK
};

#define BLE_MASTER_PREAMBLE             0x58
#define BLE_CMD_SET_ALARM_PREAMBLE      0x50
#define BLE_CMD_SCAN_DEV_PREAMBLE       0x51
#define BLE_CMD_TIME_SYNC_PREAMBLE      0x52
#define BLE_CMD_CONNECT_DEV_PREAMBLE    0x53
#define BLE_CMD_UNPAIR_DEV_PREAMBLE     0x54
#define BLE_CMD_TIMEREADY_DEV_PREAMBLE  0x55
#define BLE_CMD_DISCONNECT_DEV_PREAMBLE 0x56
#define BLE_CMD_ACK_PREAMBLE            0x57

#define BLE_CMD_ACK_BYTE                0x96
#define BLE_CMD_ACK_ERR_BYTE            0xB5


/*Head Byte*/

#define BLE_RSV_PREAMBLE_A           0x41 //"A"
#define BLE_RSV_PREAMBLE_C           0x43 //"C"
#define BLE_RSV_PREAMBLE_N           0x4E //"N"

#define BLE_RSV_PREAMBLE_O           0x4F //"O"
#define BLE_RSV_PREAMBLE_K           0x4B //"K"


/*Send command type*/



/*Recevie command type */


/*Sensor Type */


/*Sensor Sub Type*/


/*Remote Key Type*/


/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

 
/*
 *********************************************************************************************************
 * Data Type
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Extern Global Variable
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

#endif
