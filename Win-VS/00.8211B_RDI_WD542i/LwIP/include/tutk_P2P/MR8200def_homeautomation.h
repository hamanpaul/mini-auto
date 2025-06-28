//
//  MR8200def.h
//  Mars MR8200
//
//  Created by RD1-Gavin on 2014/8/7.
//  Copyright (c) 2014年 com.tutk. All rights reserved.
//

#ifndef Mars_MR8200_MR8200def_h
#define Mars_MR8200_MR8200def_h
#include "AVIOCTRLDEFs.h"
//----------------------------------
#define room_test        1
#define sensor_test      1
//----------------------------------

#define SENSORNAME_LEN        16

typedef enum {
    mrst_Door           = 0,
    mrst_RemoteCtrl     = 1,
    mrst_PIR            = 2,
    mrst_Vibrate        = 3,
    mrst_Leak           = 4,
    mrst_Smoke          = 5,
    mrst_Gas            = 6,
    mrst_Siren          = 7,
    mrst_Temperature    = 8,
    mrst_HearBeat       = 9,
    mrst_BooldPressure  = 10,
    mrst_PowerPlug		= 11,
    mrst_Panic		    = 12,
    mrst_IAQ			= 13,
    mrst_ADE			= 14,
    mrst_FDS			= 15,
    mrst_SWITCH			= 16,
    mrst_TEMP_HYG		= 17
} emSensorType;

typedef enum {
    mrss_Unknown         = 0,
    mrss_Normal          = 1,
    mrss_Tirggered	     = 2,	// 表示Sensor處於觸發狀態
    mrss_alarm	         = 3,	// 表示Siren狀態用
	mrss_disalarm	     = 4,	// 表示Siren狀態用
	mrss_sos	         = 5,	// 表示Siren狀態用
    mrss_LowPower        = 10,
    mrss_Unavailable     = 20   // 表示Sensor無法正常連線
} emSensorStatus;
// ---------------- 以下EventID ----------------
#define EventID_Unlink			    0x000
#define EventID_LowBattery			0x001
#define EventID_MotionDetected      0x100
#define EventID_DoorWindowOpen      0x200
#define EventID_DoorWindowClose     0x201
#define EventID_PIRDetected         0x300
#define EventID_TemperatureHigh     0x400
#define EventID_TemperatureLow      0x401
#define EventID_SirenSwitchOn		0x500


// ---------------- 以下為門窗感測 ----------------
typedef struct {
    unsigned int nIsOpen;    // 0: Close ; 1: Open
} SDoor;

// ---------------- 以下為PIR感測 ----------------
typedef struct {
    unsigned int nReserved;	// 無意義,要獲得Trigger狀態改由emSensorStatus
} SPIR;

// ---------------- 以下為溫度感測 ----------------
typedef struct {
	int nTemperature;	// 攝氏度
	int nHigh;	        // 高溫警報設定值
	int nLow;	        // 低溫警報設定值
			            //只用前兩個byte，第一個byte(Float)，第二個
			            //byte(Int)，兩個byte第八個bit皆用來表示正負號(0:正號, 1: 負號)
    int nAlarmSwitch;   // bit0: all off; bit 1:高溫警報; bit 2:低溫警報
} STemperature;

typedef struct {
	u8	Thermometer_H;
	u8	Thermometer_L;
	u8	Hygrometer_H;
	u8	Hygrometer_L;
} STEMP_HYG;


// ---------------- 以下為Plug感測 ----------------
typedef struct {
	unsigned int nIsPowerOn;	// Plug的狀態 0: Off ; 1: On
	unsigned int nWattage;	// 顯示plug瓦數
	unsigned int nVoltage;	// 顯示plug電壓
	unsigned int nCurrent;	// 顯示plug電流
	unsigned char byteHasGalvanometer;	// 是否支援電流計0: false, 1: true
	unsigned char reserved[3];	// 湊４的倍數

} SPowerPlug;


// ---------------- 以下為Siren感測 ----------------
typedef struct {
	unsigned int nIsRinging;	// Siren的狀態 0: Mute ; 1: Ringing 
} SSiren;


typedef struct
{
	unsigned int nSensorID;	        //此值由Device端配發,理應不重複
	unsigned char szName[SENSORNAME_LEN];
	unsigned char byteType;	        // refer to emSensorType
	unsigned char byteStatus;	    // refer to emSensorStatus
	unsigned char bytePushAlarm;	// 為該Sensor警示的總開關,若將其關閉則所有client將無法接收到此Sensor的警示
	unsigned char byteBattery;	    // -1: 不支援電量顯示，電量數值範圍由 0~100
	unsigned char byteSirenAlarm;	// 表示該Sensor是否會觸發Siren
	unsigned char byteIsHealthType;	// 0: false, 1: true
	unsigned char byteIsSceneAffect;// 0: false, 1: true
	unsigned char byteSameOldID;	//Sean: 20170612 Add.

	union {
		SDoor door;
		SPIR pir;
		STemperature tp;
		SSiren siren;
		SPowerPlug plug;
		STEMP_HYG temp;
		
	} data;

} SSensorStore;


// 以下為IO command定義
//

// ----------------------
// 讀取Sensor清單
// ----------------------
#define IOTYPE_USER_GETSENSORLST_REQ        0x600
#define IOTYPE_USER_GETSENSORLST_RESP       0x601

typedef struct
{
    unsigned int channel;    // AvServer Index
} SMsgAVIoctrlGetSensorLstReq;

#define MAXSENSOR_NUM_ONCE        20
typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nTotalCount;    // Sensor總數
    unsigned int nStartIdx;    // 起始index
                            // Ex. (1)    channel:0    nTotalCount:15    startIdx:0 ...
                            //     (2)    channel:0    nTotalCount:15    startIdx:10 ...
                            //
    unsigned int nCount;    // sRooms有效長度
    SSensorStore sSensors[MAXSENSOR_NUM_ONCE];
} SMsgAVIoctrlGetSensorLstResp;

// ----------------------
// 新增Sensor
// ----------------------
#define IOTYPE_USER_SETADDSENSOR_REQ        0x602
#define IOTYPE_USER_SETADDSENSOR_RESP       0x603

typedef struct
{
    unsigned int channel;    // AvServer Index
} SMsgAVIoctrlSetAddSensorReq;

typedef struct
{
    unsigned int channel;    // AvServer Index
    int result;    // 回傳值    0: success; otherwise: failed.
    SSensorStore sSensor;
} SMsgAVIoctrlSetAddSensorResp;

// ----------------------
// 修改/編輯Sensor
// ----------------------
#define IOTYPE_USER_SETEDITSENSOR_REQ        0x604
#define IOTYPE_USER_SETEDITSENSOR_RESP       0x605

typedef struct
{
    unsigned int channel;    // AvServer Index
    SSensorStore sSensor;
} SMsgAVIoctrlSetEditSensorReq;

typedef struct
{
    int result;    // 回傳值    0: success; otherwise: failed.
    unsigned char reserved[4];
    
} SMsgAVIoctrlSetEditSensorResp;

// ----------------------
// 刪除Sensor
// ----------------------
#define IOTYPE_USER_SETDELSENSOR_REQ        0x606
#define IOTYPE_USER_SETDELSENSOR_RESP       0x607

typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nSensorID;
} SMsgAVIoctrlSetDelSensorReq;

typedef struct
{
    int result;    // 回傳值    0: success; otherwise: failed.
    unsigned char reserved[4];
    
} SMsgAVIoctrlSetDelSensorResp;

// ----------------------
// Room基本資料結構
// ----------------------
#define MAXSENSOR_NUM        32
#define ROOMNAME_LEN         64

typedef struct
{
    int nRoomID;
    unsigned char szName[ROOMNAME_LEN];
    
    unsigned int nSensorCount;
    unsigned int nSensors[MAXSENSOR_NUM];
} SRoomStore;

// ----------------------
// 讀取報警主機Room清單
// ----------------------
#define IOTYPE_USER_GETROOMLST_REQ        0x60C
#define IOTYPE_USER_GETROOMLST_RESP       0x60D

typedef struct
{
    unsigned int channel;    // AvServer Index
} SMsgAVIoctrlGetRoomLstReq;

#define MAXROOM_NUM_ONCE        5 //自行訂意
typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nTotalCount;    // Room總數
    unsigned int nStartIdx;    // 起始index
    unsigned int nCount;    // sRooms有效長度
    SRoomStore sRooms[MAXROOM_NUM_ONCE];
} SMsgAVIoctrlGetRoomLstResp;
//P.S.若Room總數超過MAXROOM_NUM_ONCE數值,則一次的REQ會發生多次的RESP

// ----------------------
// 新增Room
// ----------------------
#define IOTYPE_USER_SETADDROOM_REQ        0x60E
#define IOTYPE_USER_SETADDROOM_RESP       0x60F

typedef struct
{
    unsigned int channel;    // AvServer Index
    SRoomStore sRoom;    // 這邊的SRoomStore. nRoomID設為-1
} SMsgAVIoctrlSetAddRoomReq;
typedef struct
{
    unsigned int channel;    // AvServer Index
    SRoomStore sRoom;    // 這邊的SRoomStore. nRoomID會回傳有效的值(>=0)
} SMsgAVIoctrlSetAddRoomResp;

// ----------------------
// 修改/編輯Room
// ----------------------
#define IOTYPE_USER_SETEDITROOM_REQ     0x610
#define IOTYPE_USER_SETEDITROOM_RESP    0x611

typedef struct
{
    unsigned int channel;    // AvServer Index
    SRoomStore sRoom;
} SMsgAVIoctrlSetEditRoomReq;

typedef struct
{
    int result;    // 回傳值    0: success; otherwise: failed.
    unsigned char reserved[4];
    
} SMsgAVIoctrlSetEditRoomResp;

// ----------------------
// 刪除Room
// ----------------------
#define IOTYPE_USER_SETDELROOM_REQ        0x612
#define IOTYPE_USER_SETDELROOM_RESP       0x613

typedef struct
{
    unsigned int channel;    // AvServer Index
    int nRoomID;
} SMsgAVIoctrlSetDelRoomReq;

typedef struct
{
    int result;    // 回傳值    0: success; otherwise: failed.
    unsigned char reserved[4];
    
} SMsgAVIoctrlSetDelRoomResp;

// ----------------------
// 是否支援IP Camera
// ----------------------
#define IOTYPE_USER_GETIPCAMSUPPORT_REQ     0x614
#define IOTYPE_USER_GETIPCAMSUPPORT_RESP    0x615

typedef struct
{
    unsigned int channel;    // AvServer Index
} SMsgAVIoctrlGetIPCamSupportReq;

#define MAXROOM_NUM_ONCE        5
typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nIsSupport;    // 0: Not support ; 1: Support
    
} SMsgAVIoctrlGetIPCamSupportResp;

// ----------------------
// 讀取特定Sensor的資訊
// ----------------------
#define IOTYPE_USER_GETSENSOR_REQ       0x616
#define IOTYPE_USER_GETSENSOR_RESP      0x617

typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nSensorID;
} SMsgAVIoctrlGetSensorReq;

typedef struct
{
    int result;    // 回傳值    0: success; otherwise: failed.
    SSensorStore sSensor;
} SMsgAVIoctrlGetSensorResp;

// ----------------------
// Scene基本資料結構
// ----------------------
#define SCENENAME_LEN        64

typedef struct
{
    unsigned int nSceneID;
    unsigned char szName[SCENENAME_LEN];
} SSceneHead;

// ----------------------
// 讀取報警主機Scene清單
// ----------------------
#define IOTYPE_USER_GETSCENCELST_REQ        0x618
#define IOTYPE_USER_GETSCENCELST_RESP       0x619

typedef struct
{
    unsigned int channel;    // AvServer Index
} SMsgAVIoctrlGetSceneLstReq;

#define MAXSCENE_NUM_ONCE        12
typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nTotalCount;    // Scene總數
    unsigned int nStartIdx;    // 起始index
    unsigned int nCount;    // sSceneHead有效長度
    SSceneHead sSceneHead[MAXSCENE_NUM_ONCE];
} SMsgAVIoctrlGetSceneLstResp;

//P.S.若Scene總數超過MAXSCENE_NUM_ONCE數值,則一次的REQ會發生多次的RESP

// ----------------------
// 新增Scene - 新增SceneHead
// ----------------------
#define IOTYPE_USER_SETADDSCENEHEAD_REQ    0x61A
#define IOTYPE_USER_SETADDSCENEHEAD_RESP   0x61B

typedef struct
{
    unsigned int channel;    // AvServer Index
    SSceneHead sScene;    // 這邊的SSceneHead. nSceneID設為-1
} SMsgAVIoctrlSetAddSceneHeadReq;

typedef struct
{
    unsigned int channel;    // AvServer Index
    SSceneHead sScene;    // 這邊的SSceneHead. nSceneID會回傳有效的值(>=0)
} SMsgAVIoctrlSetAddSceneHeadResp;

// ----------------------
// 讀取Scene內容 (All sensors in Scene)
// ----------------------
#define IOTYPE_USER_GETSCENE_REQ    0x61C
#define IOTYPE_USER_GETSCENE_RESP   0x61D

typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nSceneID;    
} SMsgAVIoctrlGetSceneReq;

//#define MAXSENSOR_NUM_ONCE        10
typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nTotalCount;    // 這個Scene下的所有Sensor總數
    unsigned int nStartIdx;    // 起始index
    unsigned int nCount;    // sSensors有效長度
    SSensorStore sSensors[MAXSENSOR_NUM_ONCE];
} SMsgAVIoctrlGetSceneResp;

//P.S.若Sensor總數超過MAXSENSOR_NUM_ONCE數值,則一次的REQ會發生多次的RESP typedef 

// ----------------------
// 修改/編輯Scene名稱
// ----------------------
#define IOTYPE_USER_SETEDITSCENEHEAD_REQ    0x61E
#define IOTYPE_USER_SETEDITSCENEHEAD_RESP   0x61F

typedef struct
{
    unsigned int channel;    // AvServer Index
    SSceneHead sScene;
} SMsgAVIoctrlSetEditSceneHeadReq;

typedef struct
{
    int result;    // 回傳值    0: success; otherwise: failed.
    unsigned char reserved[4];

} SMsgAVIoctrlSetEditSceneHeadResp;

// ----------------------
// 修改/編輯Scene (All sensors in Scene)
// ----------------------
#define IOTYPE_USER_SETEDITSCENE_REQ    0x620
#define IOTYPE_USER_SETEDITSCENE_RESP   0x621

//#define MAXSENSOR_NUM_ONCE        10
typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nSceneID;    // 欲設定的SceneID
    unsigned int nTotalCount;    // 這個Scene下的所有Sensor總數
    unsigned int nStartIdx;    // 起始index
    unsigned int nCount;    // sSensors有效長度
    SSensorStore sSensors[MAXSENSOR_NUM_ONCE];
} SMsgAVIoctrlSetEditSceneReq;

typedef struct
{
    int result;    // 回傳值    0: success; otherwise: failed.
    int nStartIdx;    // 同 SMsgAVIoctrlSetEditSceneReq 裡的 nStartIdx
} SMsgAVIoctrlSetEditSceneResp;

// ----------------------
// 刪除Scene
// ----------------------
#define IOTYPE_USER_SETDELSCENE_REQ    0x622
#define IOTYPE_USER_SETDELSCENE_RESP   0x623

typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nSceneID;
} SMsgAVIoctrlSetDelSceneReq;

typedef struct
{
    int result;    // 回傳值    0: success; otherwise: failed.
    unsigned char reserved[4];

} SMsgAVIoctrlSetDelSceneResp;

// ----------------------
// 執行Scene
// ----------------------
#define IOTYPE_USER_PLAYSCENE_REQ       0x626
#define IOTYPE_USER_PLAYSCENE_RESP      0x627

typedef struct
{
	unsigned int channel;	// AvServer Index
	unsigned int nSceneID;
} SMsgAVIoctrlPlaySceneReq;

typedef struct
{
	int result;	// 回傳值	0: success; otherwise: failed.
	unsigned char reserved[4];
} SMsgAVIoctrlPlaySceneResp;


// ----------------------
// 讀取特定Sensor的log
// ----------------------
#define IOTYPE_USER_GETSENSORLOG_REQ    0x624
#define IOTYPE_USER_GETSENSORLOG_RESP   0x625

typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nSensorID;
    unsigned int nStartIdx;    // 指定起始index
 } SMsgAVIoctrlGetSensorLogReq;

// ----------------------
// 關閉警報
// ----------------------
#define IOTYPE_USER_STOPALARM_REQ       0x628
#define IOTYPE_USER_STOPALARM_RESP      0x629

typedef struct
{
	unsigned char reserved[4];

} SMsgAVIoctrlStopAlarmReq;

typedef struct
{
	int result;	// 回傳值	0: success; otherwise: failed.
	unsigned char reserved[4];
} SMsgAVIoctrlStopAlarmResp;

/*typedef struct
{
    unsigned short year;    // The number of year.
    unsigned char month;    // The number of months since January, in the range 1 to 12.
    unsigned char day;        // The day of the month, in the range 1 to 31.
    unsigned char wday;        // The number of days since Sunday, in the range 0 to 6. (Sunday = 0, Monday = 1, ...)
    unsigned char hour;     // The number of hours past midnight, in the range 0 to 23.
    unsigned char minute;   // The number of minutes after the hour, in the range 0 to 59.
    unsigned char second;   // The number of seconds after the minute, in the range 0 to 59.
}STimeDay;*/
 // define in AVIOCTRLDEFs.h
typedef struct
{
    STimeDay time;    // Event time
    unsigned int nPreDefEventID;    // EventID define in SensorEnentStringTable.csv
    unsigned char szName[SENSORNAME_LEN];	//Triggered sensor name
} SSensorLogRecord;

#define MAX_LOGRECORD_NUM    20
typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nSensorID;    // 所屬Sensor的ID
    unsigned int nTotalCount; // Log record 總數
    unsigned int nStartIdx; // 起始index
    unsigned int nCount;    // sRecords有效長度
    SSensorLogRecord sRecords[MAX_LOGRECORD_NUM];
} SMsgAVIoctrlGetSensorLogResp;


typedef struct
{
    STimeDay time;    // Event time
    unsigned int nPreDefEventID;    // EventID define in SensorEnentStringTable.csv
    unsigned int nSensorType;    // 所屬Sensor的Type    
    unsigned char szName[SENSORNAME_LEN];	//Triggered sensor name
} HOMERF_SensorLogRecord;

#define MAX_UI_LOGRECORD_NUM    7
#define MAX_UI_LOGRECORD_DAY    7

typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nTotalCount; // Log record 總數
    unsigned int nStartIdx; // 起始index
    unsigned int nCount;    // sRecords有效長度
    HOMERF_SensorLogRecord sRecords[MAX_UI_LOGRECORD_NUM];
} HOMERF_SensorLogList;

typedef struct
{
    STimeDay time;    // Event time
} HOMERF_SensorLogDayRecord;

typedef struct
{
    unsigned int nTotalCount; // Log record 總數
    unsigned int nStartIdx; // 起始index
    unsigned int nCount;    // sRecords有效長度
    HOMERF_SensorLogDayRecord sRecords[MAX_UI_LOGRECORD_NUM];
} HOMERF_SensorLogDayList;


#endif
