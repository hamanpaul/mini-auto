/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	HomeRF.h

Abstract:

   	The declarations of Home sensor interface.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2014/09/16    Roy Create  

*/

#ifndef __HOMERF_H__
#define __HOMERF_H__




#if((HOME_RF_OPTION == HOME_SENSOR_MARS) || (HOME_RF_OPTION == HOME_SENSOR_BARVOTECH) || (HOME_RF_OPTION == HOME_SENSOR_SWANN) || (HOME_RF_OPTION == HOME_SENSOR_TRANWO))
enum
{
    HOMERF_CMD_BIT_PREAMBLE=0,
    HOMERF_CMD_BIT_PREAMBLE2,
    HOMERF_CMD_BIT_LEN,
    HOMERF_CMD_BIT_CMD,
};


/*Head Byte*/
#define HOMERF_PREAMBLE_BYTE1               0x69
#define HOMERF_PREAMBLE_BYTE2               0x96


/*Send command type*/
#define HOMERF_TX_CMDID_QUERY_ALL_STATUS    0x01
#define HOMERF_TX_CMDID_QUERY_ALL_PAIR      0x02
#define HOMERF_TX_CMDID_QUERY_ONE_STATUS    0x03
#define HOMERF_TX_CMDID_DELETE_ALL          0x04
#define HOMERF_TX_CMDID_DELETE_ONE          0x05
#define HOMERF_TX_CMDID_VERSION             0x06
#define HOMERF_TX_CMDID_SET_AES_KEY         0x07
#define HOMERF_TX_CMDID_CLEAR_AES_KEY       0x08
#define HOMERF_TX_CMDID_SET_SENSOR          0x09
#define HOMERF_TX_CMDID_ALARM_MODE          0x0A
#define HOMERF_TX_CMDID_PAIR_MODE           0x0B
#define HOMERF_TX_CMDID_FREQ_SELECT         0x0C
#define HOMERF_TX_CMDID_SET_RFID            0x0D
#define HOMERF_TX_CMDID_NEW_FW_VERSION      0x0E

/*Recevie command type */
#define HOMERF_RX_CMDID_TRIG_EVENET         0x11
#define HOMERF_RX_CMDID_PERIOD_EVENT        0x12
#define HOMERF_RX_CMDID_PAIR_MODE           0x13
#define HOMERF_RX_CMDID_GET_FW_DATA         0x14
#define HOMERF_RX_CMDID_GET_FW_STATUS       0x15
#define HOMERF_RX_CMDID_SENSOR_STATUS       0x83
#define HOMERF_RX_CMDID_DELETE              0x85
#define HOMERF_RX_CMDID_DELETE_ALL          0x82
#define HOMERF_RX_CMDID_SET_SENSOR          0x89


/*Sensor Type */
enum
{
    HOMERF_MARS_TYPE_UNKNOWN=0x00,
    HOMERF_MARS_TYPE_DOOR	=0x11,              
    HOMERF_MARS_TYPE_PIR,                
    HOMERF_MARS_TYPE_SIREN,              
    HOMERF_MARS_TYPE_REMOTE,             
    HOMERF_MARS_TYPE_PLUG,               
    HOMERF_MARS_TYPE_TEMP_HUMIDITY,
    HOMERF_MARS_TYPE_TEMP,               
    HOMERF_MARS_TYPE_HUMIDITY, 
    HOMERF_MARS_TYPE_LEAK,
    HOMERF_MARS_TYPE_VIBRATE,
    HOMERF_MARS_TYPE_SMOKE,
    HOMERF_MARS_TYPE_GAS,
    HOMERF_MARS_TYPE_ROUTER=0x1F,
    HOMERF_MARS_TYPE_MULTI_SENSOR=0xFE,

	HOMERF_MARS_TYPE_IAQ		= 0x20, // Indoor Air Quility
	HOMERF_MARS_TYPE_ADE		= 0x21,	// Analog Device
	HOMERF_MARS_TYPE_FDS		= 0x22,	// Fall Detection Sensor 
	HOMERF_MARS_TYPE_SWITCH		= 0x23,			

};

/*Sensor Sub Type*/
enum
{
    HOMERF_SUB_TYPE_RDI_REMOTE=0x01,
    HOMERF_SUB_TYPE_TRANWO_REMOTE,
    HOMERF_SUB_TYPE_PANIC,
    HOMERF_SUB_TYPE_TRANWO_DOOR,
    HOMERF_SUB_TYPE_TRANWO_PIR,
    HOMERF_SUB_TYPE_MEIAN_REMOTE,
};


/*Remote Key Type*/
enum
{
    HOMERF_KEY_NONE=0,
    HOMERF_KEY_ALARM,
    HOMERF_KEY_DISALARM,
    HOMERF_KEY_HOME,
    HOMERF_KEY_PANIC,
};


#endif

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */


enum
{
    HOMERF_DEL_SENSOR = 0,
    HOMERF_ADD_SENSOR,
};

enum
{
    HOMERF_PAIR_NORMAL,
    HOMERF_PAIR_SUCCESS,
    HOMERF_PAIR_FAIL,
    HOMERF_PAIR_SAME,
    HOMERF_PAIR_TIMEOUT,
};
#define HOMERF_WAiT_TIME                       10    /*1s*/


#define HOMERF_ALARM_PERIOD                    3   /*3s*/
#define HOMERF_TEMP_HUM_ALARM_PERIOD                    600   /*10 mins for Tranwo, Add by Paul@20190401*/


#define HOMERF_FLAG_SEND_ACK                   0x00000001

#define DEFAULT_SCENEID_ALARM                 0x04FFFFFF
#define DEFAULT_SCENEID_DISALARM           0x04FFFFFE
#define DEFAULT_SCENEID_INHOMEMODE      0x04FFFFFD

#define HOMERF_QUEUE_SIZE                      20
#define HOMERF_LOG_DATA_SIZE                   64
/*
 *********************************************************************************************************
 * Data Type
 *********************************************************************************************************
 */


typedef struct _HOMERF_CMD_QUEUE
{   
    u32 sID;
    u32 sType;
    u32 cmdType;
    u32 timeCount;
}HOMERF_CMD_QUEUE; 


/*
 *********************************************************************************************************
 * Extern Global Variable
 *********************************************************************************************************
 */


extern u64 homeRFPID;
extern u32 guiRFTimerID;
extern u32 guiSysTimerId;
extern u32 guiIRTimerId;

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

void homeRFSendCommand(u8 cmd,u8 sensor);
void homeRFSetDefaultSensorName(u8 type, u8 *sName);
void homeRFCheckAppPairSensor(void);
void homeRFStartUpdate(u32 starPos, u8 codeLen);
void homeRFKeyParse(u8 key);
void homeRFSentEvent2UI(u8 idx);
u8 homeRFGetSensorIndex(u32 ID);
void homeRFSetDefaultScene(void);
u8 homeRFGetBattery(u32 adcVal);
#endif
